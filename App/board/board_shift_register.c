#include "board_config.h"
#include "board_shift_register.h"

static uint16_t pending_image;
static uint8_t bit_index;
static bool busy;
static bool failed;
static bool enable_outputs_after_latch;

void board_shift_register_init(void)
{
    pending_image = 0u;
    bit_index = 0u;
    busy = false;
    failed = false;
    enable_outputs_after_latch = false;
#if BOARD_SHIFT_REGISTER_CONFIGURED
    HAL_GPIO_WritePin(BOARD_SHIFT_REGISTER_OE_PORT, BOARD_SHIFT_REGISTER_OE_PIN, GPIO_PIN_SET);
#endif
}

bool board_shift_register_request(uint16_t output_image)
{
    if (busy) return false;
#if BOARD_SHIFT_REGISTER_CONFIGURED
    pending_image = output_image;
    bit_index = 0u;
    HAL_GPIO_WritePin(BOARD_SHIFT_REGISTER_OE_PORT, BOARD_SHIFT_REGISTER_OE_PIN, GPIO_PIN_SET);
    enable_outputs_after_latch = true;
    busy = true;
    return true;
#else
    (void)output_image;
    failed = true; /* Fail closed until the physical HC595 pins are configured. */
    return false;
#endif
}

void board_shift_register_emergency_disable(void)
{
#if BOARD_SHIFT_REGISTER_CONFIGURED
    /* OE is hardware fail-safe: make the previous image electrically inert first. */
    HAL_GPIO_WritePin(BOARD_SHIFT_REGISTER_OE_PORT, BOARD_SHIFT_REGISTER_OE_PIN, GPIO_PIN_SET);
    pending_image = 0u;
    bit_index = 0u;
    enable_outputs_after_latch = false;
    busy = true;
#else
    failed = true;
#endif
}

void board_shift_register_step(void)
{
#if BOARD_SHIFT_REGISTER_CONFIGURED
    if (!busy) return;
    HAL_GPIO_WritePin(BOARD_SHIFT_REGISTER_DATA_PORT, BOARD_SHIFT_REGISTER_DATA_PIN,
                      (pending_image & (uint16_t)(1u << (15u - bit_index))) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BOARD_SHIFT_REGISTER_CLOCK_PORT, BOARD_SHIFT_REGISTER_CLOCK_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(BOARD_SHIFT_REGISTER_CLOCK_PORT, BOARD_SHIFT_REGISTER_CLOCK_PIN, GPIO_PIN_RESET);
    if (++bit_index == 16u) {
        HAL_GPIO_WritePin(BOARD_SHIFT_REGISTER_LATCH_PORT, BOARD_SHIFT_REGISTER_LATCH_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(BOARD_SHIFT_REGISTER_LATCH_PORT, BOARD_SHIFT_REGISTER_LATCH_PIN, GPIO_PIN_RESET);
        if (enable_outputs_after_latch)
            HAL_GPIO_WritePin(BOARD_SHIFT_REGISTER_OE_PORT, BOARD_SHIFT_REGISTER_OE_PIN, GPIO_PIN_RESET);
        busy = false;
    }
#endif
}

bool board_shift_register_busy(void) { return busy; }
bool board_shift_register_failed(void) { return failed; }
