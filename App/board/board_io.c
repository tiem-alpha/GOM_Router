#include "board_io.h"
#include "board_shift_register.h"

bool board_route_request_image(void *context, uint16_t image)
{
    (void)context;
    return board_shift_register_request(image);
}
bool board_route_busy(void *context) { (void)context; return board_shift_register_busy(); }
bool board_route_failed(void *context) { (void)context; return board_shift_register_failed(); }
