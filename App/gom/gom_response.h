#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    GOM_RESPONSE_NUMBER,
    GOM_RESPONSE_OVER_RANGE,
    GOM_RESPONSE_HVP,
    GOM_RESPONSE_INVALID
} gom_measurement_status_t;

gom_measurement_status_t gom_parse_measurement(const char *text, double *value_ohm);
bool gom_parse_identity(const char *text, char *model, size_t model_size,
                        char *serial, size_t serial_size, char *firmware, size_t firmware_size);
