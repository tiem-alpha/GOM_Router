# GOM-850 controller firmware core

tôi muốn tạo ứng dụng như sau PC <-UART2-> stm32 <--UART6-> 8  GOM (GOM1, GOM2 ... GOM8)
PC giao tiếp với stm32f411 bằng SCPI và stm32 bới GOM bằng SCPI 
tại một thời điểm stm32 chỉ chọn 1 gom thôn qua điều khiển GPIO relay , mỗi gom 2 relay , stm32 không dùng trực tiếp GPIO mà dùng 74HC595
stm32 nhận lệnh từ pc -> parser PC request -> handle -> chọn GOM -> gửi GOM command -> nhận  response từ GOM -> parser GOM respone -> xử lý-> gửi PC response. 
khi khỏi động lênh POR ngoài inint các thức cần thiết sau đó cần đọc cấu hình từng GOM quản lý cấu hình từng GOM 
các thong tin kĩ thuật của GOM để trong GOM_850_introduction.md
các lệnh scpi GOM hỗ trợ để trong GOM_850_Command.md

yêu cầu :
  code cần sử dujgn thư viện parser có sắn không tự viết parser , cần tham khảo ở đây /home/nguyentiem/Embedded/Freelance/GOM_Router/reference
  viết state machine rõ ràng
  thời gian đáp ứng và xử lý nhanh , 
  dùng ring buffer cho RX UART đảm bảo không miss dữ liệu 
  không dùng cấp phát động 
  độc lập phần cứng 
  phân tầng rõ ràng 
  các hàm có api document đầy đủ
  dùng một task duy nhất 

  tuân thủ codign convention 


## Build and test

```powershell
cd Source
cmake --preset Debug
cmake --build --preset Debug
```

## STM32 integration

The STM32F411 integration is now in `App/gom/gom_firmware.c`:

- USART2 (PA2/PA3) is `UART_PC`; USART6 (PC6/PC7) is the shared `UART_GOM`.
  USART1 (PA9/PA10) is reserved for RTT/debug and STM32 UART bootloader.
- Two cascaded 74HC595 are connected as `PB3=DATA`, `PB4=CLOCK`,
  `PB5=LATCH`, and `PB6=/OE`. Their 16 outputs drive two relay inputs per
  GOM; `PB6` is high from reset until a zero relay image has been latched.
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
3. **application** App/aplication 
4. SCPI `App/scpi` contains the PC SCPI server and typed GOM command mapping.
5. Transport is owned by `App/gom`: UART ISR callbacks only fill fixed rings;
   the one application task drains them and drives the transaction state machine.
6. thirdparty App/thirdparty/scpi-parser chưas thư viện parser scpi cung cấp sẵn 

The PC API currently supports router commands (`*IDN?`, `*RST`, `*TST?`,
`SYST:ERR?`, `ROUT:CHAN`, `ROUT:OPEN:ALL`, `SYST:COMM:TIMEOUT`) and the
verified base GOM commands (`SYST:DEV:IDN?`, `SYST:DEV:ERR?`, `READ?`,
`SENS:FUNC`, `SENS:AUTO`, `SENS:RANG`, `SENS:SPE`, and relative settings).
Only typed, whitelisted commands are encoded for USART6; PC text is never
forwarded directly to a GOM.



## GOM serial simulator

`tools/gom_simulator.py` emulates one GOM-804 or GOM-805 over a serial port.
Use a virtual COM-pair driver on Windows, connect the STM32/router side to one
endpoint and launch the simulator on the other:

```powershell
python -m pip install -r tools/requirements.txt
python tools/gom_simulator.py --port COM11 --model GOM-805 --resistance 0.125
```
