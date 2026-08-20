#pragma once

/*
 * Board-only configuration.
 *
 * USART2 is the PC-facing RS-232 interface (PA2/PA3). USART6 is the switched
 * GOM bus (PC6/PC7). USART1 (PA9/PA10) is reserved for the debug CLI and the
 * STM32 ROM bootloader. Keep this mapping here: application code must never
 * depend on GPIO names or CubeMX-generated handles directly.
 */
#include "main.h"

/* CubeMX owns the definitions in main.c; application modules only borrow them. */
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart6;

#define BOARD_UART_PC             (&huart2)
#define BOARD_UART_GOM            (&huart6)
#define BOARD_UART_DEBUG          (&huart1)

/*
 * Two cascaded 74HC595 drive the 16 relay inputs. They are bit-banged so SPI
 * remains available for future peripherals. OE is active low and is held high
 * while an image is shifted, preventing relay glitches.
 */
#define BOARD_SHIFT_REGISTER_CONFIGURED 1u
#define BOARD_SHIFT_REGISTER_COUNT      2u
#define BOARD_SHIFT_REGISTER_BITS       16u

#if BOARD_SHIFT_REGISTER_CONFIGURED
#define BOARD_SHIFT_REGISTER_DATA_PORT  GPIOB
#define BOARD_SHIFT_REGISTER_DATA_PIN   GPIO_PIN_3
#define BOARD_SHIFT_REGISTER_CLOCK_PORT GPIOB
#define BOARD_SHIFT_REGISTER_CLOCK_PIN  GPIO_PIN_4
#define BOARD_SHIFT_REGISTER_LATCH_PORT GPIOB
#define BOARD_SHIFT_REGISTER_LATCH_PIN  GPIO_PIN_5
#define BOARD_SHIFT_REGISTER_OE_PORT    GPIOB
#define BOARD_SHIFT_REGISTER_OE_PIN     GPIO_PIN_6
#endif


/* Bounded transactions: a wedged GOM must never hold the router indefinitely. */
#define BOARD_UART_BYTE_TIMEOUT_MS  100u
#define BOARD_UART_TX_TIMEOUT_MS   1000u
