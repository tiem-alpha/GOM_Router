#include "gom_firmware.h"

#include "board_config.h"
#include "board_io.h"
#include "board_shift_register.h"
#include "protocol_limits.h"
#include "relay_matrix.h"
#include "scpi/utils.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define GOM_BOOT_TIMEOUT_MS 500u
#define GOM_RECOVERY_DELAY_MS 250u
#define GOM_LINE_BUDGET 96u

_Static_assert(GOM_UART_RING_SIZE > 1u &&
               (GOM_UART_RING_SIZE & (GOM_UART_RING_SIZE - 1u)) == 0u,
               "GOM UART ring size must be a power of two");

typedef struct {
    uint8_t data[GOM_UART_RING_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile bool overflow;
} byte_ring_t;

typedef enum {
    FSM_BOOT_ROUTE, FSM_BOOT_IDN_TX, FSM_BOOT_IDN_RX, FSM_BOOT_CFG_TX,
    FSM_BOOT_CFG_RX, FSM_READY, FSM_ROUTE, FSM_TX, FSM_RX, FSM_VERIFY_TX,
    FSM_VERIFY_RX, FSM_SELECT, FSM_OPEN_WAIT, FSM_SAFE_WAIT
} gom_fsm_t;

typedef struct {
    bool online;
    bool configured;
    char identity[64];
    char function[16];
    char auto_range[8];
    char range[24];
} gom_device_t;

static byte_ring_t pc_rx;
static byte_ring_t gom_rx;
static byte_ring_t pc_tx;
static relay_matrix_t relays;
static gom_device_t devices[GOM_CHANNEL_COUNT];
static gom_completion_callback_t completion;
static gom_operation_t active_operation;
static gom_fsm_t state;
static uint8_t pc_rx_byte;
static uint8_t gom_rx_byte;
static uint8_t boot_channel;
static uint8_t boot_query;
static uint16_t pc_tx_count;
static uint16_t pc_line_used;
static uint16_t gom_line_used;
static char gom_line[GOM_REPLY_LINE_MAX + 1u];
static char gom_tx[GOM_WIRE_COMMAND_MAX];
static uint32_t deadline_ms;
static uint32_t timeout_ms = 5000u;
static volatile bool pc_tx_busy;
static volatile bool gom_tx_busy;
static volatile bool uart_fault;
static bool boot_config_ok;
static uint8_t selected_channel;

static uint32_t ring_lock(void)
{
    uint32_t primask = __get_PRIMASK();

    __disable_irq();
    __DMB();
    return primask;
}

static void ring_unlock(uint32_t primask)
{
    __DMB();
    __set_PRIMASK(primask);
}

static bool ring_push(byte_ring_t *ring, uint8_t value)
{
    uint32_t primask = ring_lock();
    uint16_t next = (uint16_t)((ring->head + 1u) & (GOM_UART_RING_SIZE - 1u));
    if (next == ring->tail) {
        ring->overflow = true;
        ring_unlock(primask);
        return false;
    }
    ring->data[ring->head] = value;
    ring->head = next;
    ring_unlock(primask);
    return true;
}

static bool ring_pop(byte_ring_t *ring, uint8_t *value)
{
    uint32_t primask = ring_lock();

    if (ring->tail == ring->head) {
        ring_unlock(primask);
        return false;
    }
    *value = ring->data[ring->tail];
    ring->tail = (uint16_t)((ring->tail + 1u) & (GOM_UART_RING_SIZE - 1u));
    ring_unlock(primask);
    return true;
}

static void ring_clear(byte_ring_t *ring)
{
    uint32_t primask = ring_lock();

    ring->tail = ring->head;
    ring->overflow = false;
    ring_unlock(primask);
}

static bool ring_write(byte_ring_t *ring, const char *data, size_t length)
{
    uint16_t free_space;
    uint16_t head;
    size_t index;
    uint32_t primask;

    if (length > GOM_UART_RING_SIZE - 1u) return false;
    primask = ring_lock();
    head = ring->head;
    free_space = (uint16_t)((ring->tail - head - 1u) & (GOM_UART_RING_SIZE - 1u));
    if (length > free_space) {
        ring_unlock(primask);
        return false;
    }
    for (index = 0u; index < length; ++index) {
        ring->data[head] = (uint8_t)data[index];
        head = (uint16_t)((head + 1u) & (GOM_UART_RING_SIZE - 1u));
    }
    ring->head = head;
    ring_unlock(primask);
    return true;
}

static bool elapsed(uint32_t now, uint32_t deadline)
{
    return (int32_t)(now - deadline) >= 0;
}

static bool take_line(byte_ring_t *ring, char *line, uint16_t *used, size_t size)
{
    uint8_t value;
    uint32_t budget = GOM_LINE_BUDGET;

    while (budget-- != 0u && ring_pop(ring, &value)) {
        if (value == '\r') continue;
        if (value == '\n') {
            line[*used] = '\0';
            *used = 0u;
            return line[0] != '\0';
        }
        if (!isprint(value) || *used + 1u >= size) {
            *used = 0u;
            ring->overflow = true;
            return false;
        }
        line[(*used)++] = (char)value;
    }
    return false;
}

static void start_pc_tx(void)
{
    uint16_t count;

    if (pc_tx_busy || pc_tx.tail == pc_tx.head) return;
    count = pc_tx.head > pc_tx.tail ? (uint16_t)(pc_tx.head - pc_tx.tail) :
                                     (uint16_t)(GOM_UART_RING_SIZE - pc_tx.tail);
    pc_tx_count = count;
    if (HAL_UART_Transmit_IT(BOARD_UART_PC, &pc_tx.data[pc_tx.tail], count) == HAL_OK) {
        pc_tx_busy = true;
    } else {
        uart_fault = true;
    }
}

bool gom_firmware_pc_write(const char *data, size_t length)
{
    if (data == NULL) return false;
    return ring_write(&pc_tx, data, length);
}

static bool gom_send(const char *text)
{
    size_t length = strlen(text);

    if (gom_tx_busy || length >= GOM_WIRE_COMMAND_MAX) return false;
    if (HAL_UART_Transmit_IT(BOARD_UART_GOM, (uint8_t *)text, (uint16_t)length) != HAL_OK) {
        uart_fault = true;
        return false;
    }
    gom_tx_busy = true;
    return true;
}

static void safe_open(void)
{
    relay_matrix_emergency_off(&relays);
    selected_channel = 0u;
    ring_clear(&gom_rx);
    gom_line_used = 0u;
}

static void complete(gom_result_t result, const char *response)
{
    if (completion != NULL) completion(&active_operation, result, response);
}

static void copy_text(char *destination, size_t destination_size, const char *source)
{
    size_t length = strlen(source);

    if (length >= destination_size) length = destination_size - 1u;
    memcpy(destination, source, length);
    destination[length] = '\0';
}

static bool append_text(char *destination, size_t destination_size, size_t *used,
                        const char *source)
{
    size_t length = strlen(source);
    if (*used + length >= destination_size) return false;
    memcpy(&destination[*used], source, length);
    *used += length;
    destination[*used] = '\0';
    return true;
}

static bool encode_operation(const gom_operation_t *operation, char *output, size_t output_size)
{
    const char *header = NULL;
    char number[32];
    size_t used = 0u;

    switch (operation->id) {
    case GOM_OP_IDN: header = "*IDN?"; break;
    case GOM_OP_DEVICE_ERROR: header = "SYST:ERR?"; break;
    case GOM_OP_READ: header = "READ?"; break;
    case GOM_OP_FUNCTION: header = "SENS:FUNC"; break;
    case GOM_OP_AUTO: header = "SENS:AUTO"; break;
    case GOM_OP_RANGE: header = "SENS:RANG"; break;
    case GOM_OP_SPEED: header = "SENS:SPE"; break;
    case GOM_OP_RELATIVE_STATE: header = "SENS:REL:STAT"; break;
    case GOM_OP_RELATIVE_DATA: header = "SENS:REL:DAT"; break;
    case GOM_OP_TRIGGER: header = "*TRG"; break;
    default: return false;
    }
    output[0] = '\0';
    if (!append_text(output, output_size, &used, header)) return false;
    if (operation->query && header[strlen(header) - 1u] != '?' &&
        !append_text(output, output_size, &used, "?")) return false;
    if (!operation->query && (operation->id == GOM_OP_AUTO || operation->id == GOM_OP_RELATIVE_STATE)) {
        if (!append_text(output, output_size, &used, " ") ||
            !append_text(output, output_size, &used, operation->boolean ? "ON" : "OFF")) return false;
    } else if (!operation->query && (operation->id == GOM_OP_RANGE || operation->id == GOM_OP_RELATIVE_DATA)) {
        (void)SCPI_DoubleToStr(operation->number, number, sizeof(number));
        if (!append_text(output, output_size, &used, " ") ||
            !append_text(output, output_size, &used, number)) return false;
    } else if (!operation->query && (operation->id == GOM_OP_FUNCTION || operation->id == GOM_OP_SPEED)) {
        if (memchr(operation->token, '\0', sizeof(operation->token)) == NULL) return false;
        if (!append_text(output, output_size, &used, " ") ||
            !append_text(output, output_size, &used, operation->token)) return false;
    }
    return append_text(output, output_size, &used, "\r\n");
}

static void begin_safe_recovery(uint32_t now, gom_result_t result)
{
    safe_open();
    complete(result, NULL);
    deadline_ms = now + GOM_RECOVERY_DELAY_MS;
    state = FSM_SAFE_WAIT;
}

static void boot_finish_channel(void)
{
    gom_device_t *device = &devices[boot_channel - 1u];

    device->configured = device->online && boot_config_ok;
    safe_open();
    ++boot_channel;
    boot_query = 0u;
    boot_config_ok = true;
    state = boot_channel > GOM_CHANNEL_COUNT ? FSM_READY : FSM_BOOT_ROUTE;
}

void gom_firmware_init(gom_completion_callback_t complete_callback)
{
    const relay_matrix_io_t io = {
        .request_image = board_route_request_image,
        .emergency_disable = board_route_emergency_disable,
        .busy = board_route_busy,
        .failed = board_route_failed,
        .context = NULL,
    };

    memset(&pc_rx, 0, sizeof(pc_rx));
    memset(&gom_rx, 0, sizeof(gom_rx));
    memset(&pc_tx, 0, sizeof(pc_tx));
    memset(devices, 0, sizeof(devices));
    board_shift_register_init();
    relay_matrix_init(&relays, &io);
    completion = complete_callback;
    boot_channel = 1u;
    pc_line_used = 0u;
    gom_line_used = 0u;
    boot_config_ok = true;
    state = relays.interlock_fault ? FSM_SAFE_WAIT : FSM_BOOT_ROUTE;
    if (HAL_UART_Receive_IT(BOARD_UART_PC, &pc_rx_byte, 1u) != HAL_OK) uart_fault = true;
    if (HAL_UART_Receive_IT(BOARD_UART_GOM, &gom_rx_byte, 1u) != HAL_OK) uart_fault = true;
}

bool gom_firmware_ready(void)
{
    return state == FSM_READY && !relays.interlock_fault;
}

bool gom_firmware_select_channel(uint8_t channel)
{
    if (!gom_firmware_ready() || channel == 0u || channel > GOM_CHANNEL_COUNT) return false;
    if (!relay_matrix_request(&relays, channel, HAL_GetTick())) return false;
    selected_channel = channel;
    state = FSM_SELECT;
    return true;
}

void gom_firmware_open_all(void)
{
    bool transaction_active = state == FSM_ROUTE || state == FSM_TX || state == FSM_RX ||
                              state == FSM_VERIFY_TX || state == FSM_VERIFY_RX;

    if (transaction_active) complete(GOM_RESULT_CANCELLED, NULL);
    (void)HAL_UART_Abort_IT(BOARD_UART_GOM);
    gom_tx_busy = false;
    safe_open();
    state = FSM_OPEN_WAIT;
}

uint8_t gom_firmware_selected_channel(void)
{
    return selected_channel;
}

bool gom_firmware_set_timeout(uint32_t value)
{
    if (value < GOM_QUERY_TIMEOUT_MIN_MS || value > GOM_QUERY_TIMEOUT_MAX_MS) return false;
    timeout_ms = value;
    return true;
}

uint32_t gom_firmware_timeout(void)
{
    return timeout_ms;
}

bool gom_firmware_submit(const gom_operation_t *operation)
{
    if (!gom_firmware_ready() || operation == NULL || selected_channel == 0u ||
        !devices[selected_channel - 1u].configured) return false;
    active_operation = *operation;
    if (!relay_matrix_request(&relays, selected_channel, HAL_GetTick())) return false;
    state = FSM_ROUTE;
    return true;
}

bool gom_firmware_take_pc_line(char *line, size_t line_size)
{
    return line != NULL && line_size > 1u && take_line(&pc_rx, line, &pc_line_used, line_size);
}

void gom_firmware_step(void)
{
    static const char *const boot_config[] = {"SENS:FUNC?\r\n", "SENS:AUTO?\r\n", "SENS:RANG?\r\n"};
    uint32_t now = HAL_GetTick();

    board_shift_register_step();
    relay_matrix_step(&relays, now);
    start_pc_tx();
    if (uart_fault || pc_rx.overflow || gom_rx.overflow || relays.interlock_fault) {
        if (uart_fault) {
            (void)HAL_UART_Abort_IT(BOARD_UART_PC);
            (void)HAL_UART_Abort_IT(BOARD_UART_GOM);
            pc_tx_busy = false;
            gom_tx_busy = false;
            pc_tx_count = 0u;
            ring_clear(&pc_tx);
        }
        uart_fault = false;
        ring_clear(&pc_rx);
        ring_clear(&gom_rx);
        pc_line_used = 0u;
        gom_line_used = 0u;
        if (state != FSM_SAFE_WAIT) begin_safe_recovery(now, GOM_RESULT_UART_ERROR);
        return;
    }

    switch (state) {
    case FSM_BOOT_ROUTE:
        if (relay_matrix_request(&relays, boot_channel, now)) state = FSM_BOOT_IDN_TX;
        break;
    case FSM_BOOT_IDN_TX:
        if (relay_matrix_ready(&relays) && gom_send("*IDN?\r\n")) {
            deadline_ms = now + GOM_BOOT_TIMEOUT_MS;
            state = FSM_BOOT_IDN_RX;
        }
        break;
    case FSM_BOOT_IDN_RX:
        if (take_line(&gom_rx, gom_line, &gom_line_used, sizeof(gom_line))) {
            gom_device_t *device = &devices[boot_channel - 1u];
            device->online = strchr(gom_line, ',') != NULL;
            copy_text(device->identity, sizeof(device->identity), gom_line);
            state = FSM_BOOT_CFG_TX;
        } else if (elapsed(now, deadline_ms)) {
            boot_config_ok = false;
            state = FSM_BOOT_CFG_TX;
        }
        break;
    case FSM_BOOT_CFG_TX:
        if (boot_query == 3u) {
            boot_finish_channel();
        } else if (gom_send(boot_config[boot_query])) {
            deadline_ms = now + GOM_BOOT_TIMEOUT_MS;
            state = FSM_BOOT_CFG_RX;
        }
        break;
    case FSM_BOOT_CFG_RX:
        if (take_line(&gom_rx, gom_line, &gom_line_used, sizeof(gom_line))) {
            gom_device_t *device = &devices[boot_channel - 1u];
            char *destination = boot_query == 0u ? device->function :
                                boot_query == 1u ? device->auto_range : device->range;
            size_t destination_size = boot_query == 0u ? sizeof(device->function) :
                                      boot_query == 1u ? sizeof(device->auto_range) : sizeof(device->range);
            copy_text(destination, destination_size, gom_line);
            ++boot_query;
            state = FSM_BOOT_CFG_TX;
        } else if (elapsed(now, deadline_ms)) {
            boot_config_ok = false;
            ++boot_query;
            state = FSM_BOOT_CFG_TX;
        }
        break;
    case FSM_READY:
        break;
    case FSM_SELECT:
        if (relay_matrix_ready(&relays)) state = FSM_READY;
        break;
    case FSM_OPEN_WAIT:
        if (relay_matrix_ready(&relays)) state = FSM_READY;
        break;
    case FSM_ROUTE:
        if (relay_matrix_ready(&relays)) {
            if (!encode_operation(&active_operation, gom_tx, sizeof(gom_tx)))
                begin_safe_recovery(now, GOM_RESULT_BAD_RESPONSE);
            else
                state = FSM_TX;
        }
        break;
    case FSM_TX:
        if (gom_send(gom_tx)) {
            deadline_ms = now + timeout_ms;
            state = active_operation.query ? FSM_RX : FSM_VERIFY_TX;
        }
        break;
    case FSM_RX:
        if (take_line(&gom_rx, gom_line, &gom_line_used, sizeof(gom_line))) {
            complete(GOM_RESULT_OK, gom_line);
            state = FSM_READY;
        } else if (elapsed(now, deadline_ms)) {
            begin_safe_recovery(now, GOM_RESULT_TIMEOUT);
        }
        break;
    case FSM_VERIFY_TX:
        if (!gom_tx_busy && gom_send("SYST:ERR?\r\n")) {
            deadline_ms = now + timeout_ms;
            state = FSM_VERIFY_RX;
        }
        break;
    case FSM_VERIFY_RX:
        if (take_line(&gom_rx, gom_line, &gom_line_used, sizeof(gom_line))) {
            if (gom_line[0] == '0') complete(GOM_RESULT_OK, NULL);
            else complete(GOM_RESULT_BAD_RESPONSE, gom_line);
            state = FSM_READY;
        } else if (elapsed(now, deadline_ms)) {
            begin_safe_recovery(now, GOM_RESULT_TIMEOUT);
        }
        break;
    case FSM_SAFE_WAIT:
        if (elapsed(now, deadline_ms) && !relays.interlock_fault) {
            boot_channel = 1u;
            boot_query = 0u;
            boot_config_ok = true;
            state = FSM_BOOT_ROUTE;
        }
        break;
    default:
        break;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == BOARD_UART_PC) {
        (void)ring_push(&pc_rx, pc_rx_byte);
        if (HAL_UART_Receive_IT(huart, &pc_rx_byte, 1u) != HAL_OK) uart_fault = true;
    } else if (huart == BOARD_UART_GOM) {
        (void)ring_push(&gom_rx, gom_rx_byte);
        if (HAL_UART_Receive_IT(huart, &gom_rx_byte, 1u) != HAL_OK) uart_fault = true;
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == BOARD_UART_PC) {
        pc_tx.tail = (uint16_t)((pc_tx.tail + pc_tx_count) & (GOM_UART_RING_SIZE - 1u));
        pc_tx_busy = false;
    } else if (huart == BOARD_UART_GOM) {
        gom_tx_busy = false;
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    uart_fault = true;
    if (huart == BOARD_UART_PC) {
        pc_tx_busy = false;
        if (HAL_UART_Receive_IT(huart, &pc_rx_byte, 1u) != HAL_OK) uart_fault = true;
    }
    if (huart == BOARD_UART_GOM) {
        gom_tx_busy = false;
        if (HAL_UART_Receive_IT(huart, &gom_rx_byte, 1u) != HAL_OK) uart_fault = true;
    }
}

void HAL_UART_AbortCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == BOARD_UART_PC) {
        pc_tx_busy = false;
        if (HAL_UART_Receive_IT(huart, &pc_rx_byte, 1u) != HAL_OK) uart_fault = true;
    } else if (huart == BOARD_UART_GOM) {
        gom_tx_busy = false;
        if (HAL_UART_Receive_IT(huart, &gom_rx_byte, 1u) != HAL_OK) uart_fault = true;
    }
}
