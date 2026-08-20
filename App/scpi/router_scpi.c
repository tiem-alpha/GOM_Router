#include "router_scpi.h"

#include "protocol_limits.h"

#include "scpi/scpi.h"

#include <math.h>
#include <string.h>

#define ROUTER_SCPI_INPUT_SIZE 512u
#define ROUTER_SCPI_ERROR_QUEUE_SIZE 16u
#define ROUTER_SCPI_ERROR_INFO_SIZE 512u

static scpi_t scpi_context;
static char scpi_input_buffer[ROUTER_SCPI_INPUT_SIZE];
static char scpi_error_info[ROUTER_SCPI_ERROR_INFO_SIZE];
static scpi_error_t scpi_errors[ROUTER_SCPI_ERROR_QUEUE_SIZE];
static bool pending_query;

typedef bool (*gom_request_fn_t)(scpi_t *context, gom_operation_t *operation);

/*
 * The GOM command whitelist.  To add a command, add exactly one row here,
 * choose (or add) a typed request function, and add its wire encoding in
 * encode_operation().  The PC never supplies the GOM wire command directly.
 */
typedef struct {
    const char *pattern;
    gom_operation_id_t operation_id;
    bool query;
    gom_response_type_t response_type;
    gom_request_fn_t request;
} gom_command_map_t;

#define GOM_COMMAND_MAP(X) \
    X(IDN, "SYSTem:DEVice:IDN?", GOM_OP_IDN, true, GOM_RESPONSE_TEXT, request_none) \
    X(DEVICE_ERROR, "SYSTem:DEVice:ERRor?", GOM_OP_DEVICE_ERROR, true, GOM_RESPONSE_TEXT, request_none) \
    X(READ, "READ?", GOM_OP_READ, true, GOM_RESPONSE_NUMBER, request_none) \
    X(TRIGGER, "*TRG", GOM_OP_TRIGGER, false, GOM_RESPONSE_NONE, request_none) \
    X(FUNCTION, "SENSe:FUNCtion", GOM_OP_FUNCTION, false, GOM_RESPONSE_NONE, request_function) \
    X(FUNCTION_QUERY, "SENSe:FUNCtion?", GOM_OP_FUNCTION, true, GOM_RESPONSE_TEXT, request_none) \
    X(AUTO, "SENSe:AUTo", GOM_OP_AUTO, false, GOM_RESPONSE_NONE, request_boolean) \
    X(AUTO_QUERY, "SENSe:AUTo?", GOM_OP_AUTO, true, GOM_RESPONSE_TEXT, request_none) \
    X(RANGE, "SENSe:RANGe", GOM_OP_RANGE, false, GOM_RESPONSE_NONE, request_range) \
    X(RANGE_QUERY, "SENSe:RANGe?", GOM_OP_RANGE, true, GOM_RESPONSE_NUMBER, request_none) \
    X(SPEED, "SENSe:SPEed", GOM_OP_SPEED, false, GOM_RESPONSE_NONE, request_speed) \
    X(SPEED_QUERY, "SENSe:SPEed?", GOM_OP_SPEED, true, GOM_RESPONSE_TEXT, request_none) \
    X(RELATIVE_STATE, "SENSe:RELative:STATe", GOM_OP_RELATIVE_STATE, false, GOM_RESPONSE_NONE, request_boolean) \
    X(RELATIVE_STATE_QUERY, "SENSe:RELative:STATe?", GOM_OP_RELATIVE_STATE, true, GOM_RESPONSE_TEXT, request_none) \
    X(RELATIVE_DATA, "SENSe:RELative:DATa", GOM_OP_RELATIVE_DATA, false, GOM_RESPONSE_NONE, request_number) \
    X(RELATIVE_DATA_QUERY, "SENSe:RELative:DATa?", GOM_OP_RELATIVE_DATA, true, GOM_RESPONSE_NUMBER, request_none)

typedef enum {
#define GOM_MAP_ENUM(name, scpi_pattern, operation, query, response, request) GOM_MAP_##name,
    GOM_COMMAND_MAP(GOM_MAP_ENUM)
#undef GOM_MAP_ENUM
    GOM_MAP_COUNT
} gom_command_map_id_t;

static bool parse_response_number(const char *text, double *value)
{
    const char *cursor = text;
    double number = 0.0;
    double fraction = 0.1;
    int exponent = 0;
    int exponent_sign = 1;
    int sign = 1;
    bool digit = false;

    if (*cursor == '+' || *cursor == '-') {
        if (*cursor++ == '-') sign = -1;
    }
    while (*cursor >= '0' && *cursor <= '9') {
        digit = true;
        number = number * 10.0 + (double)(*cursor++ - '0');
    }
    if (*cursor == '.') {
        ++cursor;
        while (*cursor >= '0' && *cursor <= '9') {
            digit = true;
            number += (double)(*cursor++ - '0') * fraction;
            fraction *= 0.1;
        }
    }
    if (!digit) return false;
    if (*cursor == 'E' || *cursor == 'e') {
        ++cursor;
        if (*cursor == '+' || *cursor == '-') {
            if (*cursor++ == '-') exponent_sign = -1;
        }
        if (*cursor < '0' || *cursor > '9') return false;
        while (*cursor >= '0' && *cursor <= '9' && exponent < 309)
            exponent = exponent * 10 + (*cursor++ - '0');
        while (exponent-- > 0) number = exponent_sign > 0 ? number * 10.0 : number * 0.1;
    }
    if (*cursor != '\0' || !isfinite(number)) return false;
    *value = sign < 0 ? -number : number;
    return true;
}

static size_t scpi_write(scpi_t *context, const char *data, size_t length)
{
    (void)context;
    return gom_firmware_pc_write(data, length) ? length : 0u;
}

static scpi_result_t scpi_flush(scpi_t *context)
{
    (void)context;
    return SCPI_RES_OK;
}

static int scpi_error(scpi_t *context, int_fast16_t error)
{
    (void)context;
    (void)error;
    return 0;
}

static scpi_result_t scpi_control(scpi_t *context, scpi_ctrl_name_t ctrl, scpi_reg_val_t value)
{
    (void)context;
    (void)ctrl;
    (void)value;
    return SCPI_RES_OK;
}

static scpi_result_t scpi_reset(scpi_t *context)
{
    (void)context;
    gom_firmware_open_all();
    return SCPI_RES_OK;
}

static scpi_result_t reject_busy(scpi_t *context)
{
    if (pending_query || !gom_firmware_ready()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

static scpi_result_t submit(scpi_t *context, const gom_operation_t *operation)
{
    if (reject_busy(context) != SCPI_RES_OK) return SCPI_RES_ERR;
    if (!gom_firmware_submit(operation)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }
    pending_query = operation->query;
    return SCPI_RES_OK;
}

static scpi_result_t cmd_router_idn(scpi_t *context)
{
    static const char identity[] = "GOM850-ROUTER,STM32F411,0001,1.0";
    SCPI_ResultCharacters(context, identity, sizeof(identity) - 1u);
    return SCPI_RES_OK;
}

static scpi_result_t cmd_router_test(scpi_t *context)
{
    SCPI_ResultInt32(context, gom_firmware_ready() ? 0 : 1);
    return SCPI_RES_OK;
}

static scpi_result_t cmd_route_channel(scpi_t *context)
{
    int32_t channel;
    if (reject_busy(context) != SCPI_RES_OK) return SCPI_RES_ERR;
    if (!SCPI_ParamInt32(context, &channel, TRUE) || channel < 1 || channel > 8 ||
        !gom_firmware_select_channel((uint8_t)channel)) {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

static scpi_result_t cmd_route_channel_q(scpi_t *context)
{
    SCPI_ResultInt32(context, gom_firmware_selected_channel());
    return SCPI_RES_OK;
}

static scpi_result_t cmd_route_open_all(scpi_t *context)
{
    (void)context;
    gom_firmware_open_all();
    return SCPI_RES_OK;
}

static scpi_result_t cmd_timeout(scpi_t *context)
{
    int32_t timeout;
    if (!SCPI_ParamInt32(context, &timeout, TRUE) || !gom_firmware_set_timeout((uint32_t)timeout)) {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

static scpi_result_t cmd_timeout_q(scpi_t *context)
{
    SCPI_ResultUInt32(context, gom_firmware_timeout());
    return SCPI_RES_OK;
}

static bool request_none(scpi_t *context, gom_operation_t *operation)
{
    (void)context;
    (void)operation;
    return true;
}

static bool request_function(scpi_t *context, gom_operation_t *operation)
{
    const scpi_choice_def_t choices[] = {{"OHM", 0}, {"COMP", 1}, {"BIN", 2},
                                         {"TC", 3}, {"TCONV", 4}, {"SCAN", 5},
                                         {"DIODE", 6},
                                         SCPI_CHOICE_LIST_END};
    static const char *const tokens[] = {"OHM", "COMP", "BIN", "TC", "TCONV", "SCAN", "DIODE"};
    int32_t choice;
    if (!SCPI_ParamChoice(context, choices, &choice, TRUE)) return false;
    memcpy(operation->token, tokens[choice], strlen(tokens[choice]) + 1u);
    return true;
}

static bool request_boolean(scpi_t *context, gom_operation_t *operation)
{
    const scpi_choice_def_t choices[] = {{"OFF", 0}, {"ON", 1}, SCPI_CHOICE_LIST_END};
    int32_t enabled;
    if (!SCPI_ParamChoice(context, choices, &enabled, TRUE)) return false;
    operation->boolean = enabled != 0;
    return true;
}

static bool request_range(scpi_t *context, gom_operation_t *operation)
{
    if (!SCPI_ParamDouble(context, &operation->number, TRUE) || !isfinite(operation->number) ||
        operation->number < GOM_RANGE_MIN_OHM || operation->number > GOM_RANGE_MAX_OHM) {
        return false;
    }
    return true;
}

static bool request_number(scpi_t *context, gom_operation_t *operation)
{
    return SCPI_ParamDouble(context, &operation->number, TRUE) && isfinite(operation->number);
}

static bool request_speed(scpi_t *context, gom_operation_t *operation)
{
    const scpi_choice_def_t choices[] = {{"FAST", 0}, {"SLOW", 1}, SCPI_CHOICE_LIST_END};
    int32_t choice;
    if (!SCPI_ParamChoice(context, choices, &choice, TRUE)) return false;
    memcpy(operation->token, choice == 0 ? "FAST" : "SLOW", 5u);
    return true;
}

#define GOM_MAP_ENTRY(name, scpi_pattern, operation, query, response, request) \
    {scpi_pattern, operation, query, response, request},
static const gom_command_map_t gom_command_map[] = {GOM_COMMAND_MAP(GOM_MAP_ENTRY)};
#undef GOM_MAP_ENTRY

_Static_assert(sizeof(gom_command_map) / sizeof(gom_command_map[0]) == GOM_MAP_COUNT,
               "GOM command map tags must remain contiguous");

static scpi_result_t cmd_gom_mapped(scpi_t *context)
{
    const int32_t tag = SCPI_CmdTag(context);
    const gom_command_map_t *definition;
    gom_operation_t operation;

    if (tag < 0 || tag >= (int32_t)GOM_MAP_COUNT) return SCPI_RES_ERR;
    definition = &gom_command_map[tag];

    memset(&operation, 0, sizeof(operation));
    operation.id = definition->operation_id;
    operation.query = definition->query;
    operation.response_type = definition->response_type;
    if (!definition->request(context, &operation)) {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }
    return submit(context, &operation);
}

static const scpi_command_t commands[] = {
    {.pattern = "*CLS", .callback = SCPI_CoreCls},
    {.pattern = "*ESE", .callback = SCPI_CoreEse},
    {.pattern = "*ESE?", .callback = SCPI_CoreEseQ},
    {.pattern = "*ESR?", .callback = SCPI_CoreEsrQ},
    {.pattern = "*IDN?", .callback = cmd_router_idn},
    {.pattern = "*OPC", .callback = SCPI_CoreOpc},
    {.pattern = "*OPC?", .callback = SCPI_CoreOpcQ},
    {.pattern = "*RST", .callback = SCPI_CoreRst},
    {.pattern = "*SRE", .callback = SCPI_CoreSre},
    {.pattern = "*SRE?", .callback = SCPI_CoreSreQ},
    {.pattern = "*STB?", .callback = SCPI_CoreStbQ},
    {.pattern = "*TST?", .callback = cmd_router_test},
    {.pattern = "SYSTem:ERRor[:NEXT]?", .callback = SCPI_SystemErrorNextQ},
    {.pattern = "ROUTe:CHANnel", .callback = cmd_route_channel},
    {.pattern = "ROUTe:CHANnel?", .callback = cmd_route_channel_q},
    {.pattern = "ROUTe:OPEN:ALL", .callback = cmd_route_open_all},
    {.pattern = "SYSTem:COMM:TIMEout", .callback = cmd_timeout},
    {.pattern = "SYSTem:COMM:TIMEout?", .callback = cmd_timeout_q},
#define GOM_SCPI_ENTRY(name, scpi_pattern, operation, query, response, request) \
    {.pattern = scpi_pattern, .callback = cmd_gom_mapped, .tag = GOM_MAP_##name},
    GOM_COMMAND_MAP(GOM_SCPI_ENTRY)
#undef GOM_SCPI_ENTRY
    SCPI_CMD_LIST_END,
};

static scpi_interface_t interface = {
    .error = scpi_error,
    .write = scpi_write,
    .control = scpi_control,
    .flush = scpi_flush,
    .reset = scpi_reset,
};

void router_scpi_init(void)
{
    SCPI_Init(&scpi_context, commands, &interface, NULL,
              "GOM850-ROUTER", "STM32F411", "0001", "1.0",
              scpi_input_buffer, sizeof(scpi_input_buffer), scpi_errors,
              ROUTER_SCPI_ERROR_QUEUE_SIZE);
    SCPI_InitHeap(&scpi_context, scpi_error_info, sizeof(scpi_error_info));
}

void router_scpi_input(const char *line)
{
    size_t length;

    if (line == NULL || strchr(line, ';') != NULL) {
        SCPI_ErrorPush(&scpi_context, SCPI_ERROR_SYNTAX);
        return;
    }
    length = strlen(line);
    if (length == 0u || length > GOM_PC_LINE_MAX) return;
    /* Emergency open and reset are the only commands allowed to cancel a query. */
    if (pending_query && !SCPI_Match("ROUTe:OPEN:ALL", line, length) &&
        !SCPI_Match("*RST", line, length)) {
        SCPI_ErrorPush(&scpi_context, SCPI_ERROR_SYNTAX);
        return;
    }
    (void)SCPI_Input(&scpi_context, line, (int)length);
    (void)SCPI_Input(&scpi_context, "\n", 1);
}

void router_scpi_gom_complete(const gom_operation_t *operation,
                              gom_result_t result, const char *response)
{
    char pc_response[GOM_REPLY_LINE_MAX + 3u];
    double value;
    size_t response_length;

    if (operation == NULL) return;
    pending_query = false;
    if (result != GOM_RESULT_OK) {
        SCPI_ErrorPush(&scpi_context, result == GOM_RESULT_TIMEOUT || result == GOM_RESULT_CANCELLED ?
                       SCPI_ERROR_QUERY_INTERRUPTED : SCPI_ERROR_COMMUNICATION_ERROR);
        return;
    }
    if (!operation->query) return;
    if (response == NULL) {
        SCPI_ErrorPush(&scpi_context, SCPI_ERROR_EXECUTION_ERROR);
        return;
    }
    if (operation->response_type == GOM_RESPONSE_NUMBER) {
        if (!parse_response_number(response, &value) || value >= 9.0e9) {
            SCPI_ErrorPush(&scpi_context, SCPI_ERROR_EXECUTION_ERROR);
            return;
        }
        response_length = SCPI_DoubleToStr(value, pc_response, sizeof(pc_response) - 2u);
    } else {
        response_length = strlen(response);
        if (response_length > GOM_REPLY_LINE_MAX) {
            SCPI_ErrorPush(&scpi_context, SCPI_ERROR_EXECUTION_ERROR);
            return;
        }
        memcpy(pc_response, response, response_length);
    }
    if (response_length == 0u || response_length > GOM_REPLY_LINE_MAX) {
        SCPI_ErrorPush(&scpi_context, SCPI_ERROR_EXECUTION_ERROR);
        return;
    }
    pc_response[response_length++] = '\r';
    pc_response[response_length++] = '\n';
    if (!gom_firmware_pc_write(pc_response, response_length)) {
        SCPI_ErrorPush(&scpi_context, SCPI_ERROR_COMMUNICATION_ERROR);
    }
}
