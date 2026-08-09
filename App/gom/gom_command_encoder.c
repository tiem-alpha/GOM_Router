#include "gom_router.h"
#include <stdio.h>
#include <string.h>

static const char *const wire[] = {
    "*IDN?","SYST:ERR?",NULL,"SENS:FUNC","SENS:AUTO","SENS:RANG","SENS:SPE","SENS:REL:STAT","SENS:REL:DAT","SENS:REAL:STAT","SENS:DISP","READ?","TRIG:SOUR","TRIG:DEL:STAT","TRIG:DEL:DAT","TRIG:EDGE","*TRG","CALC:COMP:TYPE","CALC:COMP:LIM:REF","CALC:COMP:LIM:MODE","CALC:COMP:LIM:LOW","CALC:COMP:LIM:UPP","CALC:COMP:PERC:LOW","CALC:COMP:PERC:UPP","CALC:COMP:BEEP","CALC:COMP:MATH:DAT?","CALC:COMP:LIM:RES?","BINN:COUN:CLE","BINN:COUN:TOT?","BINN:COUN:OUT?",NULL,NULL,NULL,NULL,NULL,"BINN:LIM:BEEP","BINN:LIM:DISP","BINN:LIM:MODE","BINN:LIM:REF","BINN:LIM:RES?","TEMP:COMP:CORR","TEMP:COMP:COEF","TEMP:CONV:RES","TEMP:CONV:TEMP","TEMP:CONV:CONS","TEMP:CONV:DISP","TEMP:CONV:MATH:DAT?","TEMP:STAT","TEMP:DAT?","TEMP:UNIT","TEMP:AMB:STAT","TEMP:AMB:DAT","SYST:AVER:STAT","SYST:AVER:DAT","SYST:MDEL:STAT","SYST:MDEL:DAT","SYST:LFR","SYST:PWM:ON","SYST:PWM:OFF","SOUR:DRY","SOUR:DRIV" };

static const char *bin_header(gom_command_id_t id, uint8_t index) {
    static char header[24];
    const char *tail = id == GOM_CMD_BIN_COUNT ? "COUN:RES?" : id == GOM_CMD_BIN_LOW ? "LIM:LOW" : id == GOM_CMD_BIN_UPP ? "LIM:UPP" : id == GOM_CMD_BIN_PLOW ? "PERC:LOW" : "PERC:UPP";
    if (index < 1u || index > 8u) return NULL;
    (void)snprintf(header, sizeof header, "BINN%u:%s", index, tail);
    return header;
}

gom_router_status_t gom_encode_operation(const gom_operation_t *op, char *out, size_t size) {
    const char *header; int n;
    if (op == NULL || out == NULL || size == 0u) return GOM_ROUTER_ERR_SYNTAX;
    if (op->id == GOM_CMD_CONFIG_RES) {
        n = op->boolean ? snprintf(out, size, "SENS:FUNC OHM\r\nSENS:AUTO ON\r\n") : snprintf(out, size, "SENS:FUNC OHM\r\nSENS:AUTO OFF\r\nSENS:RANG %.9G\r\n", op->number);
        return n < 0 || (size_t)n >= size ? GOM_ROUTER_ERR_RANGE : GOM_ROUTER_OK;
    }
    header = (op->id >= GOM_CMD_BIN_COUNT && op->id <= GOM_CMD_BIN_PUPP) ? bin_header(op->id, op->index) : wire[op->id];
    if (header == NULL) return GOM_ROUTER_ERR_SYNTAX;
    if (op->query) n = snprintf(out, size, "%s%s\r\n", header, header[strlen(header) - 1u] == '?' ? "" : "?");
    else if (op->value_kind == GOM_VALUE_NONE) n = snprintf(out, size, "%s\r\n", header);
    else if (op->value_kind == GOM_VALUE_BOOL) n = snprintf(out, size, "%s %s\r\n", header, op->boolean ? "ON" : "OFF");
    else if (op->value_kind == GOM_VALUE_TOKEN) n = snprintf(out, size, "%s %s\r\n", header, op->token);
    else if (op->value_kind == GOM_VALUE_INTEGER) n = snprintf(out, size, "%s %ld\r\n", header, (long)op->integer);
    else n = snprintf(out, size, "%s %.9G\r\n", header, op->number);
    return n < 0 || (size_t)n >= size ? GOM_ROUTER_ERR_RANGE : GOM_ROUTER_OK;
}
