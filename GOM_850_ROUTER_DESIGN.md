# Thiết kế firmware — STM32F411 GOM-850 Controller

## 1. Mục tiêu và nguyên tắc

STM32F411 là một **SCPI controller đa máy** cho tối đa 8 GOM-804/GOM-805 (gọi chung là `GOM`). Nó không phải serial tunnel/proxy.

- PC gửi một tập lệnh SCPI do firmware định nghĩa qua `UART_PC` + RS-232.
- `scpi-parser` phân tích từng command từ PC, kiểm tra cú pháp, kiểu dữ liệu, giới hạn và trạng thái máy đích.
- Callback SCPI tạo một `gom_operation_t` có kiểu rõ ràng (ví dụ `GOM_OP_SET_RANGE`), chọn GOM đích, rồi `gom_command_encoder` tạo command GOM tương ứng.
- Chỉ command được đăng ký trong `scpi_commands[]` mới có thể đi tới GOM. Không tồn tại `RAW`, `WRITE`, `QUERY` hoặc đường chuyển tiếp chuỗi lệnh nguyên văn từ PC.
- STM32 chỉ có một `UART_GOM`, do đó chỉ một relay/GOM được kết nối và chỉ một transaction được thực thi tại mọi thời điểm.

Lợi ích của cách này là PC không thể gửi lệnh trái phép, sai model, sai dải hoặc chèn nhiều command vào GOM. Firmware cũng có thể dùng một SCPI thống nhất cho GOM-804 và GOM-805, đồng thời trả lỗi rõ ràng khi command không được model đang chọn hỗ trợ.

## 2. Quyết định kiến trúc

| Hạng mục | Quyết định |
| --- | --- |
| Framework | STM32Cube HAL + FreeRTOS (CMSIS-RTOS v2) |
| PC protocol | SCPI server, parse thực sự bằng `scpi-parser` |
| GOM protocol | SCPI client nội bộ; encoder tự tạo từng command GOM đã whitelist |
| Luồng UART | RX DMA circular + task xử lý; TX DMA |
| Chuyển kênh | Break-before-make: tất cả relay OFF, chờ, rồi đóng một relay đích |
| Đồng thời | Một `gom_operation_t` chạy; hàng đợi PC có giới hạn |
| Khôi phục lỗi | Timeout/overrun/relay fault mở toàn bộ relay và đưa lỗi vào SCPI error queue |
| Cấp phát động | Không dùng sau khi khởi động |

`scpi-parser` là parser phía instrument/server, đúng với vai trò của STM32. Callback query phải trả kết quả khi parser đang chạy; vì vậy `ScpiTask` chờ có kiểm soát transaction từ `GomManagerTask`. UART GOM, timeout và relay vẫn chạy bằng DMA/task khác; không có busy-wait và không gọi parser trong ISR.

FreeRTOS được cấu hình **static-only**: `configSUPPORT_STATIC_ALLOCATION = 1`, `configSUPPORT_DYNAMIC_ALLOCATION = 0`. Task, queue, semaphore và buffer DMA đều có storage tại `.bss`; firmware không gọi `pvPortMalloc`, `malloc`, `calloc`, `realloc`, `free`, `new`, VLA hay `alloca`.

## 3. Kiến trúc phần cứng

```text
 PC -- RS-232 -- MAX3232 #1 -- UART_PC      STM32F411
                                           +---------------------+
 GOM 1 --[2P relay]--\                    | SCPI command parser |
 GOM 2 --[2P relay]---+-- MAX3232 #2 -- UART_GOM               |
 ...                  |                    | GOM operation mgr   |
 GOM 8 --[2P relay]--/                    | relay matrix         |
                                           +---------------------+
 GPIO_RLY_1...GPIO_RLY_8 --> transistor/ULN2803A --> relay coils
```

### 3.1 Relay và RS-232

Mỗi kênh cần relay **DPDT (2P)** hoặc hai relay SPDT liên động để chuyển cả TX và RX. Tám relay SPDT đơn không đủ để chọn an toàn một cổng serial full-duplex.

- Dùng tiếp điểm `NO`; sau reset, mất nguồn hoặc watchdog, mọi relay đều OFF.
- Dùng MOSFET low-side hoặc ULN2803A cùng diode dập xung ngược cho coil; GPIO có pull-down để relay không bật khi boot.
- Relay phải là break-before-make. Firmware vẫn buộc một khoảng tất cả relay OFF, không phụ thuộc riêng vào cơ khí.
- Đường điều khiển coil cần interlock phần cứng one-hot (ví dụ decoder/enable chung) để một lỗi phần mềm không thể kích đồng thời hai coil.
- Bản production cần feedback độc lập cho trạng thái relay: auxiliary contact, optocoupler đọc điện áp bus hoặc mạch sense tương đương. GPIO output one-hot chỉ chứng minh lệnh điều khiển, không phát hiện relay kẹt hay tiếp điểm bị hàn.
- Trước khi đóng kênh mới, firmware phải xác nhận tất cả feedback đều OFF. Sau khi đóng, phải xác nhận đúng một feedback ON và trùng kênh yêu cầu; nếu sai thì latch `RELAY_INTERLOCK_FAULT`, ngắt enable coil chung và cấm mọi lần đóng tiếp theo cho tới reset/service.
- Nếu dùng RTS/CTS hoặc DTR/DSR, relay phải chuyển cả các dây đó, hoặc tắt hardware flow control ở PC và GOM. Không nối chung output RS-232 của nhiều GOM.
- GND RS-232 chỉ được dùng chung khi hệ thống cùng tham chiếu đất. Nếu phải chuyển signal ground thì DPDT không đủ: cần ít nhất 3 cực hoặc transceiver cách ly cho từng nhánh.

### 3.2 Ngoại vi STM32

| Vai trò | Ngoại vi | Cấu hình đề nghị |
| --- | --- | --- |
| PC | `UART_PC` (ví dụ USART1) | 8N1, baud cấu hình được, RX circular DMA, TX DMA |
| GOM | `UART_GOM` (ví dụ USART2) | đúng format GOM, RX circular DMA, TX DMA |
| Relay control | 8 GPIO output + hardware enable/interlock | polarity cấu hình trong `board_config.h` |
| Relay feedback | 8 GPIO input hoặc bus-sense tương đương | xác minh trạng thái tiếp điểm độc lập với output coil |
| Time base | TIM hoặc FreeRTOS tick | relay settling, transaction timeout |
| Debug | SWD, UART độc lập nếu cần | không dùng chung UART vận hành |

Ba tham số serial của GOM (baud, parity, stop bits) phải được xác nhận trên máy thật và nhất quán ở tất cả GOM. `SYSTem:SERial?` được nêu trong tài liệu nhưng không đủ để firmware suy đoán giá trị mặc định.

## 4. Chuyển kênh relay an toàn

`relay_matrix_select(channel)` là API runtime duy nhất có quyền đổi relay. Đường emergency-off chỉ được phép tắt coil trực tiếp; không bao giờ được bật relay.

```text
IDLE
  -> request channel N
     -> nếu N đang chọn: giữ nguyên
     -> nếu khác N:
          1. hủy transaction GOM đang chạy (nếu có)
          2. abort RX/TX DMA của UART_GOM và xóa RX buffer
          3. đặt cả 8 GPIO OFF
          4. đợi RELAY_BREAK_MS
          5. xác nhận feedback tất cả OFF; nếu sai -> INTERLOCK_FAULT
          6. đóng duy nhất relay N
          7. đợi RELAY_SETTLE_MS
          8. xác nhận feedback one-hot và đúng N; nếu sai -> INTERLOCK_FAULT
          9. xóa RX buffer lần nữa
         10. selected_channel = N; chuyển IDLE
```

Giá trị khởi tạo an toàn:

```c
#define RELAY_BREAK_MS       20u
#define RELAY_SETTLE_MS      50u
#define GOM_QUERY_TIMEOUT_MS 5000u
#define GOM_WRITE_TIMEOUT_MS  500u
```

`READ?` có thể lâu hơn do `SENS:SPEed`, averaging, measure delay hoặc trigger delay. Vì vậy timeout query là thuộc tính router cấu hình được (`SYST:COMM:TIMEout`), dải 100…30000 ms; không hard-code theo response nhanh của `*IDN?`.

## 5. Kiến trúc firmware

```text
UART_PC DMA ISR --> PC RX ring --> ScpiTask ----------+
                                                        | typed request + wait
                                                        v
                                                  GomManagerTask
                                                        |
                              RelayMatrix <------------+----> UART_GOM DMA
                                                        |
                                                        v
                                                   selected GOM

WatchdogTask <---- health counters / fault state ---- all modules
```

| Task/module | Trách nhiệm | Không được làm |
| --- | --- | --- |
| `pc_uart` | RX circular DMA, transfer block RX cho `ScpiTask`, TX DMA response | parse SCPI trong ISR |
| `scpi_server` / `ScpiTask` | gọi `SCPI_Input`, lấy tham số typed, validate, tạo operation, trả SCPI result/error | điều khiển relay trực tiếp |
| `command_mapper` | ánh xạ command PC sang operation và GOM command plan đã whitelist | đọc/ghi UART |
| `gom_manager` / `GomManagerTask` | sở hữu UART_GOM, relay state, transaction, timeout, parse response GOM | gọi parser PC |
| `relay_matrix` | one-hot, break-before-make, trạng thái kênh | gửi UART |
| `fault_manager` | latch lỗi quan trọng, mở relay, phục hồi watchdog | tự đóng relay |

### 5.1 Kiểu operation nội bộ

PC không truyền string GOM xuống `gom_manager`. Callback tạo dữ liệu typed; encoder duy nhất tạo chuỗi gửi đi.

```c
typedef enum {
    GOM_OP_IDENTIFY,
    GOM_OP_SET_FUNCTION,
    GOM_OP_SET_AUTO_RANGE,
    GOM_OP_SET_RANGE,
    GOM_OP_SET_SPEED,
    GOM_OP_SET_RELATIVE,
    GOM_OP_READ,
    GOM_OP_SET_COMPARE_LIMIT,
    GOM_OP_GET_COMPARE_RESULT,
    GOM_OP_SET_TRIGGER_SOURCE,
    GOM_OP_SET_AVERAGING,
    GOM_OP_SET_TEMP_COMP,
    GOM_OP_SET_DRY_CIRCUIT,
    GOM_OP_SET_DRIVE,
    GOM_OP_GET_DEVICE_ERROR
} gom_operation_kind_t;

typedef struct {
    uint32_t request_id;              /* tăng đơn điệu, không tái dùng trong phiên */
    uint8_t channel;                 /* 1..8, đã validate */
    gom_operation_kind_t kind;
    bool expects_response;
    union {
        double value;
        bool enabled;
        uint16_t count;
        gom_function_t function;
        gom_speed_t speed;
        gom_drive_t drive;
        gom_compare_limit_t limit;
    } arg;
} gom_operation_t;

typedef struct {
    uint32_t request_id;
    gom_completion_status_t status;
    gom_typed_result_t result;
} gom_completion_t;
```

Ví dụ `SENS:RANG 0.005` từ PC được parse thành `double`, kiểm tra `5e-3 <= value <= 5e6`, sau đó tạo `GOM_OP_SET_RANGE`. Chỉ encoder mới được phép tạo `"SENS:RANG %.7G\r\n"` cho UART_GOM.

### 5.2 State machine transaction

```text
READY
  -> SWITCHING (nếu target khác kênh đang chọn)
  -> ENCODE_COMMAND
  -> TX_START -> TX_WAIT
  -> [set command] VERIFY_WRITE -> DONE
  -> [query command] RX_WAIT -> PARSE_RESPONSE -> DONE
  -> TIMEOUT / UART_ERROR / RX_OVERFLOW / INVALID_RESPONSE
     -> SAFE_OPEN -> DESYNCHRONIZED
DESYNCHRONIZED
  -> QUIET_DRAIN -> PURGE_RX -> IDN_HANDSHAKE
  -> [handshake OK] READY
  -> [handshake fail] OFFLINE
```

Quy tắc:

- Mỗi operation có một command GOM duy nhất hoặc một command plan cố định. Không nhận command GOM tự do từ PC.
- Query nhận một response line kết thúc `LF`; bỏ `CR/LF`, sau đó parse theo kiểu response mong đợi (`double`, integer, boolean, enum, IDN, SCPI error).
- Response sai định dạng, quá dài, đến muộn hoặc lỗi UART đều không được đưa nguyên văn về PC; chúng trở thành lỗi router có thông tin diagnostic.
- Khi chuyển channel, mọi byte RX còn lại bị hủy để không thể lẫn response giữa hai GOM.
- Mỗi request/completion mang `request_id`; `ScpiTask` chỉ nhận completion có ID đúng. Completion muộn bị discard và tăng diagnostic counter, không thể hoàn tất request mới.
- Timeout không đưa channel thẳng về `READY`. Channel bị đánh dấu `DESYNCHRONIZED`; chỉ được dùng lại sau quiet-time dài hơn thời gian response tệ nhất đã HIL-verify, purge RX và `*IDN?` handshake thành công.
- Set command production dùng **verified-write**: sau mỗi command plan, đọc `SYST:ERR?` hoặc query-back thuộc tính vừa đặt. Nếu một plan nhiều bước thất bại, cache bị invalidate và trả trạng thái `PARTIAL_APPLY`; firmware không giả vờ rollback trạng thái GOM.

## 6. SCPI công khai cho PC (v1)

### 6.1 Chọn máy đích và lệnh router

PC luôn chọn GOM đích trước; mọi command thiết bị sau đó tác động lên channel đã chọn. Trong v1, mỗi program message chỉ chứa một command. Compound message bị từ chối để tránh nhầm path inheritance và để giới hạn response/buffer. Nếu bật ở phiên bản sau, mỗi command đổi subsystem phải dùng absolute header, ví dụ `ROUT:CHAN 3;:CONF:RES AUTO;:READ?`; dạng `ROUT:CHAN 3;CONF:RES AUTO` là sai vì `scpi-parser` sẽ kế thừa path thành `ROUT:CONF:RES`.

| Command | Chức năng |
| --- | --- |
| `*IDN?` | Nhận dạng router: `STM32,GOM850-CONTROLLER,<serial>,1.0` |
| `*RST` | Hủy operation, mở relay, xóa channel chọn và reset trạng thái router |
| `*TST?` | Trả `0` khi không có router fault latched |
| `SYST:ERR?` | Pop SCPI error của router |
| `ROUTe:CHANnel <1..8>` | Chuyển an toàn sang GOM đích |
| `ROUTe:CHANnel?` | Trả `0` (chưa chọn) hoặc `1..8` |
| `ROUTe:OPEN:ALL` | Mở mọi relay, bỏ chọn GOM |
| `ROUTe:STATus?` | Trả channel, state, relay mask, lỗi gần nhất |
| `SYST:COMM:TIMEout <ms>` / `?` | Đặt/đọc timeout query của router |
| `SYST:DEVice:IDN?` | Đọc IDN của GOM đang chọn; parser response và cache model/serial |
| `SYST:DEVice:ERRor?` | Đọc, parse `SYST:ERR?` của GOM đang chọn |

Lệnh đòi target nhưng chưa `ROUT:CHANnel` phải trả user error `100,"No GOM channel selected"`; không được phát bất kỳ byte nào ra UART_GOM.

### 6.2 Tập command đo điện trở — v1 bắt buộc

Đây là tập command cần triển khai đầu tiên vì toàn bộ GOM trong tài liệu đều hỗ trợ.

| Command PC được parse | Parameter/validation | Command plan GOM do firmware tự tạo |
| --- | --- | --- |
| `CONFigure:RESistance AUTO` | không có range cố định | `SENS:FUNC OHM` rồi `SENS:AUTO ON` |
| `CONFigure:RESistance <range>` | `5e-3…5e6` ohm | `SENS:FUNC OHM`; `SENS:AUTO OFF`; `SENS:RANG <range>` |
| `SENSe:FUNCtion {OHM|COMP|BIN|TC|TCONV|DIODE}` | enum; check capability theo model | `SENS:FUNC <mapped token>` |
| `SENSe:AUTo {ON|OFF}` | chỉ nhận mnemonic `ON/OFF`; nếu hỗ trợ số thì parse integer chính xác `0/1` | `SENS:AUTO ON|OFF` |
| `SENSe:RANGe <range>` | `5e-3…5e6` ohm | `SENS:RANG <range>` |
| `SENSe:SPEed {FAST|SLOW}` | enum | `SENS:SPE FAST|SLOW` |
| `SENSe:RELative:STATe {ON|OFF}` | boolean | `SENS:REL:STAT ON|OFF` |
| `SENSe:RELative:DATa <ohm>` | SI ohm; number trong dải GOM | encoder đổi sang value + suffix/range đã HIL-verify |
| `READ?` | không parameter | `READ?`; parse result số thực |
| `SENSe:RANGe?` | không parameter | `SENS:RANG?`; parse result số thực |

`READ?` xử lý hai sentinel theo tài liệu GOM: `+9.0000E+9` (over range) và `+9.9999E+9` (HVP). Chúng không được trả như một giá trị đo bình thường: callback push lỗi device-dependent, cập nhật status và trả SCPI error phù hợp.

### 6.3 Compare, trigger, temperature và system setup

Các command sau cũng được đăng ký callback riêng; PC chỉ gửi giá trị typed, còn GOM command là mapping cố định.

| Nhóm PC | Ví dụ command PC | Mapping GOM |
| --- | --- | --- |
| Compare | `CALC:COMP:TYPE {OHM|TC}`, `CALC:COMP:LIM:LOW <ohm>`, `CALC:COMP:LIM:UPP <ohm>`, `CALC:COMP:LIM:RES?` | public value chuẩn SI; encoder phát suffix rõ ràng; result `LO|PASS|HI` từ GOM `0|1|2` |
| Binning (805) | `BINN:COUN:CLE`, `BINN3:LIM:LOW <ohm>`, `BINN:LIM:RES?` | chỉ enable sau HIL; suffix/đơn vị explicit và capability check GOM-805 |
| Trigger | `TRIG:SOUR {INT|EXT}`, `TRIG:DEL:STAT {ON|OFF}`, `TRIG:DEL:DATA <ms>`, `*TRG` | `TRIG:SOUR`, `TRIG:DEL:STAT`, `TRIG:DEL:DATA`, `*TRG` |
| Temperature | `TEMP:STAT {ON|OFF}`, `TEMP:UNIT {DEGC|DEGF}`, `TEMP:AMB:DATA <n>`, `TEMP:DATA?` | Command tương ứng, parse nhiệt độ query thành number |
| Averaging | `SYST:AVER:STAT {ON|OFF}`, `SYST:AVER:DATA <2..100>` | `SYST:AVER:STAT`, `SYST:AVER:DATA` |
| Measurement delay | `SYST:MDEL:STAT {ON|OFF}`, `SYST:MDEL:DATA <n>` | `SYST:MDEL:STAT`, `SYST:MDEL:DATA` |
| GOM-805 source | `SOUR:DRY {ON|OFF}`, `SOUR:DRIV {1..6}` | `SOUR:DRY`, `SOUR:DRIV`; reject với GOM-804 |

Tài liệu hiện có có các mâu thuẫn về BIN (1–8 hay 1–9), `CALC:COMP:TYPE`, `SOUR:DRIV`, semantic `TRIG:DEL:STAT` và đơn vị/range của `SYST:MDEL:DATA`. Các command đó phải được gắn `UNVERIFIED` và không có mặt trong command table production cho tới khi HIL xác nhận trên đúng model/firmware.

### 6.4 Ví dụ vận hành

```text
PC -> ROUT:CHAN 3
PC -> SYST:DEV:IDN?
PC <- GWINSTEK,GOM805,GXXXXXXXX,V1.00

PC -> CONF:RES AUTO
PC -> SENS:SPE FAST
PC -> READ?
PC <- +2.2012E+0

PC -> CALC:COMP:LIM:LOW 95
PC -> CALC:COMP:LIM:UPP 105
PC -> CALC:COMP:LIM:RES?
PC <- PASS
```

Không có command dạng `GOM:RAW`, `GOM:WRITE`, `GOM:QUERY?` hay chuỗi quoted chứa command GOM.

Mọi đại lượng điện trở trong public API dùng đơn vị SI ohm. PC không phụ thuộc range đang hiển thị trên GOM. Encoder chịu trách nhiệm đổi sang cú pháp có suffix đã xác nhận; query parser đổi response về SI trước khi gọi `SCPI_ResultDouble()`.

## 7. Mapping và capability model

### 7.1 Registry command

Mỗi PC command có một entry tĩnh, gồm parser callback, operation type, validation và capability cần có.

```c
typedef struct {
    const char *pc_pattern;
    gom_operation_kind_t operation;
    uint32_t required_capabilities;
    gom_param_rule_t parameter_rule;
    gom_response_type_t response_type;
    gom_command_verification_t verification; /* UNVERIFIED/HIL_VERIFIED */
} command_map_t;
```

`scpi_commands[]` production chỉ được sinh từ entry `HIL_VERIFIED` và tham chiếu callback cụ thể. Không dùng wildcard fallback cho command chưa biết; `scpi-parser` sẽ đưa `-113,"Undefined header"` vào error queue. Ma trận command machine-readable phải ghi rõ model, firmware GOM đã test, range, unit, response type và bằng chứng HIL.

### 7.2 Nhận dạng và capability GOM

Khi `SYST:DEV:IDN?` thành công, firmware parse chuỗi thành manufacturer/model/serial/firmware và lưu vào `gom_device_t[8]`.

```c
typedef struct {
    bool identified;
    bool online;
    gom_model_t model;       /* UNKNOWN, GOM804, GOM805 */
    uint32_t capabilities;   /* OHM, COMPARE, TEMP, BINNING, DRY, DRIVE ... */
    char serial[16];
    char firmware[16];
    uint32_t last_seen_ms;
    gom_fault_t last_fault;
} gom_device_t;
```

Command cần `CAP_DRY`, `CAP_DRIVE` hoặc `CAP_BINNING` bị reject nếu target chưa identify hoặc là GOM-804. Model phải lấy từ nhãn máy và response `*IDN?`; tên dự án “GOM-850” không được dùng để suy ra capability khi tài liệu hiện chỉ mô tả GOM-804/805. Điều này đáp ứng yêu cầu “quyết định gửi đến GOM nào và lệnh tương ứng là gì” mà không phơi lộ command GOM cho PC.

## 8. Tích hợp `scpi-parser`

1. Add include path `thirdparty/scpi-parser/libscpi/inc`.
2. Biên dịch `.c` trong `thirdparty/scpi-parser/libscpi/src` cùng firmware.
3. `App/scpi/router_scpi_def.c` chứa command table và callback typed.
4. `App/scpi/router_scpi_io.c` hiện thực `SCPI_Write` qua PC TX buffer/DMA, `SCPI_Flush` và `SCPI_Error`.
5. `App/scpi/scpi_user_config.h` khóa cấu hình error/device-info cho target ARM.
6. Khởi tạo `SCPI_Init()` và `SCPI_InitHeap()` một lần trong `ScpiTask` trước khi nhận byte PC.

`scpi-parser` dùng tên macro hơi ngược trực giác: khi `USE_DEVICE_DEPENDENT_ERROR_INFORMATION=1`, giá trị `USE_MEMORY_ALLOCATION_FREE=1` đi qua `strndup/malloc/free`; giá trị `0` dùng `scpiheap` do application cấp. Cấu hình static-only bắt buộc:

```c
#define USE_FULL_ERROR_LIST                    1
#define USE_USER_ERROR_LIST                    1
#define USE_DEVICE_DEPENDENT_ERROR_INFORMATION 1
#define USE_MEMORY_ALLOCATION_FREE             0 /* dùng application-owned scpiheap */

#define LIST_OF_USER_ERRORS \
    X(SCPI_ROUTER_ERROR_NO_CHANNEL,       100, "No GOM channel selected") \
    X(SCPI_ROUTER_ERROR_DESYNCHRONIZED,   101, "GOM channel desynchronized") \
    X(SCPI_ROUTER_ERROR_UNSUPPORTED,      102, "Command unsupported by target") \
    X(SCPI_ROUTER_ERROR_INVALID_REPLY,    103, "Invalid GOM response") \
    X(SCPI_ROUTER_ERROR_RELAY_INTERLOCK,  104, "Relay interlock fault") \
    X(SCPI_ROUTER_ERROR_PARTIAL_APPLY,    105, "Configuration partially applied") \
    X(SCPI_ROUTER_ERROR_OVER_RANGE,       106, "Measurement over range") \
    X(SCPI_ROUTER_ERROR_HVP,              107, "High voltage protection detected") \
    X(SCPI_ROUTER_ERROR_MULTI_COMMAND,    108, "One command per message required")
```

```c
static char scpi_error_info_heap[1024];

SCPI_Init(&scpi_context, ...);
SCPI_InitHeap(&scpi_context,
              scpi_error_info_heap,
              sizeof(scpi_error_info_heap));
```

Link-map/CI phải fail nếu application image còn tham chiếu `malloc`, `calloc`, `realloc`, `free`, `pvPortMalloc` hoặc `vPortFree`. Không dùng `heap_1…heap_5.c` khi `configSUPPORT_DYNAMIC_ALLOCATION=0`.

Mọi kích thước protocol chỉ định nghĩa một lần tại `protocol_limits.h`:

```c
#define PC_PROGRAM_PAYLOAD_MAX       383u
#define PC_TERMINATOR_MAX              2u /* CRLF */
#define PC_FRAME_SLOT_SIZE            386u /* payload + CRLF + NUL */
#define SCPI_INPUT_BUFFER_LENGTH      512u
#define SCPI_ERROR_QUEUE_SIZE          16u
#define SCPI_ERROR_INFO_HEAP_SIZE    1024u
#define PC_RX_DMA_SIZE                512u
#define PC_TX_FRAME_SIZE              384u
#define GOM_TX_BUFFER_SIZE            128u
#define GOM_RX_DMA_SIZE               256u
#define GOM_RX_LINE_MAX               256u
#define PC_COMMAND_QUEUE_DEPTH          4u

_Static_assert(PC_FRAME_SLOT_SIZE >=
               PC_PROGRAM_PAYLOAD_MAX + PC_TERMINATOR_MAX + 1u,
               "PC frame needs payload, CRLF and NUL");
_Static_assert(SCPI_INPUT_BUFFER_LENGTH >= PC_FRAME_SLOT_SIZE,
               "SCPI parser input too small");
```

`SCPI_Input()` nhận fragment stream và tự tìm termination `LF`; gọi nó trong `ScpiTask` với block nhận từ PC, không tự parse/gọi callback từ DMA ISR. Callback ưu tiên `SCPI_ParamDouble`, `SCPI_ParamChoice` và `SCPI_CommandNumbers`. Với boolean GOM, chỉ dùng choice `ON|OFF`, hoặc `SCPI_ParamInt32` rồi kiểm tra chính xác `0/1`; không dùng trực tiếp `SCPI_ParamBool` cho numeric input.

## 9. Cấu trúc project đề nghị

```text
Core/
  Inc/                         # CubeMX-generated headers
  Src/                         # HAL init, startup, main
App/
  board/
    board_config.h             # UART, relay polarity, timing mặc định
  comm/
    uart_dma_ring.c/.h
    pc_link.c/.h
    gom_link.c/.h
  relay/
    relay_matrix.c/.h          # one-hot + break-before-make + feedback verify
    relay_emergency.c/.h       # OFF-only path, latch fault epoch
  gom/
    gom_manager.c/.h           # operation queue, state machine, timeout
    gom_command_encoder.c/.h   # typed operation -> SCPI của GOM
    gom_response_parser.c/.h   # response GOM -> typed data
    gom_capabilities.c/.h
    gom_command_matrix.c/.h    # model/fw/unit + UNVERIFIED/HIL_VERIFIED
  scpi/
    scpi_user_config.h         # static error-info heap/user error list
    router_scpi_def.c/.h       # scpi_commands[] và callbacks
    router_scpi_io.c/.h
    router_scpi_errors.h
  fault/
    fault_manager.c/.h
  app_main.c/.h
Middleware/
  FreeRTOS/
thirdparty/
  scpi-parser/                 # đã có; giữ nguyên upstream
tests/
  host/
    test_command_mapper.c
    test_gom_command_encoder.c
    test_gom_response_parser.c
    test_relay_matrix.c
    test_router_scpi.c
```

Code CubeMX và application không trộn lẫn: `main.c` chỉ gọi `App_Init()` sau `MX_FREERTOS_Init()`. Regenerate `.ioc` không được phá application code.

## 10. Error model

Không tái sử dụng `-311…-315` vì `scpi-parser` đã định nghĩa chúng cho lỗi memory/save-recall/configuration. Dùng mã chuẩn khi phù hợp và `LIST_OF_USER_ERRORS` dương cho lỗi riêng router:

| Mã | Nguồn | Nghĩa |
| ---: | --- | --- |
| `-350` | SCPI chuẩn | PC command/event queue overflow |
| `-360` | SCPI chuẩn | UART/DMA communication error chung |
| `-362` | SCPI chuẩn | UART framing error |
| `-363` | SCPI chuẩn | Input/response buffer overrun |
| `-365` | SCPI chuẩn | GOM transaction timeout |
| `100` | Router user error | Chưa chọn GOM |
| `101` | Router user error | Channel đang `DESYNCHRONIZED/OFFLINE` |
| `102` | Router user error | Command không hỗ trợ bởi target/model/verification state |
| `103` | Router user error | Response GOM sai kiểu hoặc sai cú pháp |
| `104` | Router user error | Relay feedback/interlock fault |
| `105` | Router user error | Command plan chỉ áp dụng được một phần |
| `106` | Router user error | Giá trị đo over-range |
| `107` | Router user error | GOM báo HVP |
| `108` | Router user error | V1 chỉ cho phép một command trong mỗi message |

Ví dụ: `102,"SOUR:DRIV requires HIL-verified GOM-805;channel=2;model=GOM804"`. Device-dependent text chỉ được bật qua application-owned `scpiheap`; không được kéo `malloc/free` vào image. Mọi lỗi transport sau khi đã phát command GOM đều safe-open và đưa channel sang `DESYNCHRONIZED`, không tự động cho command kế tiếp chạy.

## 11. Kiểm soát tài nguyên, stack và buffer

### 11.1 Task tĩnh và ngân sách stack

Chỉ tạo task bằng `xTaskCreateStatic()` trong `App_Init()`. Các task dùng stack global, không cấp phát trên heap.

| Task | Priority | Stack khởi tạo | Nhiệm vụ thời gian thực |
| --- | ---: | ---: | --- |
| `GomManagerTask` | 5 | 512 words | drain UART_GOM, timeout, điều khiển relay, parse response |
| `HealthTask` | 4 | 256 words | watchdog/deadline, stack watermark; chạy ngắn rồi block |
| `PcIngressTask` | 3 | 448 words | drain DMA theo byte budget, tạo frame, back-pressure rồi block |
| `ScpiTask` | 3 | 768 words | `SCPI_Input`, validate/map, chờ operation, tạo response |

Kích thước trên chỉ là ngân sách khởi tạo, không phải con số cuối cùng. Phải đo `uxTaskGetStackHighWaterMark()` trong HIL/soak test, cộng biên an toàn tối thiểu 25%, và kiểm tra file `.su` từ GCC (`-fstack-usage`, `-Wstack-usage=<limit>`) trong build CI. Cấm đệ quy và cấm local array lớn trong task/callback/ISR.

Trong `FreeRTOSConfig.h` bật `configCHECK_FOR_STACK_OVERFLOW = 2` và hiện thực `vApplicationStackOverflowHook()` tối thiểu: latch fault, ghi thanh ghi GPIO tắt toàn bộ relay bằng BSRR và để watchdog reset. Hook không được log, gửi queue, gọi HAL có thể block hay cấp phát bộ nhớ vì stack có thể đã hỏng.

`PcIngressTask` xử lý tối đa `PC_INGRESS_BYTE_BUDGET` mỗi lần wake, sau đó `taskYIELD()` hoặc block lại trên notification. Nó không được spin trong discard mode. Vì vậy PC flood không thể starve `ScpiTask`/`HealthTask`; `HealthTask` priority cao hơn nhưng mỗi chu kỳ có execution budget rất ngắn.

### 11.2 Định mức buffer và quyền sở hữu

Mọi giới hạn protocol lấy từ `protocol_limits.h`; không lặp lại con số tại `board_config.h` hay module khác.

| Hướng dữ liệu | Storage tĩnh | Kích thước đề nghị | Chủ sở hữu | Khi đầy |
| --- | --- | ---: | --- | --- |
| PC -> MCU, byte stream | `pc_rx_dma[512]` circular DMA | 512 B | `PcIngressTask` | drain ngay; deassert RTS nếu có |
| PC -> SCPI, frame | `pc_frame_pool[4][386]` | 4 x 386 B | free-list + `ScpiTask` | discard đến LF, record overflow event |
| SCPI parser input | `scpi_input[512]` | 512 B | `ScpiTask` | reject whole line `-363` |
| SCPI -> PC response | `pc_tx_pool[2][384]` | 2 x 384 B | `ScpiTask`/TX DMA | wait bounded for TX slot; emergency error frame |
| MCU -> GOM | `gom_tx[128]` | 128 B | `GomManagerTask` | encoder fails before TX |
| GOM -> MCU, byte stream | `gom_rx_dma[256]` circular DMA | 256 B | `GomManagerTask` | abort, drain to LF, `-363`, desync |
| GOM response line | `gom_reply[256]` | 256 B | `GomManagerTask` | reject, `-363`, safe-open/desync |
| SCPI -> GOM request | static queue, depth 1 | `gom_operation_t` copy | `ScpiTask`/`GomManagerTask` | bounded wait/error |
| GOM -> SCPI completion | static queue, depth 1 | `gom_completion_t` copy | `GomManagerTask`/`ScpiTask` | ID mismatch discarded/fault counter |

`PC_PROGRAM_PAYLOAD_MAX` là 383 byte, cộng tối đa CRLF và NUL trong slot 386 byte. Vượt giới hạn chuyển ingress sang **discard-until-LF mode**, nhờ vậy byte còn lại của dòng lỗi không thể đè frame kế tiếp. `GOM_RX_LINE_MAX` xử lý tương tự: nếu reply vượt giới hạn, vẫn drain đến `LF`, nhưng không giữ thêm byte và không trả partial response.

Không để `ScpiTask` giữ DMA ring trong lúc chờ query GOM. `PcIngressTask` luôn có thể drain UART và đưa tối đa bốn frame vào pool; khi pool đầy, nó hạ RTS (nếu phần cứng hỗ trợ) và bỏ có kiểm soát những dòng mới. Nếu PC không tôn trọng RTS, kết quả vẫn là mất command đã được báo lỗi, không phải tràn RAM hay hỏng parser.

V1 chỉ cho phép **một command trong mỗi program message**. `PcIngressTask` chạy preflight state machine nhỏ, không cấp phát động, để phát hiện dấu `;` ngoài quote; compound message bị từ chối với user error `108` trước khi tạo operation. Điều này loại bỏ path inheritance mơ hồ và giới hạn output tối đa dưới `pc_tx_pool`.

DMA circular không chỉ dựa vào vị trí `NDTR`. Mỗi UART duy trì `uint32_t producer_count` đơn điệu, cập nhật từ HT/TC epoch, và `consumer_count` của task. Snapshot phải atomic; nếu `producer_count - consumer_count > DMA_SIZE` thì đã xảy ra lap/overwrite: ghi fault counter, reset parser/line state và không dùng dữ liệu partial. Áp dụng giống nhau cho PC RX và GOM RX.

### 11.3 Ownership và giao tiếp task

- ISR DMA chỉ chụp vị trí write, clear flag và `vTaskNotifyGiveFromISR()`; không copy frame, không log, không gọi SCPI/HAL blocking.
- `PcIngressTask` là consumer duy nhất của `pc_rx_dma`; nó sở hữu pool/free-list frame PC.
- `ScpiTask` là owner duy nhất của `scpi_t`, input/error queue của `scpi-parser` và response builder.
- `GomManagerTask` là owner duy nhất của `UART_GOM`, `gom_rx_dma`, `gom_tx`, `gom_reply` và relay API.
- Request và completion queue đều truyền **bản sao** struct có `request_id`. Task notification chỉ báo “queue có dữ liệu”, không đại diện trực tiếp cho completion. Không gửi địa chỉ local stack qua queue/notification.
- TX PC chỉ nhận frame immutable từ pool; DMA complete mới trả slot về free-list. Không có buffer bị ghi lại khi DMA đang chạy.

`PcIngressTask` không được gọi `SCPI_ErrorPush()` vì không sở hữu `scpi_t`. Khi phải bỏ line (pool đầy, line quá dài, compound message), nó đẩy một `pc_ingress_event_t` tĩnh hoặc tăng atomic diagnostic counter. Trước khi `ScpiTask` nhận frame hợp lệ kế tiếp, nó đổi event đó thành `-350`, `-363` hoặc user error `108` trong SCPI error queue. Vì vậy mọi ghi vào `scpi-parser` context vẫn là single-task, không có race.

Emergency-off từ ISR/hook được phép ghi BSRR để tắt coil và tăng `relay_fault_epoch`; nó không cập nhật state cache phức tạp. Trước mỗi TX hoặc relay close, `GomManagerTask` so sánh epoch, invalidate `selected_channel`/cache nếu epoch đổi và chỉ tiếp tục sau explicit recovery.

### 11.4 Tốc độ và độ ổn định

- Chỉ ISR DMA và task priority cao xử lý byte stream; parsing SCPI và format response không chạy trong ISR.
- `GomManagerTask` có priority cao hơn `ScpiTask`, nên RX GOM không mất byte khi callback SCPI đang chờ response.
- Relay chỉ chuyển khi target thay đổi. Kênh hiện tại, model và capability được cache tĩnh.
- Encoder tự viết số vào `gom_tx` bằng formatter có giới hạn length và kiểm tra giá trị trả về; cấm `sprintf`, `strcpy`, `strcat`, `gets`, `scanf` không giới hạn và `%f` không kiểm soát.
- Mọi counter buffer/length dùng `size_t` hoặc `uint16_t` sau khi đã kiểm tra range; mọi phép `len + n` kiểm tra overflow trước khi cộng.
- Error path không phát lại response GOM raw, không retry vô hạn. Một lỗi link làm operation thất bại, safe-open relay và khóa channel ở `DESYNCHRONIZED` cho tới khi recovery handshake thành công.

### 11.5 Startup và fail-safe

1. Reset handler/early init đưa toàn bộ GPIO relay về OFF trước khi scheduler chạy.
2. `App_Init()` tạo toàn bộ static object, xác nhận free-list/pool invariants, sau đó mới enable UART DMA.
3. Heartbeat là `{state, progress_counter, deadline_ms}`. `ScpiTask` ở `WAIT_GOM` tối đa 30 giây vẫn healthy khi `GomManagerTask` có progress và deadline chưa hết; watchdog không yêu cầu ScpiTask quay vòng trong lúc callback đang chờ hợp lệ.
4. `HealthTask` chỉ feed independent watchdog khi heartbeat/deadline hợp lệ, DMA producer-consumer không overflow, relay output/feedback one-hot và không có interlock fault.
5. Bất kỳ invariant lỗi, DMA error lặp, stack watermark dưới ngưỡng hoặc stack overflow đều latch fault và emergency-off; relay chỉ được đóng lại qua explicit recovery sau khi feedback an toàn.

## 12. Kiểm thử và nghiệm thu

### Test host tự động

- Parser: short/long form, kiểu parameter sai, out-of-range, command chưa đăng ký, error queue.
- Parser framing: fragment input ở mọi byte boundary, CRLF/LF, line đúng giới hạn, vượt một byte, compound command bị reject và absolute-path behavior nếu feature được bật về sau.
- Mapper/encoder: mỗi PC command tạo đúng enum và đúng command plan GOM; không có input PC nào đi thẳng vào `UART_GOM`.
- Capability: command GOM-805 bị từ chối với GOM-804/UNKNOWN.
- Units/boolean: SI ohm chuyển đúng ở mọi range; `ON/OFF`, `0/1` hợp lệ; numeric boolean khác `0/1` bị reject; compare `0/1/2` thành `LO/PASS/HI`.
- Response parser: value bình thường, `+9.0000E+9`, `+9.9999E+9`, IDN sai, SCPI error GOM, response quá dài.
- Transaction race: timeout đúng lúc completion đến, completion muộn, ID mismatch, reuse cùng channel, partial-apply và verified-write failure.
- DMA: HT/TC wrap, producer lap consumer, atomic snapshot, PC flood, TX stall và discard-until-LF recovery.
- Relay: mọi chuyển đổi `0..8` one-hot, `N -> M` có OFF ở giữa, feedback mismatch, relay-stuck và fault epoch thay đổi giữa switch/TX.
- Static allocation: link map không chứa allocator; `scpiheap` exhaustion được xử lý xác định; đo stack task lẫn MSP/ISR stack.

### Hardware-in-the-loop

1. Không nối GOM: boot/reset/watchdog làm mọi relay OFF.
2. Gắn từng GOM: `ROUT:CHAN n`, `SYST:DEV:IDN?`; đối chiếu model/serial.
3. Đổi mọi cặp kênh 1…8 trên logic analyzer và feedback; không được có hai relay ON. Mô phỏng relay-stuck/feedback sai phải khóa hardware enable.
4. Chạy `CONF:RES`, `SENS:SPE`, `READ?`, compare và `SYST:DEV:ERR?` với GOM thật.
5. Rút cáp/tắt GOM trong query: trả `-365` hoặc communication error, relay mở, channel vào `DESYNCHRONIZED`; command tiếp theo bị khóa cho tới recovery handshake.
6. Tạo response muộn sau timeout và ngay trước request mới; tuyệt đối không được trả kết quả cũ cho request mới.
7. PC flood liên tục ở baud tối đa: `PcIngressTask` không starve parser/health, overflow được đếm và recovery sau LF.
8. Brownout/reset giữa relay switch, UART TX và verified-write; sau boot relay OFF và cache invalid.
9. Đo UART transmission-complete thực (không chỉ DMA memory complete) trước khi đổi relay hoặc bắt đầu verified response phase.
10. Xác minh từng entry command matrix trên đúng model/firmware trước khi đổi `UNVERIFIED` thành `HIL_VERIFIED`.

## 13. Lộ trình triển khai

1. **Board bring-up**: CubeMX, hai UART, relay one-hot, đo signal RS-232/relay.
2. **GOM layer**: switch channel, `SYST:DEV:IDN?`, encoder/response parser và capability cache.
3. **Core measurement API**: `ROUT`, `CONF:RES`, `SENS:*`, `READ?`, error model.
4. **Extended API**: compare, trigger, averaging, temperature, sau đó command riêng GOM-805.
5. **Robustness**: DMA ring, timeout, watchdog, test mất cáp và 8-channel soak test.

## 14. Thông tin cần chốt trước firmware cuối cùng

1. Mỗi máy thực tế là GOM-804, GOM-805 hay hỗn hợp.
2. Baud/parity/stop-bit/flow-control và DTE/DCE pinout của PC lẫn GOM.
3. Relay đang có là DPDT/SPDT, polarity coil, thời gian đóng/mở, auxiliary feedback và phương án hardware one-hot/interlock.
4. Có cần cách ly GND RS-232 hay không.
5. Danh sách command PC bắt buộc ở v1: chỉ đo điện trở hay cần đủ Compare/Binning/Temperature/Drive.

Các thông tin này không thay đổi nguyên tắc parser/mapping; chúng quyết định pin map, cấu hình UART và phạm vi command table phát hành đầu tiên.

## 15. Review remediation matrix

| Finding review | Phần đã tích hợp |
| --- | --- |
| Late response/desynchronization | State `DESYNCHRONIZED`, quiet-drain, purge, IDN handshake |
| PC ingress starvation | Priority mới, byte budget, block/yield bắt buộc |
| Relay kẹt/tiếp điểm hàn | Feedback độc lập, hardware interlock, fault lockout |
| Completion race | `request_id`, completion queue copy, reject ID muộn |
| Compound SCPI path | V1 một command/message; absolute header nếu mở rộng |
| Error code collision | Mã chuẩn `-350/-360/-362/-363/-365` + user errors `100…108` |
| Device-info kéo heap | `scpi_user_config.h`, application-owned `scpiheap`, link-map gate |
| Set command chưa xác nhận | Verified-write và `PARTIAL_APPLY` |
| Watchdog với query dài | Heartbeat state/progress/deadline |
| Circular DMA lap | HT/TC epoch + monotonic producer/consumer counter |
| Buffer off-by-one | `protocol_limits.h`, CRLF/NUL budget, `_Static_assert` |
| Relay ownership race | Runtime single-owner + OFF-only emergency path + fault epoch |
| REL/COMPARE/BIN units | Public SI ohm, explicit suffix, typed result `LO/PASS/HI` |
| Boolean numeric quá rộng | Choice `ON/OFF` hoặc integer exact `0/1` |
| Command docs mâu thuẫn | Machine-readable command matrix `UNVERIFIED/HIL_VERIFIED` |

## 16. Implemented 74HC595 routing baseline

The active firmware baseline uses a 24-bit route image: bits 0–15 are the two measurement-relay drivers for channels 1–8; bits 16–23 are the one-hot RS-232 selector. `relay_matrix_select()` always latches zero, waits break time, validates all-off feedback when installed, then latches exactly the two measurement bits plus one serial-selection bit for the requested channel.

`ROUT:CHAN <n>` now performs this physical route transition before acknowledging the PC. `ROUT:OPEN:ALL` and `*RST` latch an all-off image. At boot, all eight channels are selected one-by-one, queried with `*IDN?`, then queried for `SENS:FUNC?`, `SENS:AUTO?`, and `SENS:RANG?`; unavailable devices remain offline. Runtime commands are rejected until a channel has completed that scan.

Per-channel software acceptance limits are set and read through `ROUT:LIM:LOW <ohm>`, `ROUT:LIM:UPP <ohm>`, `ROUT:LIM:LOW?`, and `ROUT:LIM:UPP?`. `READ?` accepts only a finite numeric value within those limits. `+9.0000E+9`, `+9.9999E+9`, malformed replies, and values outside the configured limits enter the router SCPI error FIFO and are never returned as a valid measurement.

`BOARD_SHIFT_REGISTER_CONFIGURED` remains `0` until legacy CubeMX has been used to configure STM32F411RE SPI, latch, and OE pins. In that state all route requests fail closed and no former direct-relay GPIO is driven.
