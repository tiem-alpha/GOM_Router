#pragma once
#include <stdbool.h>
#include <stdint.h>

/* Non-blocking 2x 74HC595 driver.  It owns no relay policy. */
void board_shift_register_init(void);
bool board_shift_register_request(uint16_t output_image);
/** Disable HC595 outputs immediately and asynchronously latch the all-open image. */
void board_shift_register_emergency_disable(void);
void board_shift_register_step(void);
bool board_shift_register_busy(void);
bool board_shift_register_failed(void);
