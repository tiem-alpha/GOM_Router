#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "protocol_limits.h"

typedef struct {
    void (*all_off)(void *context);
    void (*set_one)(void *context, uint8_t channel); /* channel is 1..8 */
    uint8_t (*feedback_mask)(void *context);         /* one bit per relay */
    void (*delay_ms)(void *context, uint32_t ms);
    void *context;
} relay_matrix_io_t;

typedef struct {
    relay_matrix_io_t io;
    uint8_t selected_channel;
    bool interlock_fault;
    uint32_t fault_epoch;
} relay_matrix_t;

void relay_matrix_init(relay_matrix_t *relay, const relay_matrix_io_t *io);
void relay_matrix_emergency_off(relay_matrix_t *relay);
bool relay_matrix_select(relay_matrix_t *relay, uint8_t channel);
