# Thiết kế phần cứng — GOM-850 Controller

| Thuộc tính | Giá trị |
| --- | --- |
| Mục đích | Bộ điều khiển SCPI chọn 1 trong tối đa 8 máy GOM-804/GOM-805 qua RS-232 |
| MCU | STM32F411, khuyến nghị package LQFP-100 |
| Giao tiếp | 1 cổng RS-232 với PC và 8 cổng RS-232 với GOM; tại mọi thời điểm chỉ một GOM được nối vào UART_GOM |
| Trạng thái an toàn | Mất nguồn, reset, watchdog hoặc lỗi interlock: tất cả relay mở |
| Tài liệu liên quan | `GOM_850_ROUTER_DESIGN.md` — kiến trúc firmware, trạng thái và SCPI |
| Trạng thái tài liệu | Reference design để vẽ schematic/PCB; pinout, điện áp coil và kiểu DTE/DCE phải được xác nhận trước khi phát hành PCB |

## 1. Kiến trúc tổng thể

```text
                 J1: PC RS-232
                         |
                   U1 MAX3232
                         |
                       USART6
                         |
                  +----------------+
                  |  U3 STM32F411  |---- SWD / NRST / BOOT0
                  +----------------+
                    |          |
         relay select/EN     USART2
                    |          |
          U4 74HC138 + U5 ULN2803A
                    |          |
             K1 ... K8 (DPDT)  U2 MAX3232
                    |          |
        J3 ... J10: GOM RS-232 (mỗi cổng một nhánh)

    Feedback K1...K8 + fault latch ---- GPIO STM32

    USART1 PA9/PA10 ------------------- Debug CLI / UART bootloader
```

`USART2` chỉ nối với common bus của tám relay. Mỗi relay K1…K8 chuyển đồng thời hai đường dữ liệu: **GOM TXD → MAX3232 RX** và **MAX3232 TX → GOM RXD**. Không được nối song song TXD của các GOM với nhau.

PC và toàn bộ GOM không dùng hardware flow control; do đó không đưa RTS/CTS, DTR/DSR vào schematic hay connector map. Relay DPDT là đủ để chuyển hai đường dữ liệu TXD/RXD.

## 2. Phân vùng schematic

| Sheet | Nội dung | Yêu cầu chính |
| --- | --- | --- |
| `01_POWER` | Nguồn vào, buck, LDO, bảo vệ | Không cấp coil trước khi MCU khởi tạo an toàn |
| `02_MCU_DEBUG` | STM32F411, clock, reset, SWD | Tất cả chân điều khiển relay có pull-down phần cứng |
| `03_PC_RS232` | Cổng PC và MAX3232 | Bảo vệ ESD tại đầu nối; cấu hình DTE/DCE được ghi rõ |
| `04_GOM_RS232` | MAX3232 common bus, K1…K8, J3…J10 | Chỉ một nhánh được đóng; các tiếp điểm dùng NO |
| `05_RELAY_SAFETY` | decoder one-hot, driver coil, feedback, fault latch | Lỗi MCU không được có khả năng bật hai coil |

## 3. Nguồn và bảo vệ

### 3.1 Cấu trúc nguồn đề nghị

```text
J2 9…30 VDC (24 V danh định)
  -> F1 cầu chì/PTC -> chống ngược cực -> TVS -> +VIN_PROTECTED
  -> buck +5V                     -> MAX3232, logic interlock
  -> LDO +3V3                      -> STM32F411 và GPIO
  -> +VRELAY (theo điện áp coil K1…K8)
```

- Chọn điện áp đầu vào và coil sau khi chốt relay. Nếu dùng relay coil 24 V, `+VRELAY` có thể là `+VIN_PROTECTED`; nếu coil 5 V, phải tách nhánh buck coil khỏi nhánh logic.
- Bảo vệ đầu vào: F1, MOSFET chống đảo cực, TVS có điện áp standoff phù hợp nguồn danh định và tụ bulk gần relay. Giá trị cụ thể được chọn theo điện áp/nguồn thực tế.
- Mỗi IC có tụ 100 nF tại chân nguồn. STM32 thêm ít nhất một tụ 4.7–10 µF gần cụm VDD; buck cần tụ theo datasheet.
- Tính dòng nhánh coil với trường hợp xấu nhất **một coil đang hút**; thiết kế nguồn vẫn cần chịu được xung khởi động và dòng ngắn mạch của driver. Interlock one-hot là lớp bảo vệ, không thay cho rating nguồn.
- Đặt brown-out của STM32 ở mức bảo thủ. Khi brown-out/reset, `COIL_EN` phải bị kéo xuống bằng điện trở ngoài, độc lập firmware.

### 3.2 Mass và nhiễu

- Dùng ground plane liên tục. Đường hồi coil đi riêng về điểm vào nguồn/bulk capacitor, không đi dưới crystal hay qua vùng analog/reset của MCU.
- Đặt MAX3232 và TVS sát đầu nối RS-232; giữ trace phía RS-232 xa SWD/crystal.
- `SGND` của RS-232 chỉ nối chung khi PC và GOM dùng cùng tham chiếu đất. Nếu có khả năng chênh áp đất đáng kể, thay từng nhánh bằng RS-232 cách ly và DC/DC cách ly; không tự ý ngắt pin signal ground của đầu nối.

### 3.3 Mức chống nhiễu công nghiệp cần thiết kế và thử nghiệm

Các biện pháp ở trên mới là nền tảng, **chưa đủ để khẳng định đạt chuẩn công nghiệp**. Trước khi phát hành PCB, dự án phải chốt môi trường lắp đặt và mức thử tối thiểu. Với thiết bị đặt tủ công nghiệp, dùng IEC 61000-4-x hoặc chuẩn sản phẩm tương ứng làm kế hoạch thử nghiệm:

| Hiện tượng | Biện pháp phần cứng bắt buộc | Điểm cần xác minh |
| --- | --- | --- |
| ESD tại connector | TVS sát connector, đường xả TVS ngắn/rộng về chassis/return được kiểm soát | IEC 61000-4-2; mức test theo yêu cầu sản phẩm |
| EFT/burst trên dây nguồn và dây dài | TVS nguồn, lọc đầu vào, ferrite/common-mode choke khi cần, bulk capacitor; phân vùng coil khỏi logic | IEC 61000-4-4 |
| Surge trên nguồn DC ngoài tủ | Cầu chì/PTC, chống ngược cực, TVS có rating năng lượng phù hợp và buck chịu được clamp | IEC 61000-4-5 hoặc mức surge của hệ thống |
| Nhiễu dẫn/radiated RF | Ground plane liên tục, lọc nguồn từng khối, giữ loop nhỏ, vỏ/PE nếu có | IEC 61000-4-3 và 4-6 |
| Chênh áp đất cổng RS-232 | RS-232 cách ly + DC/DC cách ly cho từng port có dây đi xa/khác tủ | Đo common-mode voltage thực tế, không chỉ thử loopback |

Nếu enclosure kim loại có PE, tạo vùng `CHASSIS/PE` sát connector. TVS từ dây ngoài phải xả về vùng này với trace rất ngắn; nối `CHASSIS/PE` và digital GND bằng mạng được đánh giá EMC (thường 0 Ω/DNP, tụ an toàn hoặc RC tùy hệ thống), không mặc định nối thẳng ở mọi thiết bị. Nếu không có PE/chassis, đường hồi TVS về 0 V phải được thiết kế rộng/ngắn và xác nhận bằng thử nghiệm.

Không đặt TVS sau một trace dài rồi mong nó bảo vệ MCU: vị trí và đường hồi quyết định hiệu quả. Đường RS-232 đi ra ngoài tủ hoặc dài hơn vài mét nên được coi là ứng viên cần isolation, không chỉ TVS.

## 4. Chọn kênh RS-232 và relay

### 4.1 Topology tiếp điểm

Mỗi Kx là relay **DPDT, non-latching, break-before-make, tiếp điểm NO**. Hai pole dùng như sau:

| Pole | COM (common bus) | NO (kênh x) |
| --- | --- | --- |
| A | `GOM_BUS_TXD` từ T1OUT của U2 | `GOMx_RXD` tại Jx |
| B | `GOM_BUS_RXD` tới R1IN của U2 | `GOMx_TXD` tại Jx |

`GOM_BUS_TXD/RXD` là tín hiệu RS-232, không phải mức 3.3 V TTL. Điểm chuyển relay phải nằm ở phía RS-232 của U2. Cách này tránh việc nhiều bộ phát TTL cùng treo trên bus.

Relay phải đáp ứng điện áp/contact rating của RS-232 và có thời gian operate/release được nhà sản xuất công bố. Chọn relay có isolation coil-contact và tuổi thọ phù hợp số lần chuyển kênh dự kiến. Không dùng reed relay thiếu khả năng mang dòng coil/tiếp điểm theo yêu cầu mà không có đánh giá riêng.

### 4.2 One-hot bằng phần cứng

```text
STM32 SEL_A, SEL_B, SEL_C ---> U4 74HC138 ---> IN1...IN8 U5 ULN2803A
STM32 COIL_EN ----------------> enable chung U4 / gate nguồn coil
FAULT_LATCH -------------------> chặn enable chung (OFF-only)
U5 outputs --------------------> K1...K8 coil low-side
```

- `74HC138` tạo đúng một output active khi được enable; `COIL_EN=0` làm toàn bộ output coil OFF. Dùng logic level tương thích 3.3 V hoặc thêm level-shifter; không giả định 74HC chạy 5 V nhận mức cao 3.3 V ở mọi điều kiện. 74LVC138 chạy 3.3 V là lựa chọn đơn giản, miễn ngưỡng input của driver kế tiếp được kiểm chứng.
- U5 ULN2803A có diode clamp dùng được khi một đầu coil nối `+VRELAY` và chân COM diode nối `+VRELAY`. Với coil khác 5 V, vẫn kiểm tra giới hạn điện áp/dòng/nhiệt của IC.
- `COIL_EN` có pull-down 10 kΩ. Các chân `SEL_*` có pull-down để lúc boot/reset decoder chọn trạng thái xác định nhưng vẫn bị disable.
- `FAULT_LATCH` phải có đường phần cứng chỉ có thể **tắt** coil (ví dụ transistor kéo enable xuống). Watchdog/reset MCU không được là điều kiện duy nhất để relay nhả.
- Firmware vẫn phải thực hiện break-before-make: disable coil, chờ tối thiểu 20 ms, xác minh feedback tất cả OFF, chọn địa chỉ mới, enable, chờ tối thiểu 50 ms và xác minh đúng một feedback ON. Các giá trị là mặc định, phải kiểm tra theo relay thực tế.

### 4.3 Dập xung ngược cuộn relay

Đã có bảo vệ cơ bản: diode flyback tích hợp trong ULN2803A, với **COM nối đúng `+VRELAY`**. Đây là bắt buộc nếu dùng ULN2803A low-side; mỗi coil phải có một đường hồi năng lượng khi ngắt. Không được để COM hở và cũng không dùng chung COM với 5 V khi coil là 12/24 V.

| Phương án | Ưu điểm | Lưu ý |
| --- | --- | --- |
| Diode flyback ULN2803A | Đơn giản, giảm EMI mạnh | Relay nhả chậm hơn; phải đo release time thực tế |
| Diode + zener/TVS riêng mỗi coil | Nhả nhanh hơn, phù hợp khi cần break-before-make ngắn | Ngắt diode clamp ULN (để chân COM hở) và chọn điện áp clamp thấp hơn rating output, cao hơn `VRELAY` |
| MOSFET low-side + TVS/diode ngoài | Linh hoạt về dòng/nhiệt | Cần thiết kế riêng gate pulldown, rating avalanche và layout loop coil |

Không gắn thêm TVS song song theo cách làm diode flyback nội bộ luôn dẫn trước; khi đó TVS không có tác dụng. Với phương án nhả nhanh, dùng mạch **diode nối tiếp zener/TVS** song song coil, định hướng để nó chỉ dẫn khi điện áp đầu low-side vọt cao lúc tắt. Chọn `V_CLAMP` theo:

`VRELAY < V_CLAMP < V_CE(max) của driver – margin`, đồng thời kiểm tra năng lượng xung `E = 1/2 × Lcoil × Icoil²`, công suất lặp lại và nhiệt độ TVS/driver.

Giá trị clamp không được chốt khi chưa có datasheet relay (L/R hoặc release time) và driver. Tụ bulk đặt gần coil giúp nguồn không sụt khi relay hút nhưng **không thay thế** diode/TVS dập xung. Sau khi chọn mạch, đo bằng probe vi sai tại collector driver: điện áp tắt, thời gian nhả và nhiễu trên 3V3/UART phải nằm trong giới hạn.

### 4.4 Feedback độc lập

Không dùng chính tín hiệu ULN/decoder để kết luận relay đã đóng. Mỗi Kx cần một trong các phương án sau:

1. Tiếp điểm phụ (ưu tiên), đưa về input MCU qua lọc RC và mạch bảo vệ mức 3.3 V.
2. Mạch sense điện áp coil độc lập với output MCU, có optocoupler hoặc transistor/so sánh mức.

Tám feedback vào `RLY_FB[1..8]`; thêm `RLY_FAULT_N` từ latch. MCU chấp nhận trạng thái chỉ khi feedback one-hot và trùng kênh yêu cầu. Mismatch, relay kẹt, hoặc nhiều feedback ON phải mở tất cả coil, latch fault và yêu cầu reset/service.

## 5. RS-232

### 5.1 Transceiver và bảo vệ

- U1, U2: MAX3232 hoặc tương đương 3.0–5.5 V, dùng đúng bốn tụ charge-pump theo datasheet.
- Trang bị TVS mảng có điện dung thấp tại hai dây dữ liệu TXD/RXD của mỗi cổng. Chọn loại chịu được dải điện áp RS-232.
- Lắp điện trở series footprint 0–100 Ω phía TTL nếu đo EMI/ringing cho thấy cần; không đặt điện trở làm suy giảm quá mức biên độ RS-232.
- Ghi silk rõ `PC`, `GOM1`…`GOM8`, hướng connector và pin 1. Cùng một chuẩn connector/pinout phải được dùng trên toàn bộ các port GOM.

### 5.2 DTE/DCE — điểm cần chốt trước PCB

Không thể suy luận chéo TX/RX chỉ từ tên DB9. Cần đo/tra manual từng đầu GOM và cổng PC để xác nhận vai trò DTE hay DCE, rồi lập cable map. Schematic phải ghi tên tín hiệu logic (`PC_TXD_OUT`, `PC_RXD_IN`, `GOMx_TXD_OUT`, `GOMx_RXD_IN`) thay vì chỉ ghi số pin DB9.

Thiết bị phải giao tiếp ổn định với tất cả cấu hình baud/parity/stop bit đã chốt trong firmware. Các tham số này không được tự động đoán từ lệnh SCPI.

## 6. Pin map đề nghị (STM32F411 LQFP-100)

| Chức năng | MCU pin | Ghi chú |
| --- | --- | --- |
| `UART_PC_TX/RX` | PC6 / PC7 — USART6 | TTL 3.3 V tới U1; TX/RX DMA |
| `UART_DEBUG_TX/RX` | PA9 / PA10 — USART1 | Debug CLI và STM32 UART bootloader |
| `UART_GOM_TX/RX` | PA2 / PA3 — USART2 | TTL 3.3 V tới U2; TX/RX DMA |
| `RLY_SEL_A/B/C` | PB0 / PB1 / PB2 | Địa chỉ kênh 0…7 tới U4 |
| `RLY_COIL_EN` | PB10 | Mặc định low; đi qua logic fault chặn được |
| `RLY_FAULT_N` | PB11 | Input từ hardware latch, pull-up theo mạch chọn |
| `RLY_FB1…FB8` | PC0…PC7 | Inputs, debounce/lọc theo relay |
| `FAULT_CLEAR` | PC8 | Nút service hoặc jumper có bảo vệ; không tự clear sau lỗi nghiêm trọng |
| `STATUS_LED` | PC9 | LED trạng thái qua điện trở hạn dòng |
| `SWDIO/SWCLK` | PA13 / PA14 | Header SWD 2x5, giữ trace ngắn |
| `NRST` | NRST | Nút reset, header SWD; không treo tải lớn |
| `BOOT0` | BOOT0 | Pull-down 10 kΩ và jumper/test pad lên 3.3 V |

Pin map này cần kiểm tra lại với schematic MCU cuối cùng, đặc biệt các chân đã dùng bởi crystal, USB hoặc chức năng mở rộng. Không dùng PA13/PA14 cho tải sản phẩm.

## 7. Khối MCU, reset và debug

- Cấp toàn bộ chân VDD/VDDA theo datasheet STM32F411. `VDDA` lọc bằng ferrite/điện trở nhỏ và tụ decoupling riêng nếu ADC được dùng; nếu không dùng ADC vẫn không bỏ nguồn VDDA.
- Dùng HSE theo yêu cầu clock chính xác; footprint crystal, hai tụ tải và tuân thủ layout ST. HSI có thể đủ cho UART nếu sai số tổng được xác nhận, nhưng HSE được khuyến nghị cho thiết bị công nghiệp.
- Header SWD gồm VTref 3V3, SWDIO, SWCLK, NRST, GND. Không dùng UART vận hành để debug khi chưa có phương án cách ly rõ ràng.
- IWDG là cơ chế firmware; phần cứng bảo đảm đầu ra relay vẫn OFF trong reset và khi latch lỗi kích hoạt.

## 8. BOM khung

| Ref | Hạng mục | Số lượng | Yêu cầu |
| --- | --- | ---: | --- |
| U3 | STM32F411 LQFP-100 | 1 | Flash/RAM đủ firmware FreeRTOS + DMA |
| U1, U2 | MAX3232/equivalent | 2 | 3.3 V logic, RS-232 full duplex |
| K1…K8 | Relay DPDT non-latching | 8 | NO, break-before-make, coil theo nguồn chọn |
| U4 | Decoder one-hot | 1 | 3-bit, disable toàn bộ, tương thích 3.3 V |
| U5 | Driver coil 8 kênh | 1 | ULN2803A hoặc MOSFET array đúng điện áp/dòng coil |
| U6 | Buck 5 V | 1 | Dòng liên tục gồm MAX3232 + margin |
| U7 | LDO 3.3 V | 1 | Dòng STM32 + LED + margin; low-noise hợp lý |
| D1 | TVS nguồn | 1 | Theo dải VIN thực tế |
| D2… | TVS RS-232 | 9 cụm/array | Bảo vệ mọi cổng ngoài |
| D3…D10 | Dập xung coil (nếu không dùng diode ULN) | 8 | Diode hoặc diode + TVS/zener theo release-time mục tiêu |
| L1 / FB1… | Choke/ferrite đầu vào | tùy | Chỉ lắp sau khi tính dòng bão hòa và thử EMI |
| J1, J3…J10 | Connector RS-232 | 9 | Khóa cơ khí, silk/pinout rõ |
| J2 | Nguồn vào | 1 | Polarized/terminal block, rating phù hợp |

## 9. Quy tắc layout PCB

- Tối thiểu 4 lớp được khuyến nghị: Top signal, GND plane liên tục, Power plane, Bottom signal. Với 2 lớp, cần rà soát kỹ đường hồi và EMI trước khi sản xuất.
- Đặt relay và driver coil thành một vùng; MAX3232/TVS sát connector; MCU/crystal/SWD ở vùng digital sạch.
- Không chạy trace UART TTL hoặc feedback nhạy cảm song song dài với đường coil. Tránh khe cắt ground plane dưới trace tốc độ/clock.
- Đường `+VRELAY` và hồi coil đủ rộng theo dòng coil; tụ bulk đặt gần K1…K8/driver.
- Chừa test point: `VIN`, `5V`, `3V3`, `VRELAY`, `COIL_EN`, `SEL_A/B/C`, từng `RLY_FB`, UART TTL hai phía transceiver và RS-232 common bus.
- PCB/silk phải phân biệt rõ mass logic với signal ground RS-232 nếu có phương án isolation; không tạo cầu nối vô tình qua mounting hole/shield.

## 10. Checklist bring-up và nghiệm thu

1. Không gắn GOM: cấp nguồn, reset, brown-out và watchdog đều cho `COIL_EN=OFF`, không coil nào hút.
2. Xác nhận 3V3, 5V, `VRELAY`, ripple và nhiệt độ driver trong giới hạn tại nhiệt độ/dải VIN xấu nhất.
3. Quét đủ 8 địa chỉ: khi enable chỉ đúng một output driver/feedback ON. Đổi mọi cặp kênh phải quan sát được khoảng OFF ở giữa; không có overlap.
4. Cưỡng bức feedback sai, feedback kép và relay không nhả: `FAULT_LATCH` phải chặn coil ngay cả khi MCU vẫn yêu cầu enable.
5. Kiểm tra mức/đảo cực TX-RX bằng loopback adapter và với một GOM thực; xác nhận DTE/DCE trước khi cắm cả tám máy.
6. Chạy liên tục số chu kỳ chuyển relay đại diện cho tuổi thọ sử dụng, đo thời gian release/operate thực để chốt `RELAY_BREAK_MS` và `RELAY_SETTLE_MS`.
7. Dùng probe vi sai đo mỗi kiểu dập coil ở collector driver: xác nhận không vượt rating, thời gian nhả đáp ứng `RELAY_BREAK_MS` và không gây reset/bit error UART.
8. Thử rút cáp, tắt một GOM, ESD ở connector (theo mức thử nghiệm dự án) và reset trong khi đang truyền; relay phải về trạng thái mở an toàn.
9. Thực hiện pre-compliance ESD/EFT/surge theo môi trường đã chốt; sau mỗi phép thử xác nhận không hỏng linh kiện, không đóng sai relay, không mất/corrupt cấu hình và firmware tự hồi phục đúng.

## 11. Thông tin bắt buộc phải chốt

Trước khi phát hành schematic/BOM, cần xác nhận trên thiết bị thực:

- Điện áp nguồn đầu vào, điện áp/dòng coil relay, môi trường nhiệt độ và yêu cầu EMC/ESD.
- Model relay, dạng tiếp điểm/auxiliary feedback, operate/release time và yêu cầu tuổi thọ chuyển mạch.
- Chuẩn connector, pinout, DTE/DCE và baud/parity/stop bit cho PC lẫn GOM.
- Chính sách nối đất: common ground chấp nhận được hay cần cách ly từng RS-232 port.
- Số cổng GOM cần lắp thực tế (1…8) và yêu cầu mở rộng/USB/nguồn dự phòng.

Các mục này thay đổi chọn linh kiện và schematic chi tiết, nhưng không thay đổi các nguyên tắc bắt buộc: relay NO, chỉ một kênh vật lý được nối, one-hot có hardware enable, feedback độc lập và fault chỉ có quyền tắt relay.
