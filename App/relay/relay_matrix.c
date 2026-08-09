#include "relay_matrix.h"

#define RELAY_BREAK_MS 20u
#define RELAY_SETTLE_MS 50u

static bool feedback_is(const relay_matrix_t *relay, uint8_t expected) {
    return relay->io.feedback_mask == 0 || relay->io.feedback_mask(relay->io.context) == expected;
}

void relay_matrix_init(relay_matrix_t *relay, const relay_matrix_io_t *io) {
    relay->io = *io;
    relay->selected_channel = 0;
    relay->interlock_fault = false;
    relay->fault_epoch = 0;
    relay->io.all_off(relay->io.context);
}

void relay_matrix_emergency_off(relay_matrix_t *relay) {
    relay->io.all_off(relay->io.context);
    relay->selected_channel = 0;
    relay->fault_epoch++;
}

bool relay_matrix_select(relay_matrix_t *relay, uint8_t channel) {
    uint8_t expected;
    if (channel < 1u || channel > GOM_CHANNEL_COUNT || relay->interlock_fault) return false;
    if (relay->selected_channel == channel) return true;
    relay->io.all_off(relay->io.context);
    relay->io.delay_ms(relay->io.context, RELAY_BREAK_MS);
    if (!feedback_is(relay, 0u)) goto fault;
    relay->io.set_one(relay->io.context, channel);
    relay->io.delay_ms(relay->io.context, RELAY_SETTLE_MS);
    expected = (uint8_t)(1u << (channel - 1u));
    if (!feedback_is(relay, expected)) goto fault;
    relay->selected_channel = channel;
    return true;
fault:
    relay->io.all_off(relay->io.context);
    relay->selected_channel = 0;
    relay->interlock_fault = true;
    relay->fault_epoch++;
    return false;
}
