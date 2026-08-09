#include "board_config.h"
#include "board_io.h"

/*
 * These writes use GPIO BSRR via HAL.  Clearing all coils before setting one
 * makes the software state one-hot even if a caller is faulty.  The relay
 * matrix adds the break-before-make delay and is the policy owner.
 */
void board_relay_all_off(void *context)
{
    (void)context;
    /* GPIO_PinState is an enum, so use C (not a preprocessor #if) here. */
    HAL_GPIO_WritePin(BOARD_RELAY_PORT, BOARD_RELAY_ALL_PINS,
                      BOARD_RELAY_ACTIVE_STATE == GPIO_PIN_SET ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void board_relay_set_one(void *context, uint8_t channel)
{
    uint16_t pin;
    (void)context;
    if (channel < 1u || channel > 8u) {
        board_relay_all_off(NULL);
        return;
    }
    /* The non-contiguous PB12/PB13 pins are deliberately explicit. */
    static const uint16_t pins[8] = { GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6,
                                      GPIO_PIN_7, GPIO_PIN_8, GPIO_PIN_12, GPIO_PIN_13 };
    pin = pins[channel - 1u];
    board_relay_all_off(NULL);
    HAL_GPIO_WritePin(BOARD_RELAY_PORT, pin, BOARD_RELAY_ACTIVE_STATE);
}

void board_delay_ms(void *context, uint32_t delay_ms)
{
    (void)context;
    HAL_Delay(delay_ms);
}
