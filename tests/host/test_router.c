#include "gom_response.h"
#include "gom_router.h"
#include "relay_matrix.h"

#include <assert.h>
#include <string.h>

static uint16_t route_image;
static bool request_image(void *unused, uint16_t image)
{
    (void)unused;
    route_image = image;
    return true;
}

static bool is_busy(void *unused)
{
    (void)unused;
    return false;
}

static bool is_failed(void *unused)
{
    (void)unused;
    return false;
}

int main(void)
{
    gom_router_t router;
    gom_operation_t op;
    char wire[GOM_WIRE_COMMAND_MAX];
    char error[64];
    char model[16];
    char serial[16];
    char firmware[16];
    double value;

    gom_router_init(&router);
    assert(gom_router_execute(&router, "*IDN?", &op) == GOM_ROUTER_OK && op.channel == 0u);
    assert(gom_router_execute(&router, "READ?", &op) == GOM_ROUTER_ERR_NO_CHANNEL);
    assert(gom_router_execute(&router, "ROUT:CHAN 2", &op) == GOM_ROUTER_OK && op.integer == 2);
    gom_router_set_device(&router, 2u, GOM_MODEL_805, true);
    gom_router_mark_configuration_loaded(&router, 2u, true);
    assert(gom_router_execute(&router, "CONF:RES 0.005", &op) == GOM_ROUTER_OK);
    assert(gom_encode_operation(&op, wire, sizeof wire) == GOM_ROUTER_OK);
    assert(strstr(wire, "SENS:RANG 0.005") != NULL);
    assert(gom_router_execute(&router, "CONF:RES AUTO", &op) == GOM_ROUTER_OK);
    assert(gom_encode_operation(&op, wire, sizeof wire) == GOM_ROUTER_OK && strstr(wire, "SENS:AUTO ON") != NULL);
    assert(gom_router_execute(&router, "SENS:RANG 0.004", &op) == GOM_ROUTER_ERR_RANGE);
    assert(gom_router_execute(&router, "SENS:RANG NAN", &op) == GOM_ROUTER_ERR_RANGE);
    assert(gom_router_execute(&router, "SOUR:DRIV 3", &op) == GOM_ROUTER_ERR_HIL_PENDING);
    assert(gom_router_execute(&router, "READ?;*TRG", &op) == GOM_ROUTER_ERR_COMPOUND);
    assert(gom_router_execute(&router, "SENS:SPE FAST", &op) == GOM_ROUTER_OK);
    assert(gom_encode_operation(&op, wire, sizeof wire) == GOM_ROUTER_OK && !strcmp(wire, "SENS:SPE FAST\r\n"));
    assert(gom_router_execute(&router, "SENS:RANG?", &op) == GOM_ROUTER_OK);
    assert(gom_encode_operation(&op, wire, sizeof wire) == GOM_ROUTER_OK && !strcmp(wire, "SENS:RANG?\r\n"));

    assert(gom_router_execute(&router, "ROUT:LIM:LOW 1", &op) == GOM_ROUTER_OK);
    assert(gom_router_execute(&router, "ROUT:LIM:UPP 2", &op) == GOM_ROUTER_OK);
    assert(gom_router_value_in_limits(&router, 2u, 1.0));
    assert(!gom_router_value_in_limits(&router, 2u, 2.1));
    assert(gom_router_execute(&router, "ROUT:LIM:LOW?", &op) == GOM_ROUTER_OK && op.number == 1.0);

    assert(gom_parse_measurement("+2.2012E+0", &value) == GOM_RESPONSE_NUMBER && value > 2.2);
    assert(gom_parse_measurement("+9.0000E+9", &value) == GOM_RESPONSE_OVER_RANGE);
    assert(gom_parse_measurement("+9.9999E+9", &value) == GOM_RESPONSE_HVP);
    assert(gom_parse_measurement("NAN", &value) == GOM_RESPONSE_INVALID);
    assert(gom_parse_measurement("1.0 trailing", &value) == GOM_RESPONSE_INVALID);
    assert(gom_parse_identity("GOM,GOM-804,804001,V1.00", model, sizeof model, serial, sizeof serial,
                              firmware, sizeof firmware));
    assert(!strcmp(model, "GOM-804") && !strcmp(serial, "804001") && !strcmp(firmware, "V1.00"));

    gom_router_push_error(&router, -222, "Limit failure");
    gom_router_push_error(&router, -365, "Timeout");
    gom_router_pop_error(&router, error, sizeof error);
    assert(!strcmp(error, "-222,Limit failure"));
    gom_router_pop_error(&router, error, sizeof error);
    assert(!strcmp(error, "-365,Timeout"));

    {
        relay_matrix_t matrix;
        relay_matrix_io_t io = { request_image, is_busy, is_failed, NULL };
        relay_matrix_init(&matrix, &io);
        relay_matrix_step(&matrix, 0u);
        assert(relay_matrix_request(&matrix, 3u, 1u));
        relay_matrix_step(&matrix, 1u);
        relay_matrix_step(&matrix, 21u);
        relay_matrix_step(&matrix, 71u);
        assert(route_image == (3u << 4u) && matrix.selected_channel == 3u);
        relay_matrix_emergency_off(&matrix);
        relay_matrix_step(&matrix, 72u);
        assert(route_image == 0u);
    }
    return 0;
}
