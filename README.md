# GOM-850 controller firmware core

Firmware core for an STM32F411 router that safely selects one of eight
GOM-804/GOM-805 milliohm meters.  The core is portable C11 so its command,
capability and relay safety logic can be tested on a PC before CubeMX/HAL and
FreeRTOS are connected.

## Build and test

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

`App/gom/gom_router.c` is the only PC-facing command gate.  It accepts one
SCPI program message, produces a typed `gom_operation_t`, and never forwards
the caller's string to the meter.  `gom_command_encoder.c` produces the UART
SCPI text from a whitelisted command ID only.

The table covers the documented GOM-805 command families: measurement,
compare, binning, temperature/compensation/conversion, trigger/setup,
dry-circuit, drive and PWM.  Entries whose supplied documentation is
contradictory are deliberately `HIL_PENDING`; production code rejects them
until their exact model/firmware behaviour has been verified on hardware.

## STM32 integration

The STM32F411 integration is now in `App/gom/gom_firmware.c`:

- USART1 (PA9/PA10) is `UART_PC`; USART2 (PA2/PA3) is the shared `UART_GOM`.
- K1..K8 use PB3, PB4, PB5, PB6, PB7, PB8, PB12 and PB13 respectively.
- Send `ROUT:CHAN <1..8>`, then `SYST:DEV:IDN?` to identify that GOM before
  using model-specific commands. `ROUT:OPEN:ALL` opens every relay.
- One owner task handles the complete transaction. A timeout, UART error or
  relay fault immediately opens all relays; no raw PC text is ever sent to a
  GOM.

### Safety philosophy / separation of layers

1. **Board adapter** (`App/board`) only maps STM32 pins and HAL handles. It
   contains no SCPI policy.
2. **Relay safety layer** (`App/relay`) guarantees one-hot selection and
   break-before-make; it can latch an interlock fault.
3. **Protocol policy** (`App/gom/gom_router.c`) parses and validates typed,
   whitelisted commands, capabilities and ranges. It never owns a UART.
4. **Transport owner** (`gom_firmware.c`) is the only code allowed to use the
   GOM UART or call the relay layer. It encodes typed operations, applies a
   bounded timeout and safe-opens on every transport failure.

This separation keeps CubeMX-regenerated code distinct from application
logic, prevents raw-command injection, and makes router and relay policy
host-testable. The pending commands remain blocked until the exact connected
model/firmware is HIL-verified; enabling an unverified electrical command is
not considered safe support.

## GOM serial simulator

`tools/gom_simulator.py` emulates one GOM-804 or GOM-805 over a serial port.
Use a virtual COM-pair driver on Windows, connect the STM32/router side to one
endpoint and launch the simulator on the other:

```powershell
python -m pip install -r tools/requirements.txt
python tools/gom_simulator.py --port COM11 --model GOM-805 --resistance 0.125
```

It supports the command families in the firmware whitelist, returns an IDN
string, maintains set/query configuration state, supports `READ?`, and adds
adjustable measurement noise with `--noise-ppm`.

Call `gom_router_execute()` only from `ScpiTask` after `scpi-parser` has
framed one line (or replace the small host parser with individual callbacks).
Pass the returned operation to `GomManagerTask`; that single task calls
`gom_encode_operation()`, owns UART_GOM and invokes `relay_matrix_select()`.
Hardware callbacks in `relay_matrix_io_t` must drive the real relay enable and
read independent feedback.  No PC raw-command or write/query bridge exists.
