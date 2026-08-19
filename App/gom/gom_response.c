#include "gom_response.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static bool copy_field(char *out, size_t out_size, const char *start, size_t length)
{
    if (length == 0u || length >= out_size) return false;
    memcpy(out, start, length);
    out[length] = '\0';
    return true;
}

gom_measurement_status_t gom_parse_measurement(const char *text, double *value_ohm)
{
    char *end;
    double value;

    if (text == NULL || value_ohm == NULL || *text == '\0') return GOM_RESPONSE_INVALID;
    errno = 0;
    value = strtod(text, &end);
    if (end == text || errno == ERANGE || !isfinite(value)) return GOM_RESPONSE_INVALID;
    while (isspace((unsigned char)*end)) ++end;
    if (*end != '\0') return GOM_RESPONSE_INVALID;
    if (value == 9.0e9) return GOM_RESPONSE_OVER_RANGE;
    if (value == 9.9999e9) return GOM_RESPONSE_HVP;
    *value_ohm = value;
    return GOM_RESPONSE_NUMBER;
}

bool gom_parse_identity(const char *text, char *model, size_t model_size,
                        char *serial, size_t serial_size, char *firmware, size_t firmware_size)
{
    const char *first;
    const char *second;
    const char *third;

    if (text == NULL || model == NULL || serial == NULL || firmware == NULL) return false;
    first = strchr(text, ',');
    if (first == NULL) return false;
    second = strchr(first + 1, ',');
    if (second == NULL) return false;
    third = strchr(second + 1, ',');
    if (third == NULL || strchr(third + 1, ',') != NULL) return false;
    return copy_field(model, model_size, first + 1, (size_t)(second - first - 1)) &&
           copy_field(serial, serial_size, second + 1, (size_t)(third - second - 1)) &&
           copy_field(firmware, firmware_size, third + 1, strlen(third + 1));
}
