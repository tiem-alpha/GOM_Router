#include "board_config.h"
#include "board_io.h"
#include "gom_firmware.h"
#include "gom_router.h"
#include "log.h"
#include "relay_matrix.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

/*
 * Transaction boundary
 * --------------------
 * PC input is parsed to a gom_operation_t before any byte reaches a GOM.
 * The encoder then creates a whitelist-derived wire command.  Thus a PC can
 * neither inject arbitrary GOM SCPI nor talk to a non-selected relay channel.
 *
 * This first hardware integration intentionally uses bounded blocking HAL I/O
 * in the single owner task.  It is deterministic and safe; replacing it with
 * DMA later only changes this adapter, never router/relay safety policy.
 */
static gom_router_t router;
static relay_matrix_t relays;

static void pc_write(const char *text)
{
    (void)HAL_UART_Transmit(BOARD_UART_PC, (uint8_t *)text, (uint16_t)strlen(text),
                            BOARD_UART_TX_TIMEOUT_MS);
}

static void pc_error(gom_router_status_t status)
{
    char line[96];
    log_print("[ROUTER] reject: ");
    log_println(gom_router_status_text(status));
    (void)snprintf(line, sizeof line, "%s\r\n", gom_router_status_text(status));
    pc_write(line);
}

/* Receive exactly one CR/LF terminated response.  Partial/oversize replies
 * are discarded and treated as a link failure; they are never forwarded. */
static bool gom_read_line(char *line, size_t size, uint32_t timeout_ms)
{
    size_t used = 0u;
    uint32_t start = HAL_GetTick();
    while ((uint32_t)(HAL_GetTick() - start) < timeout_ms) {
        uint8_t ch;
        if (HAL_UART_Receive(BOARD_UART_GOM, &ch, 1u, BOARD_UART_BYTE_TIMEOUT_MS) != HAL_OK)
            continue;
        if (ch == '\r') continue;
        if (ch == '\n') { line[used] = '\0'; return used != 0u; }
        if (used + 1u >= size) return false;
        line[used++] = (char)ch;
    }
    return false;
}

static void update_identity(uint8_t channel, const char *answer)
{
    gom_model_t model = GOM_MODEL_UNKNOWN;
    /* GOM replies differ by firmware; model token is the safe minimum. */
    if (strstr(answer, "GOM-805") != NULL || strstr(answer, "GOM805") != NULL) model = GOM_MODEL_805;
    if (strstr(answer, "GOM-804") != NULL || strstr(answer, "GOM804") != NULL) model = GOM_MODEL_804;
    gom_router_set_device(&router, channel, model, model != GOM_MODEL_UNKNOWN);
}

static bool is_router_local(const char *line)
{
    return strcmp(line, "*IDN?") == 0 || strcmp(line, "*TST?") == 0 ||
           strcmp(line, "*RST") == 0 || strcmp(line, "SYST:ERR?") == 0 ||
           strcmp(line, "ROUT:CHAN?") == 0 || strcmp(line, "ROUT:OPEN:ALL") == 0 ||
           strncmp(line, "ROUT:CHAN ", 10u) == 0 ||
           strcmp(line, "SYST:COMM:TIMEOUT?") == 0 ||
           strncmp(line, "SYST:COMM:TIMEOUT ", 18u) == 0;
}

static void execute_line(char *line)
{
    gom_operation_t operation;
    char wire[GOM_WIRE_COMMAND_MAX];
    char answer[GOM_WIRE_COMMAND_MAX];
    log_print("[PC] command: ");
    log_println(line);
    gom_router_status_t status = gom_router_execute(&router, line, &operation);
    if (status != GOM_ROUTER_OK) { pc_error(status); return; }

    /* Router-owned commands never touch UART_GOM. */
    if (is_router_local(line)) {
        log_println("[ROUTER] local command");
        /* Reset/open are physical safety actions as well as logical actions. */
        if (strcmp(line, "*RST") == 0 || strcmp(line, "ROUT:OPEN:ALL") == 0)
            relay_matrix_emergency_off(&relays);
        if (strcmp(line, "*IDN?") == 0) pc_write("GOM850-ROUTER,STM32F411,0001,1.0\r\n");
        else if (strcmp(line, "*TST?") == 0) pc_write(relays.interlock_fault ? "1\r\n" : "0\r\n");
        else if (strcmp(line, "SYST:ERR?") == 0) pc_write("0,No error\r\n");
        else if (strcmp(line, "ROUT:CHAN?") == 0 || strcmp(line, "SYST:COMM:TIMEOUT?") == 0) {
            (void)snprintf(answer, sizeof answer, "%ld\r\n", (long)operation.integer); pc_write(answer);
        } else pc_write("0,No error\r\n");
        return;
    }

    log_print("[GOM] select channel ");
    log_print_u32_dec(operation.channel);
    log_println("");
    if (!relay_matrix_select(&relays, operation.channel)) {
        log_println("[FAULT] relay interlock");
        pc_write("104,Relay interlock fault\r\n");
        return;
    }
    log_println("[GOM] relay selected");
    status = gom_encode_operation(&operation, wire, sizeof wire);
    if (status != GOM_ROUTER_OK || HAL_UART_Transmit(BOARD_UART_GOM, (uint8_t *)wire,
            (uint16_t)strlen(wire), BOARD_UART_TX_TIMEOUT_MS) != HAL_OK) {
        log_println("[FAULT] GOM TX/encode failed; relays off");
        relay_matrix_emergency_off(&relays); pc_write("-360,GOM communication error\r\n"); return;
    }
    log_print("[GOM] TX: ");
    log_println(wire);
    if (!operation.query) { pc_write("0,No error\r\n"); return; }
    if (!gom_read_line(answer, sizeof answer, router.timeout_ms)) {
        log_println("[FAULT] GOM response timeout; relays off");
        relay_matrix_emergency_off(&relays); pc_write("-365,GOM response timeout\r\n"); return;
    }
    log_print("[GOM] RX: ");
    log_println(answer);
    if (operation.id == GOM_CMD_IDN) update_identity(operation.channel, answer);
    pc_write(answer); pc_write("\r\n");
}

void gom_firmware_init(void)
{
    const relay_matrix_io_t io = { board_relay_all_off, board_relay_set_one, NULL, board_delay_ms, NULL };
    gom_router_init(&router);
    relay_matrix_init(&relays, &io); /* Power-up invariant: every GOM is open. */
    log_println("[ROUTER] initialized; relays off");
}

void gom_firmware_task(void *argument)
{
    char line[GOM_PC_MESSAGE_MAX + 1u];
    size_t used = 0u;
    (void)argument;
    log_println("[ROUTER] task started");
    for (;;) {
        uint8_t ch;
        if (HAL_UART_Receive(BOARD_UART_PC, &ch, 1u, HAL_MAX_DELAY) != HAL_OK) continue;
        if (ch == '\r') continue;
        if (ch == '\n') { line[used] = '\0'; if (used != 0u) execute_line(line); used = 0u; continue; }
        if (used + 1u < sizeof line && isprint((unsigned char)ch)) line[used++] = (char)ch;
        else { used = 0u; pc_write("-363,Input buffer overrun\r\n"); }
    }
}
