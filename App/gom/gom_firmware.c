#include "board_config.h"
#include "board_io.h"
#include "board_shift_register.h"
#include "gom_firmware.h"
#include "gom_response.h"
#include "gom_router.h"
#include "relay_matrix.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define BOOT_QUERY_TIMEOUT_MS 500u
#define RECOVERY_DELAY_MS 250u
#define LINE_BUDGET 64u

typedef struct { uint8_t data[GOM_UART_RING_SIZE]; volatile uint16_t head, tail; volatile bool overflow; } byte_ring_t;
typedef enum {
    APP_INIT_SAFE, APP_INIT_ROUTE, APP_INIT_IDN_TX, APP_INIT_IDN_RX, APP_INIT_CFG_TX, APP_INIT_CFG_RX,
    APP_NORMAL_IDLE, APP_NORMAL_ROUTE, APP_NORMAL_TX, APP_NORMAL_RX, APP_NORMAL_VERIFY_TX,
    APP_NORMAL_VERIFY_RX, APP_LOCAL_ROUTE, APP_ERROR_SAFE, APP_ERROR_WAIT
} app_state_t;

static gom_router_t router;
static relay_matrix_t relays;
static byte_ring_t pc_rx, gom_rx, pc_tx;
static volatile bool pc_tx_busy, gom_tx_busy, uart_fault;
static uint16_t pc_tx_count;
static app_state_t state;
static uint32_t deadline;
static uint8_t boot_channel, boot_query;
static bool boot_config_ok;
static uint8_t pc_rx_byte, gom_rx_byte;
static gom_operation_t operation;
static char pc_line[GOM_PC_MESSAGE_MAX + 1u];
static char gom_line[GOM_WIRE_COMMAND_MAX];
static uint16_t pc_line_used, gom_line_used;

static bool ring_push(byte_ring_t *ring, uint8_t value)
{
    uint16_t next = (uint16_t)((ring->head + 1u) & (GOM_UART_RING_SIZE - 1u));
    if (next == ring->tail) { ring->overflow = true; return false; }
    ring->data[ring->head] = value; ring->head = next; return true;
}
static bool ring_pop(byte_ring_t *ring, uint8_t *value)
{
    if (ring->tail == ring->head) return false;
    *value = ring->data[ring->tail]; ring->tail = (uint16_t)((ring->tail + 1u) & (GOM_UART_RING_SIZE - 1u)); return true;
}
static void ring_clear(byte_ring_t *ring) { ring->tail = ring->head; ring->overflow = false; }
static bool expired(uint32_t now, uint32_t when) { return (int32_t)(now - when) >= 0; }

static void pc_puts(const char *text)
{
    while (*text != '\0') {
        if (!ring_push(&pc_tx, (uint8_t)*text++)) { uart_fault = true; return; }
    }
}
static void pc_error(int16_t code, const char *text)
{
    char out[96];
    gom_router_push_error(&router, code, text);
    (void)snprintf(out, sizeof out, "%d,%s\r\n", (int)code, text); pc_puts(out);
}
static void transport_step(void)
{
    uint16_t contiguous;
    if (pc_tx_busy || pc_tx.tail == pc_tx.head) return;
    contiguous = pc_tx.head > pc_tx.tail ? (uint16_t)(pc_tx.head - pc_tx.tail) : (uint16_t)(GOM_UART_RING_SIZE - pc_tx.tail);
    pc_tx_count = contiguous;
    if (HAL_UART_Transmit_IT(BOARD_UART_PC, &pc_tx.data[pc_tx.tail], contiguous) == HAL_OK) pc_tx_busy = true;
    else uart_fault = true;
}
static bool gom_send(const char *text)
{
    if (gom_tx_busy) return false;
    if (HAL_UART_Transmit_IT(BOARD_UART_GOM, (uint8_t *)text, (uint16_t)strlen(text)) != HAL_OK) { uart_fault = true; return false; }
    gom_tx_busy = true; return true;
}
static void all_outputs_safe(void) { relay_matrix_emergency_off(&relays); }
static void enter_error(int16_t code, const char *text, uint32_t now)
{
    all_outputs_safe(); ring_clear(&gom_rx); pc_error(code, text); deadline = now + RECOVERY_DELAY_MS; state = APP_ERROR_SAFE;
}
static bool take_line(byte_ring_t *ring, char *line, uint16_t *used, size_t size)
{
    uint8_t c; uint32_t budget = LINE_BUDGET;
    while (budget-- != 0u && ring_pop(ring, &c)) {
        if (c == '\r') continue;
        if (c == '\n') { line[*used] = '\0'; *used = 0u; return line[0] != '\0'; }
        if (*used + 1u >= size || !isprint(c)) { *used = 0u; return false; }
        line[(*used)++] = (char)c;
    }
    return false;
}
static void set_identity(uint8_t channel, const char *answer)
{
    char model[16], serial[16], firmware[16]; gom_model_t type = GOM_MODEL_UNKNOWN; gom_device_t *device;
    if (!gom_parse_identity(answer, model, sizeof model, serial, sizeof serial, firmware, sizeof firmware)) { gom_router_set_device(&router, channel, type, false); return; }
    if (!strcmp(model, "GOM-804") || !strcmp(model, "GOM804")) type = GOM_MODEL_804;
    else if (!strcmp(model, "GOM-805") || !strcmp(model, "GOM805")) type = GOM_MODEL_805;
    gom_router_set_device(&router, channel, type, type != GOM_MODEL_UNKNOWN);
    device = &router.devices[channel - 1u];
    (void)snprintf(device->serial, sizeof device->serial, "%s", serial);
    (void)snprintf(device->firmware, sizeof device->firmware, "%s", firmware);
}
static bool local_command(const char *line)
{
    return !strcmp(line, "*IDN?") || !strcmp(line, "*TST?") || !strcmp(line, "*RST") || !strcmp(line, "SYST:ERR?") ||
           !strcmp(line, "ROUT:CHAN?") || !strcmp(line, "ROUT:OPEN:ALL") || !strcmp(line, "ROUT:LIM:LOW?") || !strcmp(line, "ROUT:LIM:UPP?") ||
           !strncmp(line, "ROUT:CHAN ", 10u) || !strncmp(line, "ROUT:LIM:LOW ", 13u) || !strncmp(line, "ROUT:LIM:UPP ", 13u) ||
           !strcmp(line, "SYST:COMM:TIMEOUT?") || !strncmp(line, "SYST:COMM:TIMEOUT ", 18u);
}
static void finish_response(void)
{
    char out[96]; double value; gom_measurement_status_t result;
    if (operation.id == GOM_CMD_READ) {
        result = gom_parse_measurement(gom_line, &value);
        if (result != GOM_RESPONSE_NUMBER) { pc_error(result == GOM_RESPONSE_OVER_RANGE ? -231 : -350, "Invalid GOM measurement"); return; }
        if (!gom_router_value_in_limits(&router, operation.channel, value)) { pc_error(-222, "Measurement outside configured limits"); return; }
        (void)snprintf(out, sizeof out, "%.9G\r\n", value); pc_puts(out);
    } else if (operation.id == GOM_CMD_IDN) { set_identity(operation.channel, gom_line); pc_puts(gom_line); pc_puts("\r\n"); }
    else { pc_puts(gom_line); pc_puts("\r\n"); }
}
static void execute_local(uint32_t now)
{
    char out[96];
    if (!strncmp(pc_line, "ROUT:CHAN ", 10u)) { if (!relay_matrix_request(&relays, (uint8_t)operation.integer, now)) { enter_error(-350, "Relay request failed", now); return; } state = APP_LOCAL_ROUTE; return; }
    if (!strcmp(pc_line, "*RST") || !strcmp(pc_line, "ROUT:OPEN:ALL")) { all_outputs_safe(); router.selected_channel = 0u; }
    if (!strcmp(pc_line, "*IDN?")) pc_puts("GOM850-ROUTER,STM32F411,0001,3.0\r\n");
    else if (!strcmp(pc_line, "*TST?")) pc_puts(relays.interlock_fault ? "1\r\n" : "0\r\n");
    else if (!strcmp(pc_line, "SYST:ERR?")) { gom_router_pop_error(&router, out, sizeof out); pc_puts(out); pc_puts("\r\n"); }
    else if (!strcmp(pc_line, "ROUT:LIM:LOW?") || !strcmp(pc_line, "ROUT:LIM:UPP?")) { (void)snprintf(out, sizeof out, "%.9G\r\n", operation.number); pc_puts(out); }
    else if (!strcmp(pc_line, "ROUT:CHAN?") || !strcmp(pc_line, "SYST:COMM:TIMEOUT?")) { (void)snprintf(out, sizeof out, "%ld\r\n", (long)operation.integer); pc_puts(out); }
    else pc_puts("0,No error\r\n");
    state = APP_NORMAL_IDLE;
}
static void start_pc_command(uint32_t now)
{
    gom_router_status_t status; char *p;
    for (p = pc_line; *p != '\0'; ++p) *p = (char)toupper((unsigned char)*p);
    status = gom_router_execute(&router, pc_line, &operation);
    if (status != GOM_ROUTER_OK) { pc_error(-300, gom_router_status_text(status)); state = APP_NORMAL_IDLE; return; }
    if (local_command(pc_line)) { execute_local(now); return; }
    if (!router.devices[operation.channel - 1u].online || !router.devices[operation.channel - 1u].configuration_loaded || router.devices[operation.channel - 1u].desynchronized) { pc_error(-360, "GOM channel unavailable"); state = APP_NORMAL_IDLE; return; }
    if (!relay_matrix_request(&relays, operation.channel, now)) { enter_error(-350, "Relay request failed", now); return; }
    state = APP_NORMAL_ROUTE;
}

void gom_firmware_init(void)
{
    const relay_matrix_io_t io = { board_route_request_image, board_route_busy, board_route_failed, NULL };
    memset(&pc_rx, 0, sizeof pc_rx); memset(&gom_rx, 0, sizeof gom_rx); memset(&pc_tx, 0, sizeof pc_tx);
    gom_router_init(&router); board_shift_register_init(); relay_matrix_init(&relays, &io);
    (void)HAL_UART_Receive_IT(BOARD_UART_PC, &pc_rx_byte, 1u);
    (void)HAL_UART_Receive_IT(BOARD_UART_GOM, &gom_rx_byte, 1u);
    boot_channel = 1u; boot_query = 0u; boot_config_ok = true; state = relays.interlock_fault ? APP_ERROR_SAFE : APP_INIT_SAFE;
}

void gom_firmware_step(void)
{
    static const char *const cfg[] = { "SENS:FUNC?\r\n", "SENS:AUTO?\r\n", "SENS:RANG?\r\n" };
    char wire[GOM_WIRE_COMMAND_MAX]; uint32_t now = HAL_GetTick();
    board_shift_register_step(); relay_matrix_step(&relays, now); transport_step();
    if (uart_fault || pc_rx.overflow || gom_rx.overflow) { uart_fault = false; ring_clear(&pc_rx); ring_clear(&gom_rx); enter_error(-363, "UART transport overflow/fault", now); return; }
    switch (state) {
    case APP_INIT_SAFE: if (!relays.interlock_fault && !board_route_busy(NULL)) { if (relay_matrix_request(&relays, boot_channel, now)) state = APP_INIT_ROUTE; } break;
    case APP_INIT_ROUTE: if (relay_matrix_ready(&relays)) state = APP_INIT_IDN_TX; break;
    case APP_INIT_IDN_TX: if (gom_send("*IDN?\r\n")) { deadline = now + BOOT_QUERY_TIMEOUT_MS; state = APP_INIT_IDN_RX; } break;
    case APP_INIT_IDN_RX:
        if (take_line(&gom_rx, gom_line, &gom_line_used, sizeof gom_line)) { set_identity(boot_channel, gom_line); boot_query = 0u; state = APP_INIT_CFG_TX; }
        else if (expired(now, deadline)) { gom_router_set_device(&router, boot_channel, GOM_MODEL_UNKNOWN, false); boot_config_ok = false; state = APP_INIT_CFG_TX; }
        break;
    case APP_INIT_CFG_TX:
        if (boot_query >= 3u) { gom_router_mark_configuration_loaded(&router, boot_channel, router.devices[boot_channel - 1u].online && boot_config_ok); all_outputs_safe(); if (++boot_channel > GOM_CHANNEL_COUNT) state = APP_NORMAL_IDLE; else { boot_config_ok = true; state = APP_INIT_SAFE; } }
        else if (gom_send(cfg[boot_query])) { deadline = now + BOOT_QUERY_TIMEOUT_MS; state = APP_INIT_CFG_RX; }
        break;
    case APP_INIT_CFG_RX:
        if (take_line(&gom_rx, gom_line, &gom_line_used, sizeof gom_line)) { ++boot_query; state = APP_INIT_CFG_TX; }
        else if (expired(now, deadline)) { boot_config_ok = false; ++boot_query; state = APP_INIT_CFG_TX; }
        break;
    case APP_NORMAL_IDLE:
        if (take_line(&pc_rx, pc_line, &pc_line_used, sizeof pc_line)) start_pc_command(now);
        break;
    case APP_NORMAL_ROUTE:
        if (relay_matrix_ready(&relays)) { if (gom_encode_operation(&operation, wire, sizeof wire) != GOM_ROUTER_OK) enter_error(-300, "Command encoding failed", now); else { (void)snprintf(gom_line, sizeof gom_line, "%s", wire); state = APP_NORMAL_TX; } }
        break;
    case APP_NORMAL_TX:
        if (gom_send(gom_line)) { deadline = now + router.timeout_ms; state = operation.query ? APP_NORMAL_RX : APP_NORMAL_VERIFY_TX; }
        break;
    case APP_NORMAL_RX:
        if (take_line(&gom_rx, gom_line, &gom_line_used, sizeof gom_line)) { finish_response(); state = APP_NORMAL_IDLE; }
        else if (expired(now, deadline)) enter_error(-365, "GOM response timeout", now);
        break;
    case APP_NORMAL_VERIFY_TX:
        if (!gom_tx_busy && gom_send("SYST:ERR?\r\n")) { deadline = now + router.timeout_ms; state = APP_NORMAL_VERIFY_RX; }
        break;
    case APP_NORMAL_VERIFY_RX:
        if (take_line(&gom_rx, gom_line, &gom_line_used, sizeof gom_line)) { if (gom_line[0] == '0') pc_puts("0,No error\r\n"); else enter_error(-350, "GOM rejected configuration write", now); state = APP_NORMAL_IDLE; }
        else if (expired(now, deadline)) enter_error(-365, "GOM verify timeout", now);
        break;
    case APP_LOCAL_ROUTE: if (relay_matrix_ready(&relays)) { pc_puts("0,No error\r\n"); state = APP_NORMAL_IDLE; } break;
    case APP_ERROR_SAFE: if (!board_route_busy(NULL)) state = APP_ERROR_WAIT; break;
    case APP_ERROR_WAIT: if (expired(now, deadline)) { ring_clear(&gom_rx); boot_channel = 1u; boot_query = 0u; state = relays.interlock_fault ? APP_ERROR_WAIT : APP_INIT_SAFE; } break;
    default: break;
    }
}

void gom_firmware_task(void *argument)
{
    (void)argument;
    for (;;) gom_firmware_step();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t value;
    if (huart == BOARD_UART_PC) { value = pc_rx_byte; (void)ring_push(&pc_rx, value); (void)HAL_UART_Receive_IT(huart, &pc_rx_byte, 1u); }
    else if (huart == BOARD_UART_GOM) { value = gom_rx_byte; (void)ring_push(&gom_rx, value); (void)HAL_UART_Receive_IT(huart, &gom_rx_byte, 1u); }
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == BOARD_UART_PC) { pc_tx.tail = (uint16_t)((pc_tx.tail + pc_tx_count) & (GOM_UART_RING_SIZE - 1u)); pc_tx_busy = false; }
    else if (huart == BOARD_UART_GOM) gom_tx_busy = false;
}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    uart_fault = true;
    if (huart == BOARD_UART_PC || huart == BOARD_UART_GOM) (void)HAL_UART_Receive_IT(huart, huart == BOARD_UART_PC ? &pc_rx_byte : &gom_rx_byte, 1u);
}
