# STM32F411RE LQFP64 pinout status

| Function | Proposed STM32 pin | Status |
| --- | --- | --- |
| `UART_DEBUG_TX/RX` | PA9 / PA10 — USART1 | Existing, retained |
| `UART_GOM_TX/RX` | PA2 / PA3 — USART2 | Existing, retained |
| `UART_PC_TX/RX` | PC6 / PC7 — USART6 | Existing, retained |
| `SR_SPI_SCK/MOSI` | To be assigned in CubeMX | Required for three 74HC595 devices |
| `SR_LATCH` | To be assigned in CubeMX | GPIO, low while shifting then rising latch pulse |
| `SR_OE_N` | To be assigned in CubeMX | GPIO with external pull keeping relay drivers disabled at reset |
| `RELAY_FB[0..15]` | To be assigned after schematic | Optional independent measurement-contact/coil feedback |
| `RS232_SELECTOR_FB[0..7]` | To be assigned after schematic | Optional independent selector feedback |
| `FAULT_LATCH_N` | To be assigned after schematic | Hardware OFF-only fault path |
| `SWDIO/SWCLK` | PA13 / PA14 | Reserved for debug |
| `BOOT0` / `NRST` | Dedicated pins | BOOT0 external pull-down and reset function retained |

The prior PB3/PB4/PB5/PB6/PB7/PB8/PB12/PB13 direct relay outputs are retired. Do not assign SPI, latch, OE, or feedback pins by editing `Source.ioc`; select STM32F411RE LQFP64 and regenerate with legacy CubeMX first.

The exact 74HC595 output map is fixed by firmware as bits 0–15 for two measurement relay drivers per channel and bits 16–23 for the one-hot RS-232 selector. The schematic must use 3.3-V-compatible logic and leave OE disabled while reset or brownout is active.
