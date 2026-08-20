#pragma once
#include <stdbool.h>
#include <stdint.h>

/* Board adapter: the only app module that binds the HC595 driver to relays. */
bool board_route_request_image(void *context, uint16_t image);
void board_route_emergency_disable(void *context);
bool board_route_busy(void *context);
bool board_route_failed(void *context);
