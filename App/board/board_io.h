#pragma once

#include <stdint.h>

/* HAL/GPIO adapter.  This is the only application module allowed to know pins. */
void board_relay_all_off(void *context);
void board_relay_set_one(void *context, uint8_t channel);
void board_delay_ms(void *context, uint32_t delay_ms);

