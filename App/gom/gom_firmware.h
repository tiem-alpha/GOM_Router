#pragma once

/* One cooperative application owner. Call gom_firmware_step() continuously
 * from the only application task; UART callbacks only move bytes to rings. */
void gom_firmware_init(void);
void gom_firmware_step(void);
void gom_firmware_task(void *argument);
