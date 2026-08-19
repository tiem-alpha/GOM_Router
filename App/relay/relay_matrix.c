#include "relay_matrix.h"

#define RELAY_BREAK_MS 20u
#define RELAY_SETTLE_MS 50u

static uint16_t route_mask(uint8_t channel)
{
    return (uint16_t)(3u << (2u * (channel - 1u)));
}

void relay_matrix_init(relay_matrix_t *relay, const relay_matrix_io_t *io)
{
    relay->io = *io;
    relay->selected_channel = 0u;
    relay->requested_channel = 0u;
    relay->state = RELAY_OPENING;
    relay->deadline_ms = 0u;
    relay->interlock_fault = false;
    relay->fault_epoch = 0u;
    if (!relay->io.request_image(relay->io.context, 0u)) {
        relay->interlock_fault = true;
        relay->state = RELAY_FAULT;
        relay->fault_epoch++;
    }
}

void relay_matrix_emergency_off(relay_matrix_t *relay)
{
    if (!relay->io.busy(relay->io.context) && !relay->io.request_image(relay->io.context, 0u)) relay->interlock_fault = true;
    relay->selected_channel = 0u;
    relay->requested_channel = 0u;
    relay->state = relay->interlock_fault ? RELAY_FAULT : RELAY_OPENING;
    relay->fault_epoch++;
}

bool relay_matrix_request(relay_matrix_t *relay, uint8_t channel, uint32_t now_ms)
{
    if (channel < 1u || channel > GOM_CHANNEL_COUNT || relay->interlock_fault) return false;
    if (relay->selected_channel == channel && relay->state == RELAY_IDLE) return true;
    if (relay->state != RELAY_IDLE) return false;
    relay->requested_channel = channel;
    relay->deadline_ms = now_ms;
    relay->state = RELAY_OPENING;
    return true;
}

void relay_matrix_step(relay_matrix_t *relay, uint32_t now_ms)
{
    if (relay->state == RELAY_IDLE || relay->state == RELAY_FAULT) return;
    if (relay->io.failed(relay->io.context)) goto fault;
    switch (relay->state) {
    case RELAY_OPENING:
        if (relay->io.busy(relay->io.context)) return;
        if (relay->requested_channel == 0u) { relay->state = RELAY_IDLE; return; }
        if (!relay->io.request_image(relay->io.context, 0u)) goto fault;
        relay->selected_channel = 0u;
        relay->deadline_ms = now_ms + RELAY_BREAK_MS;
        relay->state = RELAY_BREAKING;
        break;
    case RELAY_BREAKING:
        if (relay->io.busy(relay->io.context) || (int32_t)(now_ms - relay->deadline_ms) < 0) return;
        if (!relay->io.request_image(relay->io.context, route_mask(relay->requested_channel))) goto fault;
        relay->deadline_ms = now_ms + RELAY_SETTLE_MS;
        relay->state = RELAY_SETTLING;
        break;
    case RELAY_SETTLING:
        if (relay->io.busy(relay->io.context) || (int32_t)(now_ms - relay->deadline_ms) < 0) return;
        relay->selected_channel = relay->requested_channel;
        relay->state = RELAY_IDLE;
        break;
    default: break;
    }
    return;
fault:
    (void)relay->io.request_image(relay->io.context, 0u);
    relay->selected_channel = 0u;
    relay->interlock_fault = true;
    relay->fault_epoch++;
    relay->state = RELAY_FAULT;
}

bool relay_matrix_ready(const relay_matrix_t *relay)
{
    return relay->state == RELAY_IDLE && !relay->interlock_fault;
}
