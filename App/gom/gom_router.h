#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "protocol_limits.h"

typedef enum { GOM_MODEL_UNKNOWN, GOM_MODEL_804, GOM_MODEL_805 } gom_model_t;
enum { GOM_CAP_OHM = 1u << 0, GOM_CAP_COMPARE = 1u << 1, GOM_CAP_TEMP = 1u << 2,
       GOM_CAP_BINNING = 1u << 3, GOM_CAP_DRY = 1u << 4, GOM_CAP_DRIVE = 1u << 5,
       GOM_CAP_PWM = 1u << 6 };
typedef enum { GOM_VERIFY_HIL, GOM_VERIFY_PENDING } gom_verification_t;
typedef enum {
    GOM_CMD_IDN, GOM_CMD_DEVICE_ERROR, GOM_CMD_CONFIG_RES, GOM_CMD_FUNCTION, GOM_CMD_AUTO, GOM_CMD_RANGE,
    GOM_CMD_SPEED, GOM_CMD_REL_STATE, GOM_CMD_REL_DATA, GOM_CMD_REALTIME, GOM_CMD_DISPLAY,
    GOM_CMD_READ, GOM_CMD_TRIGGER_SOURCE, GOM_CMD_TRIGGER_DELAY_STATE, GOM_CMD_TRIGGER_DELAY_DATA,
    GOM_CMD_TRIGGER_EDGE, GOM_CMD_TRIGGER, GOM_CMD_COMP_TYPE, GOM_CMD_COMP_REF,
    GOM_CMD_COMP_MODE, GOM_CMD_COMP_LOW, GOM_CMD_COMP_UPP, GOM_CMD_COMP_PLOW, GOM_CMD_COMP_PUPP,
    GOM_CMD_COMP_BEEP, GOM_CMD_COMP_MATH, GOM_CMD_COMP_RESULT, GOM_CMD_BIN_CLEAR,
    GOM_CMD_BIN_TOTAL, GOM_CMD_BIN_OUT, GOM_CMD_BIN_COUNT, GOM_CMD_BIN_LOW, GOM_CMD_BIN_UPP,
    GOM_CMD_BIN_PLOW, GOM_CMD_BIN_PUPP, GOM_CMD_BIN_BEEP, GOM_CMD_BIN_DISPLAY, GOM_CMD_BIN_MODE,
    GOM_CMD_BIN_REFERENCE, GOM_CMD_BIN_RESULT, GOM_CMD_TEMP_COMP_CORRECT, GOM_CMD_TEMP_COMP_COEF,
    GOM_CMD_TEMP_CONV_RES, GOM_CMD_TEMP_CONV_TEMP, GOM_CMD_TEMP_CONV_CONST, GOM_CMD_TEMP_CONV_DISPLAY,
    GOM_CMD_TEMP_CONV_MATH, GOM_CMD_TEMP_STATE, GOM_CMD_TEMP_DATA, GOM_CMD_TEMP_UNIT,
    GOM_CMD_TEMP_AMBIENT_STATE, GOM_CMD_TEMP_AMBIENT_DATA, GOM_CMD_AVERAGE_STATE, GOM_CMD_AVERAGE_DATA,
    GOM_CMD_MDELAY_STATE, GOM_CMD_MDELAY_DATA, GOM_CMD_LINE_FREQ, GOM_CMD_PWM_ON, GOM_CMD_PWM_OFF,
    GOM_CMD_DRY, GOM_CMD_DRIVE
} gom_command_id_t;
typedef enum { GOM_VALUE_NONE, GOM_VALUE_BOOL, GOM_VALUE_NUMBER, GOM_VALUE_INTEGER, GOM_VALUE_TOKEN } gom_value_kind_t;
typedef struct { gom_command_id_t id; uint8_t channel; uint8_t index; bool query; gom_value_kind_t value_kind; double number; int32_t integer; bool boolean; char token[12]; } gom_operation_t;
typedef struct {
    bool identified;
    bool online;
    bool configuration_loaded;
    bool desynchronized;
    gom_model_t model;
    uint32_t capabilities;
    uint32_t communication_faults;
    char serial[16];
    char firmware[16];
} gom_device_t;
typedef enum { GOM_ROUTER_OK, GOM_ROUTER_ERR_SYNTAX, GOM_ROUTER_ERR_RANGE, GOM_ROUTER_ERR_NO_CHANNEL, GOM_ROUTER_ERR_CAPABILITY, GOM_ROUTER_ERR_HIL_PENDING, GOM_ROUTER_ERR_COMPOUND, GOM_ROUTER_ERR_UNDEFINED } gom_router_status_t;

typedef struct {
    uint8_t selected_channel;
    uint32_t timeout_ms;
    double lower_limit_ohm[GOM_CHANNEL_COUNT];
    double upper_limit_ohm[GOM_CHANNEL_COUNT];
    bool limits_enabled[GOM_CHANNEL_COUNT];
    int16_t error_codes[8];
    char error_text[8][40];
    uint8_t error_head;
    uint8_t error_count;
    gom_device_t devices[GOM_CHANNEL_COUNT];
} gom_router_t;

void gom_router_init(gom_router_t *router);
void gom_router_set_device(gom_router_t *router, uint8_t channel, gom_model_t model, bool identified);
void gom_router_mark_configuration_loaded(gom_router_t *router, uint8_t channel, bool loaded);
void gom_router_set_limits(gom_router_t *router, uint8_t channel, double lower_ohm, double upper_ohm);
bool gom_router_value_in_limits(const gom_router_t *router, uint8_t channel, double value_ohm);
void gom_router_push_error(gom_router_t *router, int16_t code, const char *text);
void gom_router_pop_error(gom_router_t *router, char *out, size_t out_size);
gom_router_status_t gom_router_execute(gom_router_t *router, const char *message, gom_operation_t *operation);
gom_router_status_t gom_encode_operation(const gom_operation_t *operation, char *out, size_t out_size);
const char *gom_router_status_text(gom_router_status_t status);
