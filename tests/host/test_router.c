#include "gom_router.h"
#include "relay_matrix.h"
#include <assert.h>
#include <string.h>

static uint8_t relays;
static void all_off(void *unused) { (void)unused; relays=0; }
static void set_one(void *unused, uint8_t channel) { (void)unused; relays=(uint8_t)(1u<<(channel-1u)); }
static uint8_t feedback(void *unused) { (void)unused; return relays; }
static void no_delay(void *unused, uint32_t ms) { (void)unused; (void)ms; }

int main(void) {
    gom_router_t router; gom_operation_t op; char wire[GOM_WIRE_COMMAND_MAX];
    gom_router_init(&router);
    assert(gom_router_execute(&router,"*IDN?",&op)==GOM_ROUTER_OK && op.channel==0u);
    assert(gom_router_execute(&router,"READ?",&op)==GOM_ROUTER_ERR_NO_CHANNEL);
    assert(gom_router_execute(&router,"ROUT:CHAN 2",&op)==GOM_ROUTER_OK);
    gom_router_set_device(&router,2,GOM_MODEL_805,true);
    assert(gom_router_execute(&router,"CONF:RES 0.005",&op)==GOM_ROUTER_OK);
    assert(gom_encode_operation(&op,wire,sizeof wire)==GOM_ROUTER_OK);
    assert(strstr(wire,"SENS:RANG 0.005")!=NULL);
    assert(gom_router_execute(&router,"CONF:RES AUTO",&op)==GOM_ROUTER_OK);
    assert(gom_encode_operation(&op,wire,sizeof wire)==GOM_ROUTER_OK && strstr(wire,"SENS:AUTO ON")!=NULL);
    assert(gom_router_execute(&router,"SENS:RANG 0.004",&op)==GOM_ROUTER_ERR_RANGE);
    assert(gom_router_execute(&router,"SOUR:DRIV 3",&op)==GOM_ROUTER_ERR_HIL_PENDING);
    assert(gom_router_execute(&router,"READ?;*TRG",&op)==GOM_ROUTER_ERR_COMPOUND);
    assert(gom_router_execute(&router,"SENS:SPE FAST",&op)==GOM_ROUTER_OK);
    assert(gom_encode_operation(&op,wire,sizeof wire)==GOM_ROUTER_OK && !strcmp(wire,"SENS:SPE FAST\r\n"));
    assert(gom_router_execute(&router,"SENS:RANG?",&op)==GOM_ROUTER_OK);
    assert(gom_encode_operation(&op,wire,sizeof wire)==GOM_ROUTER_OK && !strcmp(wire,"SENS:RANG?\r\n"));
    { relay_matrix_t matrix; relay_matrix_io_t io={all_off,set_one,feedback,no_delay,0}; relay_matrix_init(&matrix,&io); assert(relay_matrix_select(&matrix,3)); assert(relays==4u && matrix.selected_channel==3u); relay_matrix_emergency_off(&matrix); assert(relays==0u); }
    return 0;
}
