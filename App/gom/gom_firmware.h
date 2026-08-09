#pragma once

/* Initialise state after HAL peripherals are ready, then run forever in one
 * dedicated task.  The task is the sole owner of UART_GOM and relay outputs. */
void gom_firmware_init(void);
void gom_firmware_task(void *argument);

