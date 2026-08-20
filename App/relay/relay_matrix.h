#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "protocol_limits.h"

/* Generic, non-blocking 16-output relay layer.  Image bit 2*(channel-1) and
 * bit 2*(channel-1)+1 select one of eight break-before-make routes. */
typedef struct {
    bool (*request_image)(void *context, uint16_t image);
    void (*emergency_disable)(void *context);
    bool (*busy)(void *context);
    bool (*failed)(void *context);
    void *context;
} relay_matrix_io_t;

typedef enum { RELAY_IDLE, RELAY_OPENING, RELAY_BREAKING, RELAY_SETTLING, RELAY_FAULT } relay_state_t;
typedef struct {
    relay_matrix_io_t io;
    uint8_t selected_channel;
    uint8_t requested_channel;
    relay_state_t state;
    uint32_t deadline_ms;
    bool interlock_fault;
    uint32_t fault_epoch;
} relay_matrix_t;

void relay_matrix_init(relay_matrix_t *relay, const relay_matrix_io_t *io);
void relay_matrix_emergency_off(relay_matrix_t *relay);
bool relay_matrix_request(relay_matrix_t *relay, uint8_t channel, uint32_t now_ms);
void relay_matrix_step(relay_matrix_t *relay, uint32_t now_ms);
bool relay_matrix_ready(const relay_matrix_t *relay);
