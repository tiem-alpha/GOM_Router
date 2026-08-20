#pragma once

#include "gom_firmware.h"

/** Initialise the application-owned scpi-parser context. */
void router_scpi_init(void);
/** Parse one complete PC program message in the router owner task. */
void router_scpi_input(const char *line);
/** Deliver an asynchronous GOM completion to the pending SCPI query. */
void router_scpi_gom_complete(const gom_operation_t *operation,
                              gom_result_t result, const char *response);
