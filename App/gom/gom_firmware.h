#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** @file gom_firmware.h Single-task GOM transport and relay transaction API. */

typedef enum {
    GOM_OP_IDN,
    GOM_OP_DEVICE_ERROR,
    GOM_OP_READ,
    GOM_OP_FUNCTION,
    GOM_OP_AUTO,
    GOM_OP_RANGE,
    GOM_OP_SPEED,
    GOM_OP_RELATIVE_STATE,
    GOM_OP_RELATIVE_DATA,
    GOM_OP_TRIGGER
} gom_operation_id_t;

/** Expected payload returned by a whitelisted GOM command. */
typedef enum {
    GOM_RESPONSE_NONE,
    GOM_RESPONSE_TEXT,
    GOM_RESPONSE_NUMBER
} gom_response_type_t;

typedef struct {
    gom_operation_id_t id;
    bool query;
    gom_response_type_t response_type;
    bool boolean;
    double number;
    char token[12];
} gom_operation_t;

typedef enum {
    GOM_RESULT_OK,
    GOM_RESULT_TIMEOUT,
    GOM_RESULT_UART_ERROR,
    GOM_RESULT_BAD_RESPONSE,
    GOM_RESULT_UNAVAILABLE,
    GOM_RESULT_CANCELLED
} gom_result_t;

typedef void (*gom_completion_callback_t)(const gom_operation_t *operation,
                                          gom_result_t result,
                                          const char *response);

/** Initialise UART rings, safe relay matrix, and boot-time GOM discovery. */
void gom_firmware_init(gom_completion_callback_t complete);
/** Progress UART, 74HC595, relay and GOM transaction state machines once. */
void gom_firmware_step(void);
/** Submit one typed, whitelisted GOM operation. Never accepts raw SCPI text. */
bool gom_firmware_submit(const gom_operation_t *operation);
/** Request safe break-before-make selection of GOM channel 1..8. */
bool gom_firmware_select_channel(uint8_t channel);
/**
 * Cancel an active transaction, release all GOM relays, and discard the
 * selected channel. A pending query completes with GOM_RESULT_CANCELLED.
 */
void gom_firmware_open_all(void);
/** Return the selected GOM channel, or zero when all relays are open. */
uint8_t gom_firmware_selected_channel(void);
/** Return true only after discovery is complete and no transaction is active. */
bool gom_firmware_ready(void);
/** Get one complete PC SCPI line, without CR/LF, from the ISR-fed ring. */
bool gom_firmware_take_pc_line(char *line, size_t line_size);
/** Queue bytes for DMA transmission to the PC. Returns false on overflow. */
bool gom_firmware_pc_write(const char *data, size_t length);
/** Set/read the bounded GOM query timeout. */
bool gom_firmware_set_timeout(uint32_t timeout_ms);
uint32_t gom_firmware_timeout(void);
