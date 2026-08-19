#pragma once

/*
 * Board-only configuration.
 *
 * USART6 is the PC-facing RS-232 interface (PC6/PC7). USART2 is the switched
 * GOM bus (PA2/PA3). USART1 (PA9/PA10) is reserved for the debug CLI and the
 * STM32 ROM bootloader. Keep this mapping here: application code must never
 * depend on GPIO names or CubeMX-generated handles directly.
 */
#include "main.h"

/* CubeMX owns the definitions in main.c; application modules only borrow them. */
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart6;

#define BOARD_UART_PC             (&huart6)
#define BOARD_UART_GOM            (&huart2)
#define BOARD_UART_DEBUG          (&huart1)

/*
 * 74HC595 hardware is introduced only after CubeMX has generated the SPI and
 * latch/OE GPIO setup for STM32F411RE.  Until then this remains zero: every
 * route operation fails closed and no legacy direct-relay GPIO is driven.
 */
#define BOARD_SHIFT_REGISTER_CONFIGURED 0u
#define BOARD_SHIFT_REGISTER_COUNT      2u
#define BOARD_SHIFT_REGISTER_BITS       16u

#if BOARD_SHIFT_REGISTER_CONFIGURED
#define BOARD_SHIFT_REGISTER_DATA_PORT  GPIOA
#define BOARD_SHIFT_REGISTER_DATA_PIN   GPIO_PIN_7
#define BOARD_SHIFT_REGISTER_CLOCK_PORT GPIOA
#define BOARD_SHIFT_REGISTER_CLOCK_PIN  GPIO_PIN_5
#define BOARD_SHIFT_REGISTER_LATCH_PORT GPIOA
#define BOARD_SHIFT_REGISTER_LATCH_PIN  GPIO_PIN_4
#define BOARD_SHIFT_REGISTER_OE_PORT    GPIOB
#define BOARD_SHIFT_REGISTER_OE_PIN     GPIO_PIN_0
#endif


/* Bounded transactions: a wedged GOM must never hold the router indefinitely. */
#define BOARD_UART_BYTE_TIMEOUT_MS  100u
#define BOARD_UART_TX_TIMEOUT_MS   1000u
