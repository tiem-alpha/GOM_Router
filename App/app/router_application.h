#pragma once

#include "cmsis_os2.h"

/** Initialise the one-task router application after CubeMX peripherals. */
void router_application_init(void);
/** Create the sole router task with application-owned static storage. */
osThreadId_t router_application_create_task(osThreadFunc_t entry,
                                            const osThreadAttr_t *attributes);
/** Sole task entry: owns SCPI parsing, GOM transactions and relay switching. */
void router_application_task(void *argument);
