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

static scpi_result_t cmd_idn(scpi_t *context)
{
    const gom_operation_t operation = {.id = GOM_OP_IDN, .query = true};
    return submit(context, &operation);
}

static scpi_result_t cmd_device_error(scpi_t *context)
{
    const gom_operation_t operation = {.id = GOM_OP_DEVICE_ERROR, .query = true};
    return submit(context, &operation);
}

static scpi_result_t cmd_read(scpi_t *context)
{
    const gom_operation_t operation = {.id = GOM_OP_READ, .query = true};
    return submit(context, &operation);
}

static scpi_result_t cmd_function(scpi_t *context)
{
    const scpi_choice_def_t choices[] = {{"OHM", 0}, {"COMP", 1}, {"BIN", 2},
                                         {"TC", 3}, {"TCONV", 4}, {"DIODE", 5},
                                         SCPI_CHOICE_LIST_END};
    static const char *const tokens[] = {"OHM", "COMP", "BIN", "TC", "TCONV", "DIODE"};
    gom_operation_t operation = {.id = GOM_OP_FUNCTION};
    int32_t choice;
    if (!SCPI_ParamChoice(context, choices, &choice, TRUE)) return SCPI_RES_ERR;
    memcpy(operation.token, tokens[choice], strlen(tokens[choice]) + 1u);
    return submit(context, &operation);
}

static scpi_result_t cmd_function_q(scpi_t *context)
{
    const gom_operation_t operation = {.id = GOM_OP_FUNCTION, .query = true};
    return submit(context, &operation);
}

static scpi_result_t cmd_auto(scpi_t *context)
{
    const scpi_choice_def_t choices[] = {{"OFF", 0}, {"ON", 1}, SCPI_CHOICE_LIST_END};
    gom_operation_t operation = {.id = GOM_OP_AUTO};
    int32_t enabled;
    if (!SCPI_ParamChoice(context, choices, &enabled, TRUE)) return SCPI_RES_ERR;
    operation.boolean = enabled != 0;
    return submit(context, &operation);
}

static scpi_result_t cmd_auto_q(scpi_t *context)
{
    const gom_operation_t operation = {.id = GOM_OP_AUTO, .query = true};
    return submit(context, &operation);
}

static scpi_result_t cmd_range(scpi_t *context)
{
    gom_operation_t operation = {.id = GOM_OP_RANGE};
    if (!SCPI_ParamDouble(context, &operation.number, TRUE) || !isfinite(operation.number) ||
        operation.number < GOM_RANGE_MIN_OHM || operation.number > GOM_RANGE_MAX_OHM) {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }
    return submit(context, &operation);
}

static scpi_result_t cmd_range_q(scpi_t *context)
{
    const gom_operation_t operation = {.id = GOM_OP_RANGE, .query = true};
    return submit(context, &operation);
}

static scpi_result_t cmd_speed(scpi_t *context)
{
    const scpi_choice_def_t choices[] = {{"FAST", 0}, {"SLOW", 1}, SCPI_CHOICE_LIST_END};
    gom_operation_t operation = {.id = GOM_OP_SPEED};
    int32_t choice;
    if (!SCPI_ParamChoice(context, choices, &choice, TRUE)) return SCPI_RES_ERR;
    memcpy(operation.token, choice == 0 ? "FAST" : "SLOW", 5u);
    return submit(context, &operation);
}

static scpi_result_t cmd_speed_q(scpi_t *context)
{
    const gom_operation_t operation = {.id = GOM_OP_SPEED, .query = true};
    return submit(context, &operation);
}

static scpi_result_t cmd_relative_state(scpi_t *context)
{
    const scpi_choice_def_t choices[] = {{"OFF", 0}, {"ON", 1}, SCPI_CHOICE_LIST_END};
    gom_operation_t operation = {.id = GOM_OP_RELATIVE_STATE};
    int32_t enabled;
    if (!SCPI_ParamChoice(context, choices, &enabled, TRUE)) return SCPI_RES_ERR;
    operation.boolean = enabled != 0;
    return submit(context, &operation);
}

static scpi_result_t cmd_relative_data(scpi_t *context)
{
    gom_operation_t operation = {.id = GOM_OP_RELATIVE_DATA};
    if (!SCPI_ParamDouble(context, &operation.number, TRUE) || !isfinite(operation.number)) return SCPI_RES_ERR;
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
    {.pattern = "SYSTem:DEVice:IDN?", .callback = cmd_idn},
    {.pattern = "SYSTem:DEVice:ERRor?", .callback = cmd_device_error},
    {.pattern = "READ?", .callback = cmd_read},
    {.pattern = "SENSe:FUNCtion", .callback = cmd_function},
    {.pattern = "SENSe:FUNCtion?", .callback = cmd_function_q},
    {.pattern = "SENSe:AUTo", .callback = cmd_auto},
    {.pattern = "SENSe:AUTo?", .callback = cmd_auto_q},
    {.pattern = "SENSe:RANGe", .callback = cmd_range},
    {.pattern = "SENSe:RANGe?", .callback = cmd_range_q},
    {.pattern = "SENSe:SPEed", .callback = cmd_speed},
    {.pattern = "SENSe:SPEed?", .callback = cmd_speed_q},
    {.pattern = "SENSe:RELative:STATe", .callback = cmd_relative_state},
    {.pattern = "SENSe:RELative:DATa", .callback = cmd_relative_data},
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
    if (line == NULL || pending_query || strchr(line, ';') != NULL) {
        SCPI_ErrorPush(&scpi_context, SCPI_ERROR_SYNTAX);
        return;
    }
    length = strlen(line);
    if (length == 0u || length > GOM_PC_LINE_MAX) return;
    (void)SCPI_Input(&scpi_context, line, (int)length);
    (void)SCPI_Input(&scpi_context, "\n", 1);
}

void router_scpi_gom_complete(const gom_operation_t *operation,
                              gom_result_t result, const char *response)
{
    double value;

    if (operation == NULL) return;
    pending_query = false;
    if (result != GOM_RESULT_OK) {
        SCPI_ErrorPush(&scpi_context, result == GOM_RESULT_TIMEOUT ?
                       SCPI_ERROR_QUERY_INTERRUPTED : SCPI_ERROR_COMMUNICATION_ERROR);
        return;
    }
    if (!operation->query) return;
    if (response == NULL) {
        SCPI_ErrorPush(&scpi_context, SCPI_ERROR_EXECUTION_ERROR);
        return;
    }
    if (operation->id == GOM_OP_READ || operation->id == GOM_OP_RANGE || operation->id == GOM_OP_RELATIVE_DATA) {
        if (!parse_response_number(response, &value) || value >= 9.0e9) {
            SCPI_ErrorPush(&scpi_context, SCPI_ERROR_EXECUTION_ERROR);
            return;
        }
        SCPI_ResultDouble(&scpi_context, value);
        (void)gom_firmware_pc_write("\r\n", 2u);
    } else {
        SCPI_ResultCharacters(&scpi_context, response, strlen(response));
        (void)gom_firmware_pc_write("\r\n", 2u);
    }
}
