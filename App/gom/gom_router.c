#include "gom_router.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *header;
    gom_command_id_t id;
    uint32_t caps;
    gom_value_kind_t value;
    double minimum, maximum;
    gom_verification_t verification;
} command_spec_t;

#define CMD(h, i, c, v, lo, hi) { h, i, c, v, lo, hi, GOM_VERIFY_HIL }
#define PENDING(h, i, c, v, lo, hi) { h, i, c, v, lo, hi, GOM_VERIFY_PENDING }
/* This is the production whitelist.  Add a command only with a typed rule here. */
static const command_spec_t commands[] = {
 CMD("SYST:DEV:IDN",GOM_CMD_IDN,0,GOM_VALUE_NONE,0,0), CMD("SYST:DEV:ERR",GOM_CMD_DEVICE_ERROR,0,GOM_VALUE_NONE,0,0),
 CMD("CONF:RES",GOM_CMD_CONFIG_RES,GOM_CAP_OHM,GOM_VALUE_NUMBER,GOM_RANGE_MIN_OHM,GOM_RANGE_MAX_OHM),
 CMD("SENS:FUNC",GOM_CMD_FUNCTION,GOM_CAP_OHM,GOM_VALUE_TOKEN,0,0), CMD("SENS:AUTO",GOM_CMD_AUTO,GOM_CAP_OHM,GOM_VALUE_BOOL,0,0), CMD("SENS:RANG",GOM_CMD_RANGE,GOM_CAP_OHM,GOM_VALUE_NUMBER,GOM_RANGE_MIN_OHM,GOM_RANGE_MAX_OHM),
 CMD("SENS:SPE",GOM_CMD_SPEED,GOM_CAP_OHM,GOM_VALUE_TOKEN,0,0), CMD("SENS:REL:STAT",GOM_CMD_REL_STATE,GOM_CAP_OHM,GOM_VALUE_BOOL,0,0), CMD("SENS:REL:DAT",GOM_CMD_REL_DATA,GOM_CAP_OHM,GOM_VALUE_NUMBER,-GOM_RANGE_MAX_OHM,GOM_RANGE_MAX_OHM),
 CMD("SENS:REAL:STAT",GOM_CMD_REALTIME,GOM_CAP_OHM,GOM_VALUE_BOOL,0,0), CMD("SENS:DISP",GOM_CMD_DISPLAY,GOM_CAP_OHM,GOM_VALUE_BOOL,0,0), CMD("READ",GOM_CMD_READ,GOM_CAP_OHM,GOM_VALUE_NONE,0,0),
 CMD("TRIG:SOUR",GOM_CMD_TRIGGER_SOURCE,GOM_CAP_OHM,GOM_VALUE_TOKEN,0,0), PENDING("TRIG:DEL:STAT",GOM_CMD_TRIGGER_DELAY_STATE,GOM_CAP_OHM,GOM_VALUE_BOOL,0,0), PENDING("TRIG:DEL:DAT",GOM_CMD_TRIGGER_DELAY_DATA,GOM_CAP_OHM,GOM_VALUE_INTEGER,0,1000), CMD("TRIG:EDGE",GOM_CMD_TRIGGER_EDGE,GOM_CAP_OHM,GOM_VALUE_TOKEN,0,0), CMD("*TRG",GOM_CMD_TRIGGER,GOM_CAP_OHM,GOM_VALUE_NONE,0,0),
 CMD("CALC:COMP:TYPE",GOM_CMD_COMP_TYPE,GOM_CAP_COMPARE,GOM_VALUE_TOKEN,0,0), CMD("CALC:COMP:LIM:REF",GOM_CMD_COMP_REF,GOM_CAP_COMPARE,GOM_VALUE_NUMBER,-GOM_RANGE_MAX_OHM,GOM_RANGE_MAX_OHM), CMD("CALC:COMP:LIM:MODE",GOM_CMD_COMP_MODE,GOM_CAP_COMPARE,GOM_VALUE_TOKEN,0,0), CMD("CALC:COMP:LIM:LOW",GOM_CMD_COMP_LOW,GOM_CAP_COMPARE,GOM_VALUE_NUMBER,-GOM_RANGE_MAX_OHM,GOM_RANGE_MAX_OHM), CMD("CALC:COMP:LIM:UPP",GOM_CMD_COMP_UPP,GOM_CAP_COMPARE,GOM_VALUE_NUMBER,-GOM_RANGE_MAX_OHM,GOM_RANGE_MAX_OHM), CMD("CALC:COMP:PERC:LOW",GOM_CMD_COMP_PLOW,GOM_CAP_COMPARE,GOM_VALUE_NUMBER,-1000,1000), CMD("CALC:COMP:PERC:UPP",GOM_CMD_COMP_PUPP,GOM_CAP_COMPARE,GOM_VALUE_NUMBER,-1000,1000), CMD("CALC:COMP:BEEP",GOM_CMD_COMP_BEEP,GOM_CAP_COMPARE,GOM_VALUE_TOKEN,0,0), CMD("CALC:COMP:MATH:DAT",GOM_CMD_COMP_MATH,GOM_CAP_COMPARE,GOM_VALUE_NONE,0,0), CMD("CALC:COMP:LIM:RES",GOM_CMD_COMP_RESULT,GOM_CAP_COMPARE,GOM_VALUE_NONE,0,0),
 PENDING("BINN:COUN:CLE",GOM_CMD_BIN_CLEAR,GOM_CAP_BINNING,GOM_VALUE_NONE,0,0), PENDING("BINN:COUN:TOT",GOM_CMD_BIN_TOTAL,GOM_CAP_BINNING,GOM_VALUE_NONE,0,0), PENDING("BINN:COUN:OUT",GOM_CMD_BIN_OUT,GOM_CAP_BINNING,GOM_VALUE_NONE,0,0), PENDING("BINN#:COUN:RES",GOM_CMD_BIN_COUNT,GOM_CAP_BINNING,GOM_VALUE_NONE,0,0), PENDING("BINN#:LIM:LOW",GOM_CMD_BIN_LOW,GOM_CAP_BINNING,GOM_VALUE_NUMBER,-GOM_RANGE_MAX_OHM,GOM_RANGE_MAX_OHM), PENDING("BINN#:LIM:UPP",GOM_CMD_BIN_UPP,GOM_CAP_BINNING,GOM_VALUE_NUMBER,-GOM_RANGE_MAX_OHM,GOM_RANGE_MAX_OHM), PENDING("BINN#:PERC:LOW",GOM_CMD_BIN_PLOW,GOM_CAP_BINNING,GOM_VALUE_NUMBER,-1000,1000), PENDING("BINN#:PERC:UPP",GOM_CMD_BIN_PUPP,GOM_CAP_BINNING,GOM_VALUE_NUMBER,-1000,1000), PENDING("BINN:LIM:BEEP",GOM_CMD_BIN_BEEP,GOM_CAP_BINNING,GOM_VALUE_TOKEN,0,0), PENDING("BINN:LIM:DISP",GOM_CMD_BIN_DISPLAY,GOM_CAP_BINNING,GOM_VALUE_TOKEN,0,0), PENDING("BINN:LIM:MODE",GOM_CMD_BIN_MODE,GOM_CAP_BINNING,GOM_VALUE_TOKEN,0,0), PENDING("BINN:LIM:REF",GOM_CMD_BIN_REFERENCE,GOM_CAP_BINNING,GOM_VALUE_NUMBER,-GOM_RANGE_MAX_OHM,GOM_RANGE_MAX_OHM), PENDING("BINN:LIM:RES",GOM_CMD_BIN_RESULT,GOM_CAP_BINNING,GOM_VALUE_NONE,0,0),
 CMD("TEMP:COMP:CORR",GOM_CMD_TEMP_COMP_CORRECT,GOM_CAP_TEMP,GOM_VALUE_NUMBER,-50,399.9), CMD("TEMP:COMP:COEF",GOM_CMD_TEMP_COMP_COEF,GOM_CAP_TEMP,GOM_VALUE_INTEGER,-9999,9999), CMD("TEMP:CONV:RES",GOM_CMD_TEMP_CONV_RES,GOM_CAP_TEMP,GOM_VALUE_NUMBER,0,GOM_RANGE_MAX_OHM), CMD("TEMP:CONV:TEMP",GOM_CMD_TEMP_CONV_TEMP,GOM_CAP_TEMP,GOM_VALUE_NUMBER,-50,399.9), CMD("TEMP:CONV:CONS",GOM_CMD_TEMP_CONV_CONST,GOM_CAP_TEMP,GOM_VALUE_NUMBER,0,10000), CMD("TEMP:CONV:DISP",GOM_CMD_TEMP_CONV_DISPLAY,GOM_CAP_TEMP,GOM_VALUE_INTEGER,0,2), CMD("TEMP:CONV:MATH:DAT",GOM_CMD_TEMP_CONV_MATH,GOM_CAP_TEMP,GOM_VALUE_NONE,0,0), CMD("TEMP:STAT",GOM_CMD_TEMP_STATE,GOM_CAP_TEMP,GOM_VALUE_BOOL,0,0), CMD("TEMP:DAT",GOM_CMD_TEMP_DATA,GOM_CAP_TEMP,GOM_VALUE_NONE,0,0), CMD("TEMP:UNIT",GOM_CMD_TEMP_UNIT,GOM_CAP_TEMP,GOM_VALUE_TOKEN,0,0), CMD("TEMP:AMB:STAT",GOM_CMD_TEMP_AMBIENT_STATE,GOM_CAP_TEMP,GOM_VALUE_BOOL,0,0), CMD("TEMP:AMB:DAT",GOM_CMD_TEMP_AMBIENT_DATA,GOM_CAP_TEMP,GOM_VALUE_NUMBER,-50,399.9),
 CMD("SYST:AVER:STAT",GOM_CMD_AVERAGE_STATE,GOM_CAP_OHM,GOM_VALUE_BOOL,0,0), CMD("SYST:AVER:DAT",GOM_CMD_AVERAGE_DATA,GOM_CAP_OHM,GOM_VALUE_INTEGER,2,100), PENDING("SYST:MDEL:STAT",GOM_CMD_MDELAY_STATE,GOM_CAP_OHM,GOM_VALUE_BOOL,0,0), PENDING("SYST:MDEL:DAT",GOM_CMD_MDELAY_DATA,GOM_CAP_OHM,GOM_VALUE_NUMBER,0,100000), CMD("SYST:LFR",GOM_CMD_LINE_FREQ,GOM_CAP_OHM,GOM_VALUE_TOKEN,0,0), PENDING("SYST:PWM:ON",GOM_CMD_PWM_ON,GOM_CAP_PWM,GOM_VALUE_INTEGER,3,99), PENDING("SYST:PWM:OFF",GOM_CMD_PWM_OFF,GOM_CAP_PWM,GOM_VALUE_INTEGER,100,9999), PENDING("SOUR:DRY",GOM_CMD_DRY,GOM_CAP_DRY,GOM_VALUE_BOOL,0,0), PENDING("SOUR:DRIV",GOM_CMD_DRIVE,GOM_CAP_DRIVE,GOM_VALUE_INTEGER,1,6)
};

static void upper_copy(char *out, size_t size, const char *in) { size_t i = 0; while (*in && i + 1u < size) out[i++] = (char)toupper((unsigned char)*in++); out[i] = '\0'; }
static bool is_token(const char *s) { for (; *s; ++s) if (!isalnum((unsigned char)*s) && *s != '_' && *s != '-') return false; return true; }
static bool token_one_of(const char *token, const char *choices) { const char *p=choices; size_t n=strlen(token); while(*p) { const char *end=strchr(p,'|'); size_t length=end?(size_t)(end-p):strlen(p); if(length==n&&!memcmp(p,token,n))return true; p=end?end+1:p+length; } return false; }
static bool valid_token(gom_command_id_t id, const char *token) {
    switch(id) {
    case GOM_CMD_FUNCTION:return token_one_of(token,"OHM|COMP|BIN|TC|TCONV|DIODE");
    case GOM_CMD_SPEED:return token_one_of(token,"FAST|SLOW"); case GOM_CMD_TRIGGER_SOURCE:return token_one_of(token,"INT|EXT"); case GOM_CMD_TRIGGER_EDGE:return token_one_of(token,"RISING|FALLING");
    case GOM_CMD_COMP_TYPE:return token_one_of(token,"OHM|TC"); case GOM_CMD_COMP_MODE:return token_one_of(token,"ABS|DPER|PER"); case GOM_CMD_COMP_BEEP: case GOM_CMD_BIN_BEEP:return token_one_of(token,"OFF|PASS|FAIL");
    case GOM_CMD_BIN_DISPLAY:return token_one_of(token,"COMP|COUNT"); case GOM_CMD_BIN_MODE:return token_one_of(token,"ABS|DPER"); case GOM_CMD_TEMP_UNIT:return token_one_of(token,"DEGC|DEGF"); case GOM_CMD_LINE_FREQ:return token_one_of(token,"AUTO|50|60"); default:return false;
    }
}
static bool match_header(const char *input, const char *pattern, uint8_t *index) { const char *a=input,*b=pattern; *index=0; while (*a && *b) { if (*b == '#') { if (*a < '1' || *a > '8') return false; *index=(uint8_t)(*a++-'0'); ++b; } else if (*a++ != *b++) return false; } return *a=='\0' && *b=='\0'; }
static const command_spec_t *find_command(const char *header, uint8_t *index) { size_t i; for (i=0;i<sizeof commands/sizeof commands[0];++i) if (match_header(header,commands[i].header,index)) return &commands[i]; return NULL; }
static uint32_t capabilities(gom_model_t m) { return m==GOM_MODEL_805 ? GOM_CAP_OHM|GOM_CAP_COMPARE|GOM_CAP_TEMP|GOM_CAP_BINNING|GOM_CAP_DRY|GOM_CAP_DRIVE|GOM_CAP_PWM : m==GOM_MODEL_804 ? GOM_CAP_OHM|GOM_CAP_COMPARE|GOM_CAP_TEMP : 0; }

void gom_router_init(gom_router_t *r)
{
    uint8_t channel;

    memset(r, 0, sizeof *r);
    r->timeout_ms = 5000u;
    for (channel = 0u; channel < GOM_CHANNEL_COUNT; ++channel) {
        r->lower_limit_ohm[channel] = GOM_RANGE_MIN_OHM;
        r->upper_limit_ohm[channel] = GOM_RANGE_MAX_OHM;
    }
}

void gom_router_set_device(gom_router_t *r, uint8_t channel, gom_model_t model, bool identified)
{
    if (channel != 0u && channel <= GOM_CHANNEL_COUNT) {
        gom_device_t *d = &r->devices[channel - 1u];
        d->model = model;
        d->identified = identified;
        d->online = identified;
        d->desynchronized = !identified;
        d->capabilities = identified ? capabilities(model) : 0u;
    }
}

void gom_router_mark_configuration_loaded(gom_router_t *r, uint8_t channel, bool loaded)
{
    if (channel != 0u && channel <= GOM_CHANNEL_COUNT) {
        r->devices[channel - 1u].configuration_loaded = loaded;
    }
}

void gom_router_set_limits(gom_router_t *r, uint8_t channel, double lower_ohm, double upper_ohm)
{
    if (channel != 0u && channel <= GOM_CHANNEL_COUNT && isfinite(lower_ohm) &&
        isfinite(upper_ohm) && lower_ohm <= upper_ohm) {
        r->lower_limit_ohm[channel - 1u] = lower_ohm;
        r->upper_limit_ohm[channel - 1u] = upper_ohm;
        r->limits_enabled[channel - 1u] = true;
    }
}

bool gom_router_value_in_limits(const gom_router_t *r, uint8_t channel, double value_ohm)
{
    if (channel == 0u || channel > GOM_CHANNEL_COUNT || !isfinite(value_ohm)) return false;
    return !r->limits_enabled[channel - 1u] ||
           (value_ohm >= r->lower_limit_ohm[channel - 1u] &&
            value_ohm <= r->upper_limit_ohm[channel - 1u]);
}

void gom_router_push_error(gom_router_t *r, int16_t code, const char *text)
{
    uint8_t index;

    if (r->error_count == (uint8_t)(sizeof r->error_codes / sizeof r->error_codes[0])) {
        r->error_head = (uint8_t)((r->error_head + 1u) % (sizeof r->error_codes / sizeof r->error_codes[0]));
        r->error_count--;
    }
    index = (uint8_t)((r->error_head + r->error_count) % (sizeof r->error_codes / sizeof r->error_codes[0]));
    r->error_codes[index] = code;
    (void)snprintf(r->error_text[index], sizeof r->error_text[index], "%s", text);
    r->error_count++;
}

void gom_router_pop_error(gom_router_t *r, char *out, size_t out_size)
{
    uint8_t index;

    if (r->error_count == 0u) {
        (void)snprintf(out, out_size, "0,No error");
        return;
    }
    index = r->error_head;
    (void)snprintf(out, out_size, "%d,%s", (int)r->error_codes[index], r->error_text[index]);
    r->error_head = (uint8_t)((r->error_head + 1u) % (sizeof r->error_codes / sizeof r->error_codes[0]));
    r->error_count--;
}

gom_router_status_t gom_router_execute(gom_router_t *r, const char *message, gom_operation_t *op) {
    char line[GOM_PC_MESSAGE_MAX+1], argument[80], *space; const command_spec_t *spec; char *end; uint8_t index=0; bool query;
    if (!r || !message || !op || strlen(message)>GOM_PC_MESSAGE_MAX) return GOM_ROUTER_ERR_SYNTAX;
    upper_copy(line,sizeof line,message); if (strchr(line,';')) return GOM_ROUTER_ERR_COMPOUND;
    while (isspace((unsigned char)*line)) memmove(line,line+1,strlen(line));
    space=strpbrk(line," \t\r\n"); argument[0]='\0'; if(space) { size_t count; *space++='\0'; while(isspace((unsigned char)*space)) ++space; count=strlen(space); if(count>=sizeof argument) return GOM_ROUTER_ERR_SYNTAX; memcpy(argument,space,count+1u); for (end=argument+strlen(argument);end>argument&&isspace((unsigned char)end[-1]);) *--end='\0'; }
    query=strlen(line)>0u && line[strlen(line)-1u]=='?'; if(query) line[strlen(line)-1u]='\0';
    /* Router-local IEEE-488.2 commands never require a selected GOM and are
       consumed by router_scpi_def.c rather than emitted on UART_GOM. */
    if (!strcmp(line,"*IDN") && query && !*argument) { memset(op,0,sizeof *op); op->query=true; return GOM_ROUTER_OK; }
    if (!strcmp(line,"*TST") && query && !*argument) { memset(op,0,sizeof *op); op->query=true; op->integer=0; return GOM_ROUTER_OK; }
    if (!strcmp(line,"*RST") && !query && !*argument) { r->selected_channel=0; memset(op,0,sizeof *op); return GOM_ROUTER_OK; }
    if (!strcmp(line,"SYST:ERR") && query && !*argument) { memset(op,0,sizeof *op); op->query=true; return GOM_ROUTER_OK; }
    if (!strcmp(line,"ROUT:CHAN")) { if(query) { memset(op,0,sizeof *op); op->integer=r->selected_channel; op->query=true; return GOM_ROUTER_OK; } long ch=strtol(argument,&end,10); if (*argument=='\0'||*end||ch<1||ch>8) return GOM_ROUTER_ERR_RANGE; r->selected_channel=(uint8_t)ch; memset(op,0,sizeof *op); op->integer=(int32_t)ch; return GOM_ROUTER_OK; }
    if (!strcmp(line,"ROUT:OPEN:ALL")) { if(query||*argument) return GOM_ROUTER_ERR_SYNTAX; r->selected_channel=0; return GOM_ROUTER_OK; }
    if (!strcmp(line,"ROUT:LIM:LOW") || !strcmp(line,"ROUT:LIM:UPP")) {
        double value;
        if (r->selected_channel == 0u) return GOM_ROUTER_ERR_NO_CHANNEL;
        if (query) {
            memset(op, 0, sizeof *op);
            op->query = true;
            op->number = !strcmp(line, "ROUT:LIM:LOW") ? r->lower_limit_ohm[r->selected_channel - 1u] : r->upper_limit_ohm[r->selected_channel - 1u];
            return GOM_ROUTER_OK;
        }
        value = strtod(argument, &end);
        if (*argument == '\0' || *end != '\0' || !isfinite(value) || value < GOM_RANGE_MIN_OHM || value > GOM_RANGE_MAX_OHM) return GOM_ROUTER_ERR_RANGE;
        if ((!strcmp(line, "ROUT:LIM:LOW") && value > r->upper_limit_ohm[r->selected_channel - 1u]) ||
            (!strcmp(line, "ROUT:LIM:UPP") && value < r->lower_limit_ohm[r->selected_channel - 1u])) return GOM_ROUTER_ERR_RANGE;
        if (!strcmp(line, "ROUT:LIM:LOW")) r->lower_limit_ohm[r->selected_channel - 1u] = value;
        else r->upper_limit_ohm[r->selected_channel - 1u] = value;
        r->limits_enabled[r->selected_channel - 1u] = true;
        memset(op, 0, sizeof *op);
        op->number = value;
        return GOM_ROUTER_OK;
    }
    if (!strcmp(line,"SYST:COMM:TIMEOUT")) { if(query) { memset(op,0,sizeof *op);op->query=true;op->integer=(int32_t)r->timeout_ms;return GOM_ROUTER_OK;} long t=strtol(argument,&end,10); if(*argument=='\0'||*end||t<GOM_QUERY_TIMEOUT_MIN_MS||t>GOM_QUERY_TIMEOUT_MAX_MS)return GOM_ROUTER_ERR_RANGE;r->timeout_ms=(uint32_t)t;return GOM_ROUTER_OK; }
    if (!r->selected_channel) return GOM_ROUTER_ERR_NO_CHANNEL;
    spec=find_command(line,&index); if(!spec) return GOM_ROUTER_ERR_UNDEFINED;
    if (spec->verification != GOM_VERIFY_HIL) return GOM_ROUTER_ERR_HIL_PENDING;
    if ((r->devices[r->selected_channel-1u].capabilities & spec->caps) != spec->caps) return GOM_ROUTER_ERR_CAPABILITY;
    memset(op,0,sizeof *op); op->id=spec->id;op->channel=r->selected_channel;op->index=index;op->query=query;
    if (query) {
        /* CONF:RES is a write-only macro; all other registered headers query normally. */
        if (*argument || (spec->value != GOM_VALUE_NONE && spec->id == GOM_CMD_CONFIG_RES)) return GOM_ROUTER_ERR_SYNTAX;
        return GOM_ROUTER_OK;
    }
    if (spec->value==GOM_VALUE_NONE) return *argument ? GOM_ROUTER_ERR_SYNTAX : GOM_ROUTER_OK;
    if (*argument=='\0') return GOM_ROUTER_ERR_SYNTAX;
    op->value_kind=spec->value;
    if (spec->value==GOM_VALUE_BOOL) { if(!strcmp(argument,"ON")||!strcmp(argument,"1"))op->boolean=true;else if(!strcmp(argument,"OFF")||!strcmp(argument,"0"))op->boolean=false;else return GOM_ROUTER_ERR_SYNTAX; }
    else if(spec->value==GOM_VALUE_TOKEN) { size_t count=strlen(argument); if(!is_token(argument)||count>=sizeof op->token||!valid_token(op->id,argument))return GOM_ROUTER_ERR_SYNTAX; memcpy(op->token,argument,count+1u); if(op->id==GOM_CMD_FUNCTION&&(!strcmp(argument,"BIN"))&&!(r->devices[r->selected_channel-1u].capabilities&GOM_CAP_BINNING))return GOM_ROUTER_ERR_CAPABILITY; }
    else if(op->id==GOM_CMD_CONFIG_RES && !strcmp(argument,"AUTO")) { op->value_kind=GOM_VALUE_BOOL;op->boolean=true; }
    else { double n=strtod(argument,&end); if(end==argument||*end||!isfinite(n)||n<spec->minimum||n>spec->maximum)return GOM_ROUTER_ERR_RANGE; op->number=n; op->integer=(int32_t)n; if(spec->value==GOM_VALUE_INTEGER && n!=(double)op->integer)return GOM_ROUTER_ERR_SYNTAX; }
    return GOM_ROUTER_OK;
}

const char *gom_router_status_text(gom_router_status_t s) { static const char *const text[]={"0,No error","-102,Syntax error","-222,Data out of range","100,No GOM channel selected","102,Unsupported command/model","102,Command awaits HIL verification","108,Only one command per message","-113,Undefined header"}; return s<=GOM_ROUTER_ERR_UNDEFINED?text[s]:"-360,Router error"; }
