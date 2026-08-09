#pragma once

/*
 * Board-only configuration.
 *
 * USART1 is the PC-facing RS-232 interface (PA9/PA10).  USART2 is the
 * switched GOM bus (PA2/PA3).  Keep this mapping here: application code must
 * never depend on GPIO names or CubeMX-generated handles directly.
 */
#include "main.h"

/* CubeMX owns the definitions in main.c; application modules only borrow them. */
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

#define BOARD_UART_PC             (&huart1)
#define BOARD_UART_GOM            (&huart2)

/* K1..K8 coil outputs.  A relay is selected only by board_relay_set_one(). */
#define BOARD_RELAY_PORT          GPIOB
#define BOARD_RELAY_ALL_PINS      (GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | \
                                   GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_12 | GPIO_PIN_13)

/* Change this once if the relay driver board is active-low. */
#define BOARD_RELAY_ACTIVE_STATE  GPIO_PIN_SET
#define BOARD_RELAY_OFF_STATE     GPIO_PIN_RESET

/* Bounded transactions: a wedged GOM must never hold the router indefinitely. */
#define BOARD_UART_BYTE_TIMEOUT_MS  100u
#define BOARD_UART_TX_TIMEOUT_MS   1000u
