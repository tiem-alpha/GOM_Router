TỔNG HỢP LỆNH GOM-805
1. Nhóm Lệnh Đo Lường Cơ Bản (General & Sense)
Các lệnh này thiết lập chức năng đo chính của máy.

Lệnh (Dạng ngắn/Dạng dài)	Mô tả chức năng	Ví dụ / Tham số
SENS:FUNC
(SENSe:FUNCtion)	Chọn chức năng đo chính.	SENS:FUNC OHM (Đo điện trở)
SENS:FUNC VOLT (Đo điện áp)
SENS:AUTO
(SENSe:AUTo)	Bật/tắt chức năng tự động chọn thang đo.	SENS:AUTO ON hoặc SENS:AUTO 1
SENS:RANG
(SENSe:RANGe)	Cài đặt thang đo bằng tay.	SENS:RANG 1000 (Đặt thang đo 1kOhm)
SENS:SPE
(SENSe:SPEed)	Cài đặt tốc độ đo.	SENS:SPE SLOW (Chậm)
SENS:SPE FAST (Nhanh)
SENS:REL:STAT
(SENSe:REL:STATe)	Bật/tắt chức năng đo tương đối (Relative).	SENS:REL:STAT ON
SENS:REL:DATA
(SENSe:REL:DATa)	Cài đặt giá trị tham chiếu cho phép đo tương đối.	SENS:REL:DATA 10.5
SENS:REAL:STAT
(SENSe:REALtime:STATe)	Bật/tắt chế độ hiển thị thời gian thực.	SENS:REAL:STAT ON
SENS:DISP
(SENSe:DISPlay)	Bật/tắt hiển thị kết quả đo trên màn hình.	SENS:DISP ON
TRIG:SOUR
(TRIGger:SOURce)	Chọn nguồn kích khởi cho phép đo.	TRIG:SOUR INT (Kích trong)
TRIG:SOUR EXT (Kích ngoài)
READ?	Kích khởi một phép đo mới và trả về kết quả. (Lệnh truy vấn)	READ?
2. Nhóm Lệnh So Sánh (Compare)
Dùng để thiết lập chức năng so sánh giá trị đo với ngưỡng cho trước.

Lệnh (Dạng ngắn/Dạng dài)	Mô tả chức năng	Ví dụ / Tham số
CALC:COMP:TYPE
(CALCulate:COMPare:TYPE)	Chọn kiểu so sánh (Giá trị tuyệt đối hoặc %).	CALC:COMP:TYPE ABS (Tuyệt đối)
CALC:COMP:LIM:REF
(...:LIMit:REFerence)	Cài đặt giá trị tham chiếu cho kiểu so sánh %.	CALC:COMP:LIM:REF 1000
CALC:COMP:LIM:MODE
(...:LIMit:MODE)	Chọn chế độ ngưỡng (Trong khoảng hoặc Ngoài khoảng).	CALC:COMP:LIM:MODE IN
CALC:COMP:LIM:LOW
(...:LIMit:LOWer)	Cài đặt ngưỡng dưới cho so sánh tuyệt đối.	CALC:COMP:LIM:LOW 95
CALC:COMP:LIM:UPP
(...:LIMit:UPPer)	Cài đặt ngưỡng trên cho so sánh tuyệt đối.	CALC:COMP:LIM:UPP 105
CALC:COMP:PERC:LOW
(...:PERCent:LOWer)	Cài đặt ngưỡng dưới cho so sánh dạng %.	CALC:COMP:PERC:LOW -5
CALC:COMP:PERC:UPP
(...:PERCent:UPPer)	Cài đặt ngưỡng trên cho so sánh dạng %.	CALC:COMP:PERC:UPP 5
CALC:COMP:BEEP
(...:BEEPer)	Bật/tắt còi báo hiệu cho kết quả so sánh (PASS/FAIL).	CALC:COMP:BEEP ON
CALC:COMP:MATH:DATA?
(...:MATH:DATa)	Trả về kết quả tính toán của phép so sánh. (Truy vấn)	CALC:COMP:MATH:DATA?
CALC:COMP:LIM:RES?
(...:LIMit:RESult)	Trả về kết quả so sánh (PASS/FAIL). (Truy vấn)	CALC:COMP:LIM:RES?
3. Nhóm Lệnh Phân Loại (Binning)
Dùng để phân loại linh kiện vào các nhóm (BIN) dựa trên giá trị đo.

Lệnh (Dạng ngắn/Dạng dài)	Mô tả chức năng	Ví dụ / Tham số
BINN:COUN:CLE
(BINNing:COUNt:CLEar)	Xóa tất cả bộ đếm số lượng của các BIN.	BINN:COUN:CLE
BINN:COUN:TOT?
(...:TOTal)	Truy vấn tổng số linh kiện đã đo.	BINN:COUN:TOT?
BINN:COUN:OUT?
(...:OUT)	Truy vấn số linh kiện nằm ngoài tất cả các BIN (OUT).	BINN:COUN:OUT?
BINN<X>:COUN:RES?
(BINNing<X>:...:RESult)	Truy vấn số lượng linh kiện trong BIN <X> (X=1-9).	BINN2:COUN:RES?
BINN<X>:LIM:LOW
(...:LIMit:LOWer)	Cài ngưỡng dưới cho BIN <X> (tuyệt đối).	BINN3:LIM:LOW 90
BINN<X>:LIM:UPP
(...:LIMit:UPPer)	Cài ngưỡng trên cho BIN <X> (tuyệt đối).	BINN3:LIM:UPP 110
BINN<X>:PERC:LOW
(...:PERCent:LOWer)	Cài ngưỡng dưới cho BIN <X> (%).	BINN2:PERC:LOW -10
BINN<X>:PERC:UPP
(...:PERCent:UPPer)	Cài ngưỡng trên cho BIN <X> (%).	BINN2:PERC:UPP 10
BINN:LIM:BEEP
(...:BEEPer)	Bật/tắt còi cho chức năng Binning.	BINN:LIM:BEEP ON
BINN:LIM:DISP
(...:DISP)	Hiển thị kết quả Binning nào đó.	BINN:LIM:DISP BIN1
BINN:LIM:MODE
(...:MODE)	Chọn chế độ ngưỡng cho Binning (ABS/%).	BINN:LIM:MODE ABS
BINN:LIM:REF
(...:REFerence)	Cài giá trị tham chiếu cho Binning dạng %.	BINN:LIM:REF 100
BINN:LIM:RES?
(...:RESult)	Truy vấn kết quả phân loại BIN hiện tại.	BINN:LIM:RES?
4. Nhóm Lệnh Bù Nhiệt & Chuyển Đổi Nhiệt Độ
Lệnh liên quan đến cảm biến nhiệt độ Pt100 và chuyển đổi giá trị.

Lệnh (Dạng ngắn/Dạng dài)	Mô tả chức năng
TEMP:COMP:CORR
(TEMPerature:COMPensate:CORRect)	Bật/tắt chức năng bù nhiệt độ.
TEMP:COMP:COEF
(...:COEFficient)	Cài đặt hệ số nhiệt độ (α) cho vật liệu cần bù.
TEMP:CONV:RES
(TEMPerature:CONVersion:RESistance)	Cài đặt giá trị điện trở ban đầu để chuyển đổi sang nhiệt độ.
TEMP:CONV:TEMP
(...:TEMPerature)	Cài đặt nhiệt độ ban đầu để chuyển đổi.
TEMP:CONV:CONS
(...:CONStant)	Cài đặt hằng số chuyển đổi.
TEMP:CONV:DISP
(...:DISPlay)	Chọn hiển thị giá trị Điện trở hay Nhiệt độ.
TEMP:CONV:MATH:DATA?
(...:MATH:DATa)	Truy vấn giá trị sau khi chuyển đổi.
TEMP:STAT
(TEMPerature:STATe)	Bật/tắt chế độ đo nhiệt độ.
TEMP:DATA?
(TEMPerature:DATa)	Truy vấn giá trị nhiệt độ hiện tại từ cảm biến.
5. Nhóm Lệnh Quét Kênh (Scan) - Dành cho model đa kênh
Lệnh tiêu biểu	Mô tả chức năng
CALC:SCAN:CHAN	Chọn kênh sẽ quét.
CALC:SCAN:DEL	Cài đặt thời gian trễ giữa các lần quét kênh.
CALC:SCAN:LIM:REF/LOW/UPP	Cài ngưỡng so sánh cho chức năng Scan.
MEAS<X>?	Truy vấn giá trị đo của kênh <X>.
SHOW?	Hiển thị/trả về dữ liệu quét mới nhất.
6. Nhóm Lệnh Nguồn Phát (Source)
Lệnh	Mô tả chức năng
SOUR:DRY
(SOURce:DRY)	Chọn chế độ mạch khô (Dry Circuit) để đo điện trở tiếp xúc.
SOUR:DRIV
(SOURce:DRIVe)	Chọn chế độ nguồn dòng DC hoặc PWM.
7. Nhóm Thiết Lập Hệ Thống & Đo Lường (System & Setup)
Lệnh	Mô tả chức năng
SYST:AVER:STAT	Bật/tắt chế độ lấy trung bình.
SYST:AVER:DATA	Cài đặt số lần đo để lấy trung bình.
SYST:MDEL:STAT	Bật/tắt trễ đo lường.
TRIG:DEL:STAT/DATA	Bật/tắt và cài đặt thời gian trễ kích khởi.
TRIG:EDGE	Chọn cạnh xung kích (Lên/Xuống).
TEMP:UNIT	Chọn đơn vị nhiệt độ (°C / °F).
SYST:LFR	Cài đặt tần số đo cho tín hiệu AC.
SYST:PWM:ON/OFF	Cài đặt thời gian ON/OFF cho tín hiệu nguồn dạng xung.
8. Nhóm Cấu Hình Hệ Thống Máy (System Config)
Lệnh	Mô tả chức năng
*IDN?	Truy vấn thông tin nhận dạng máy.
SYST:SER	Cài đặt cấu hình cổng Serial.
SYST:BRIG	Cài đặt độ sáng màn hình.
USER<X>:ACT/LOG	Kích hoạt và cài đặt logic cho chân I/O người dùng.
SYST:HAND	Cài đặt chế độ cho Handler Interface.
SYST:KEYC:BEEP	Bật/tắt tiếng bíp khi nhấn phím.
SYST:VOLT:PROT	Cài đặt ngưỡng bảo vệ quá áp đầu vào.
SYST:ERR?	Truy vấn lỗi hệ thống.
SYST:LOC	Chuyển máy về chế độ Local (điều khiển trực tiếp).
9. Nhóm Lưu Trữ & Trạng Thái (Memory & Status)
Lệnh	Mô tả chức năng
MEM:SAV	Lưu cấu hình hiện tại vào bộ nhớ trong.
MEM:REC	Gọi lại cấu hình đã lưu.
MEM:CLE	Xóa cấu hình trong bộ nhớ.
MEM:STAT?	Truy vấn trạng thái bộ nhớ.
STAT:PRES	Đặt lại thanh ghi trạng thái về mặc định.
STAT:QUES:ENAB/EVEN?	Quản lý thanh ghi sự kiện cảnh báo.
10. Lệnh Chung IEEE 488.2
Đây là các lệnh chuẩn bắt buộc.

Lệnh	Mô tả chức năng
*CLS	Xóa thanh ghi trạng thái.
*ESE	Cài đặt thanh ghi cho phép sự kiện chuẩn.
*ESR?	Truy vấn thanh ghi sự kiện chuẩn.
*OPC	Báo hiệu hoàn tất lệnh.
*RST	Khôi phục cài đặt gốc cho máy.
*SRE	Cài đặt thanh ghi cho phép yêu cầu dịch vụ.
*STB?	Đọc thanh ghi trạng thái.
*TRG	Kích khởi một phép đo (Trigger).

============================================
Tổng Hợp Chi Tiết: General Commands
1. SENSe:FUNCtion
Chức năng: Chọn hoặc truy vấn chế độ đo chính của máy.

Cú pháp: SENSe:FUNCtion {OHM|COMP|BIN|TC|TCONV|SCAN|DIODE}

Truy vấn: SENSe:FUNCtion?

Tham số/Trả về:

OHM: Chế độ đo điện trở.

COMP: Chế độ so sánh.

BIN: Chế độ phân loại (Binning).

TC: Chế độ bù nhiệt.

TCONV: Chế độ chuyển đổi nhiệt độ.

SCAN: Chế độ quét kênh.

DIODE: Chế độ đo Diode.

Ví dụ: SENS:FUNC OHM (Đặt máy vào chế độ đo Ohm).

Lưu ý quan trọng: Để chuyển sang chế độ đo Nhiệt độ (hiển thị OHM+T), hãy sử dụng lệnh trong mục TEMPerature:STATe.

2. SENSe:AUTo
Chức năng: Bật/tắt chế độ tự động chọn thang đo.

Cú pháp: SENSe:AUTo <NR1> | {OFF|ON}

Truy vấn: SENSe:AUTo?

Tham số/Trả về:

0 hoặc OFF: Tắt tự động.

1 hoặc ON: Bật tự động.

Ví dụ: SENS:AUT ON (Bật chế độ tự động chọn thang đo).

3. SENSe:RANGe
Chức năng: Cài đặt hoặc truy vấn thang đo cố định (khi Auto tắt).

Cú pháp: SENSe:RANGe <NRf>

Truy vấn: SENSe:RANGe?

Tham số: <NRf> từ 5E-3 (5mΩ) đến 5E+6 (5MΩ).

Trả về: <NR3> (VD: 5.0000E-3).

Ví dụ:

SENS:RANG 0.005 (Đặt thang đo 5mΩ).

SENS:RANG? → >5.0000E-3

4. SENSe:SPEed
Chức năng: Cài đặt hoặc truy vấn tốc độ đo.

Cú pháp: SENSe:SPEed {SLOW|FAST}

Truy vấn: SENSe:SPEed?

Tham số/Trả về: SLOW (Chậm) hoặc FAST (Nhanh).

Ví dụ: SENS:SPE FAST (Đặt tốc độ đo nhanh).

5. SENSe:REL:STATe
Chức năng: Bật/tắt chức năng đo tương đối (Relative).

Cú pháp: SENSe:REL:STATe <NR1> | {OFF|ON}

Truy vấn: SENSe:REL:STATe?

Tham số/Trả về: 0 hoặc OFF (Tắt), 1 hoặc ON (Bật).

Ví dụ: SENS:REL:STAT OFF

Lưu ý: Chỉ có thể bật khi có giá trị đo đang hiển thị.

6. SENSe:REL:DATa
Chức năng: Cài đặt hoặc truy vấn giá trị tham chiếu cho phép đo tương đối.

Cú pháp: SENSe:REL:DATa <NRf>

Truy vấn: SENSe:REL:DATa?

Tham số: <NRf> từ 0.0000 đến 500.00 (đơn vị tự động theo thang đo).

Trả về: <NR3> (VD: 4.9032E+2).

Ví dụ:

SENS:REL:DAT 490.32 (Đặt giá trị tương đối là 490.32Ω).

SENS:REL:DAT? → >4.9032E+2

Lưu ý: Chỉ có thể cài đặt khi có giá trị đo đang hiển thị.

7. SENSe:REALtime:STATe
Chức năng: Bật/tắt chức năng đo thời gian thực.

Cú pháp: SENSe:REALtime:STATe <NR1> | {OFF|ON}

Truy vấn: SENSe:REALtime:STATe?

Tham số/Trả về: 0 hoặc OFF (Tắt), 1 hoặc ON (Bật).

Ví dụ: SENS:REAL:STAT ON

8. SENSe:DISPlay
Chức năng: Chọn chế độ hiển thị (Bình thường hoặc Đơn giản).

Cú pháp: SENSe:DISPlay <NR1> | {OFF|ON}

Truy vấn: SENSe:DISPlay?

Tham số/Trả về:

0 hoặc OFF: Chế độ hiển thị bình thường (Normal).

1 hoặc ON: Chế độ hiển thị đơn giản (Simple).

Ví dụ: SENS:DISP OFF (Đặt về chế độ hiển thị bình thường).

9. TRIGger:SOURce
Chức năng: Chọn hoặc truy vấn nguồn kích khởi.

Cú pháp: TRIGger:SOURce {INT|EXT}

Truy vấn: TRIGger:SOURce?

Tham số/Trả về: INT (Kích trong) hoặc EXT (Kích ngoài).

Ví dụ: TRIG:SOUR EXT

10. READ?
Chức năng: Lệnh truy vấn, kích hoạt một lần đo và trả về giá trị đo mới.

Cú pháp: READ?

Trả về: <NR3> (VD: +2.2012E+0).

Ví dụ: READ? → >+2.2012E+0

Giá trị đặc biệt:

+9.0000E+9: Giá trị vượt quá thang đo (Over Range).

+9.9999E+9: Phát hiện bảo vệ quá áp (HVP).

Tổng Hợp Chi Tiết: Compare Commands
1. CALCulate:COMPare:TYPE
Chức năng: Chọn kiểu giá trị để so sánh.

Cú pháp: CALCulate:COMPare:TYPE {OHM|TC}

Truy vấn: CALCulate:COMPare:TYPE?

Tham số/Trả về:

OHM: So sánh giá trị điện trở.

TC: So sánh giá trị hệ số nhiệt độ.

Ví dụ: CALC:COMP:TYPE TC

2. CALCulate:COMPare:LIMit:REFerence
Chức năng: Đặt giá trị tham chiếu cho phép so sánh dạng %.

Cú pháp: CALCulate:COMPare:LIMit:REFerence {<NRf>[,<String>]}

Truy vấn: CALCulate:COMPare:LIMit:REFerence?

Tham số:

<NRf>: Từ 000.0001 đến 999.9999.

<String> (Tùy chọn): Đơn vị mohm, ohm, kohm, maohm. Nếu không đặt, đơn vị theo thang đo hiện tại.

Trả về: <NR3> (VD: 10.0000E-3).

Ví dụ:

CALC:COMP:LIM:REF 10.00,mohm (Đặt tham chiếu là 10.00 mΩ).

CALC:COMP:LIM:REF? → >10.0000E-3

3. CALCulate:COMPare:LIMit:MODE
Chức năng: Chọn chế độ so sánh.

Cú pháp: CALCulate:COMPare:LIMit:MODE {ABS|DPER|PER}

Truy vấn: CALCulate:COMPare:LIMit:MODE?

Tham số/Trả về:

ABS: So sánh theo giá trị tuyệt đối.

DPER: So sánh theo % lệch so với giá trị tham chiếu (Delta %).

PER: Hiển thị kết quả là % của giá trị tham chiếu.

Ví dụ: CALC:COMP:LIM:MODE ABS

4. CALCulate:COMPare:LIMit:LOWer
Chức năng: Đặt ngưỡng dưới cho chế độ ABS.

Điều kiện: Chỉ hoạt động khi MODE là ABS.

Cú pháp: CALCulate:COMPare:LIMit:LOWer {<NRf>[,<String>]}

Truy vấn: CALCulate:COMPare:LIMit:LOWer?

Tham số: Tương tự REFerence.

Trả về: <NR3>.

Ví dụ: CALC:COMP:LIM:LOW 0.95,kohm (Đặt ngưỡng dưới là 0.95kΩ).

5. CALCulate:COMPare:LIMit:UPPer
Chức năng: Đặt ngưỡng trên cho chế độ ABS.

Điều kiện: Chỉ hoạt động khi MODE là ABS.

Cú pháp: CALCulate:COMPare:LIMit:UPPer {<NRf>[,<String>]}

Truy vấn: CALCulate:COMPare:LIMit:UPPer?

Tham số: Tương tự REFerence.

Trả về: <NR3>.

Ví dụ: CALC:COMP:LIM:UPP 0.123,maohm (Đặt ngưỡng trên là 0.123MΩ).

6. CALCulate:COMPare:PERCent:LOWer
Chức năng: Đặt ngưỡng dưới (%) cho chế độ DPER hoặc PER.

Điều kiện: Chỉ hoạt động khi MODE không phải ABS.

Cú pháp: CALCulate:COMPare:PERCent:LOWer <NRf>

Truy vấn: CALCulate:COMPare:PERCent:LOWer?

Tham số: <NRf> từ 000.00 đến 999.99 (Luôn là số dương, máy hiểu là -X%).

Trả về: <NR2>.

Ví dụ: CALC:COMP:PERC:LOW 10.00 (Đặt ngưỡng dưới là -10.00%).

7. CALCulate:COMPare:PERCent:UPPer
Chức năng: Đặt ngưỡng trên (%) cho chế độ DPER hoặc PER.

Điều kiện: Chỉ hoạt động khi MODE không phải ABS.

Cú pháp: CALCulate:COMPare:PERCent:UPPer <NRf>

Truy vấn: CALCulate:COMPare:PERCent:UPPer?

Tham số: <NRf> từ 000.00 đến 999.99.

Trả về: <NR2>.

Ví dụ: CALC:COMP:PERC:UPP 90.00 (Đặt ngưỡng trên là +90.00%).

8. CALCulate:COMPare:BEEPer
Chức năng: Cài đặt chế độ còi cho chức năng Compare.

Cú pháp: CALCulate:COMPare:BEEPer {OFF|PASS|FAIL}

Truy vấn: CALCulate:COMPare:BEEPer?

Tham số/Trả về:

OFF: Tắt còi.

PASS: Kêu khi kết quả ĐẠT.

FAIL: Kêu khi kết quả KHÔNG ĐẠT.

Ví dụ: CALC:COMP:BEEP FAIL

9. CALCulate:COMPare:MATH:DATa?
Chức năng: Truy vấn giá trị sai lệch (%) của phép đo so với giá trị tham chiếu.

Cú pháp: CALCulate:COMPare:MATH:DATa?

Trả về: <NR3> (VD: +0.3658E+2, tức là 36.58%).

Ví dụ: CALC:COMP:MATH:DAT? → >+0.3658E+2

10. CALCulate:COMPare:LIMit:RESult?
Chức năng: Truy vấn kết quả của phép so sánh cuối cùng.

Cú pháp: CALCulate:COMPare:LIMit:RESult?

Trả về: <NR1>:

0: Kết quả Thấp (LO) - Dưới ngưỡng.

1: Kết quả Đạt (IN) - Trong ngưỡng.

2: Kết quả Cao (HI) - Trên ngưỡng.

Ví dụ: CALC:COMP:LIM:RES? → >2 (Kết quả là HI).

Tổng Hợp Chi Tiết: Binning Commands (Chỉ dành cho GOM-805)
1. BINNing:COUNt:CLEar
Chức năng: Xóa tất cả bộ đếm kết quả của chức năng phân loại (Binning).

Cú pháp: BINNing:COUNt:CLEar

Ví dụ: BINN:COUN:CLE

2. BINNing:COUNt:TOTal?
Chức năng: Truy vấn tổng số linh kiện đã được kiểm tra (tổng tất cả các BIN, bao gồm cả OUT).

Cú pháp: BINNing:COUNt:TOTal?

Trả về: <NR1> từ 0 đến 999999999.

Ví dụ: BINN:COUN:TOT? → >150 (Đã đo tổng cộng 150 linh kiện).

3. BINNing:COUNt:OUT?
Chức năng: Truy vấn số linh kiện không đạt (OUT), tức là không rơi vào bất kỳ BIN nào.

Cú pháp: BINNing:COUNt:OUT?

Trả về: <NR1> từ 0 đến 99999999.

Ví dụ: BINN:COUN:OUT? → >50 (Có 50 linh kiện bị loại).

4. BINNing<X>:COUNt:RESult?
Chức năng: Truy vấn số linh kiện đạt (IN) cho một BIN cụ thể <X>.

Cú pháp: BINNing<X>:COUNt:RESult?

Tham số: <X> là số BIN, từ 1 đến 8.

Trả về: <NR1> từ 0 đến 99999999.

Ví dụ: BINN1:COUN:RES? → >100 (BIN 1 có 100 linh kiện đạt).

5. BINNing<X>:LIMit:LOWer
Chức năng: Cài đặt hoặc truy vấn ngưỡng dưới dạng giá trị tuyệt đối cho một BIN.

Cú pháp: BINNing<X>:LIMit:LOWer {<NRf>[,<String>]}

Tham số:

<X>: 1 đến 8.

<NRf>: Giá trị từ 000.0000 đến 999.9999.

<String> (Tùy chọn): Đơn vị (mohm, ohm, kohm, maohm).

Trả về: <NR3>.

Ví dụ:

BINN1:LIM:LOW 23.8,kohm

BINN1:LIM:LOW? → >23.8000E+3

6. BINNing<X>:LIMit:UPPer
Chức năng: Cài đặt hoặc truy vấn ngưỡng trên dạng giá trị tuyệt đối cho một BIN.

Cú pháp: BINNing<X>:LIMit:UPPer {<NRf>[,<String>]}

Tham số: Tương tự lệnh LOWer.

Trả về: <NR3>.

Ví dụ:

BINN1:LIM:UPP 0.95,maohm

BINN1:LIM:UPP? → >0.9500E+6

7. BINNing<X>:PERCent:LOWer
Chức năng: Cài đặt hoặc truy vấn ngưỡng dưới dạng % lệch (Δ%) so với giá trị tham chiếu cho một BIN.

Cú pháp: BINNing<X>:PERCent:LOWer <NRf>

Tham số:

<X>: 1 đến 8.

<NRf>: 000.00 đến 999.99 (luôn là số dương, hệ thống hiểu là giá trị âm, ví dụ: 10.15 → -10.15%).

Trả về: <NR2>.

Ví dụ: BINN1:PERC:LOW 10.15 (Đặt ngưỡng dưới là -10.15%).

8. BINNing<X>:PERCent:UPPer
Chức năng: Cài đặt hoặc truy vấn ngưỡng trên dạng % lệch (Δ%) so với giá trị tham chiếu cho một BIN.

Cú pháp: BINNing<X>:PERCent:UPPer <NRf>

Tham số: Tương tự lệnh PERCent:LOWer.

Trả về: <NR2>.

Ví dụ: BINN1:PERC:UPP 150.95 (Đặt ngưỡng trên là +150.95%).

9. BINNing:LIMit:BEEPer
Chức năng: Bật/tắt còi cho chức năng Binning.

Cú pháp: BINNing:LIMit:BEEPer {OFF|PASS|FAIL}

Tham số/Trả về:

OFF: Tắt còi.

PASS: Còi kêu khi kết quả Đạt.

FAIL: Còi kêu khi kết quả Không đạt (OUT).

Ví dụ: BINN:LIM:BEEP OFF

10. BINNing:LIMit:DISP
Chức năng: Chọn chế độ hiển thị cho chức năng Binning.

Cú pháp: BINNing:LIMit:DISP {COMP|COUNT}

Tham số/Trả về:

COMP: Hiển thị kết quả so sánh.

COUNT: Hiển thị số đếm của các BIN.

Ví dụ: BINN:LIM:DISP COMP

11. BINNing:LIMit:MODE
Chức năng: Chọn chế độ cài đặt ngưỡng (Tuyệt đối hoặc % lệch).

Cú pháp: BINNing:LIMit:MODE {ABS|DPER}

Tham số/Trả về:

ABS: Ngưỡng giới hạn là giá trị tuyệt đối.

DPER: Ngưỡng giới hạn là % lệch so với giá trị tham chiếu (Delta Percent).

Ví dụ: BINN:LIM:MODE DPER

12. BINNing:LIMit:REFerence
Chức năng: Cài đặt giá trị tham chiếu cho chức năng Binning (dùng cho chế độ DPER).

Cú pháp: BINNing:LIMit:REFerence {<NRf>[,<String>]}

Tham số:

<NRf>: 000.0001 đến 999.9999.

<String> (Tùy chọn): Đơn vị.

Trả về: <NR3>.

Ví dụ: BINN:LIM:REF 100 → >100.0000E+0

13. BINNing:LIMit:RESult?
Chức năng: Truy vấn kết quả phân loại BIN của lần đo cuối cùng.

Cú pháp: BINNing:LIMit:RESult?

Trả về: <NR1>:

1 đến 8: Linh kiện đạt và rơi vào BIN tương ứng.

9: Linh kiện không đạt (OUT).

Ví dụ: BINN:LIM:RES? → >1 (Kết quả đạt và rơi vào BIN 1).

Tổng Hợp Chi Tiết: Temperature Compensate Commands
1. TEMPerature:COMPensate:CORRect
Chức năng: Cài đặt hoặc truy vấn giá trị nhiệt độ tham chiếu cho chức năng bù nhiệt.

Cú pháp: TEMPerature:COMPensate:CORRect <NRf>

Tham số: <NRf> từ -50.0 đến 399.9 (Đơn vị: ºC).

Trả về: <NR2>.

Ví dụ: TEMP:COMP:CORR 25.5 (Đặt nhiệt độ tham chiếu là 25.5ºC).

2. TEMPerature:COMPensate:COEFficient
Chức năng: Cài đặt hoặc truy vấn hệ số nhiệt độ (TCR) cho chức năng bù nhiệt.

Cú pháp: TEMPerature:COMPensate:COEFficient <NR1>

Tham số/Trả về: <NR1> từ -9999 đến +9999 (Đơn vị: ppm).

Ví dụ: TEMP:COMP:COEF 3930 (Đặt hệ số nhiệt độ là 3930 ppm).

Tổng Hợp Chi Tiết: Temperature Conversion Commands
1. TEMPerature:CONVersion:RESistance
Chức năng: Cài đặt hoặc truy vấn giá trị điện trở ban đầu (R0) cho chức năng chuyển đổi nhiệt độ.

Cú pháp: TEMPerature:CONVersion:RESistance {<NRf>[,<String>]}

Tham số: <NRf> từ 000.0001 đến 999.9999. Có thể kèm đơn vị.

Trả về: <NR3>.

Ví dụ: TEMP:CONV:RES 10.00,maohm (Đặt R0 là 10.00MΩ).

2. TEMPerature:CONVersion:TEMPerature
Chức năng: Cài đặt hoặc truy vấn giá trị nhiệt độ ban đầu (T0) cho chức năng chuyển đổi nhiệt độ.

Cú pháp: TEMPerature:CONVersion:TEMPerature <NRf>

Tham số: <NRf> từ -50.0 đến 399.9 (Đơn vị: ºC).

Trả về: <NR2>.

Ví dụ: TEMP:CONV:TEMP 25.6 (Đặt T0 là 25.6ºC).

3. TEMPerature:CONVersion:CONStant
Chức năng: Cài đặt hoặc truy vấn hằng số nhiệt độ (α) cho công thức chuyển đổi.

Cú pháp: TEMPerature:CONVersion:CONStant <NRf>

Tham số: <NRf> từ 0.0 đến 999.9.

Trả về: <NR2>.

Ví dụ: TEMP:CONV:CONS 235 (Đặt hằng số là 235).

4. TEMPerature:CONVersion:DISPlay
Chức năng: Chọn chế độ hiển thị kết quả cho chức năng chuyển đổi nhiệt độ.

Cú pháp: TEMPerature:CONVersion:DISPlay <NR1>

Tham số/Trả về:

1: Hiển thị ΔT (Độ chênh lệch nhiệt độ).

2: Hiển thị T (Nhiệt độ tuyệt đối).

Ví dụ: TEMP:CONV:DISP 1 (Chọn hiển thị ΔT).

5. TEMPerature:CONVersion:MATH:DATa?
Chức năng: Truy vấn giá trị sai lệch (hoặc kết quả) của phép chuyển đổi nhiệt độ.

Cú pháp: TEMPerature:CONVersion:MATH:DATa?

Trả về: <NR3> (Ví dụ: 1.250E+2).

Ví dụ: TEMP:CONV:MATH:DAT?

Tổng Hợp Chi Tiết: Temperature Commands
1. TEMPerature:STATe
Chức năng: Bật/tắt chế độ đo nhiệt độ (chế độ OHM+TEMP).

Cú pháp: TEMPerature:STATe {<NR1>|OFF|ON}

Truy vấn: TEMPerature:STATe?

Tham số/Trả về:

0 hoặc OFF: Tắt.

1 hoặc ON: Bật.

Ví dụ: TEMP:STAT ON (Bật chế độ đo nhiệt độ).

Lưu ý: Đây là lệnh dùng để chuyển sang chế độ TEMP (OHM+T) như đã đề cập trong chú thích của lệnh SENSe:FUNCtion.

2. TEMPerature:DATa?
Chức năng: Truy vấn giá trị nhiệt độ đo được từ cảm biến PT-100 (đơn vị độ C).

Cú pháp: TEMPerature:DATa?

Trả về: <NR3> trong khoảng -50.0 đến 399.9.

Ví dụ: TEMP:DAT? → >0.250E+2 (Nhiệt độ là 25ºC).

Tổng Hợp Chi Tiết: Scan Commands
1. CALCulate:SCAN:CHANnel
Chức năng: Cài đặt hoặc truy vấn số lượng kênh sẽ quét.

Cú pháp: CALCulate:SCAN:CHANnel <NR1>

Truy vấn: CALCulate:SCAN:CHANnel?

Tham số/Trả về: <NR1> từ 1 đến 100.

Ví dụ: CALC:SCAN:CHAN 5 (Đặt quét 5 kênh đầu tiên).

2. CALCulate:SCAN:DELay
Chức năng: Cài đặt hoặc truy vấn thời gian trễ giữa các lần quét kênh.

Cú pháp: CALCulate:SCAN:DELay <NR1>

Truy vấn: CALCulate:SCAN:DELay?

Tham số/Trả về: <NR1> từ 400 đến 30000 (Đơn vị: ms).

Ví dụ: CALC:SCAN:DEL 500 (Đặt trễ 500ms).

3. CALCulate:SCAN:LIMit:REFerence
Chức năng: Cài đặt hoặc truy vấn giá trị tham chiếu cho chức năng so sánh trong Scan.

Cú pháp: CALCulate:SCAN:LIMit:REFerence {<NRf>[,<String>]}

Tham số: Tương tự các lệnh REFerence khác (000.0001 đến 999.9999, có thể kèm đơn vị).

Trả về: <NR3>.

Ví dụ: CALC:SCAN:LIM:REF 10.00,mohm → >10.0000E-3

4. CALCulate:SCAN:LIMit:MODE
Chức năng: Chọn chế độ so sánh cho chức năng Scan (Tuyệt đối hoặc %).

Cú pháp: CALCulate:SCAN:LIMit:MODE {ABS|DPER}

Tham số/Trả về:

ABS: So sánh theo giá trị tuyệt đối.

DPER: So sánh theo % lệch (Delta %).

Ví dụ: CALC:SCAN:LIM:MODE ABS

5. CALCulate:SCAN:LIMit:LOWer
Chức năng: Cài đặt hoặc truy vấn ngưỡng dưới (tuyệt đối) cho Scan.

Cú pháp: CALCulate:SCAN:LIMit:LOWer {<NRf>[,<String>]}

Tham số: Tương tự lệnh REFerence.

Trả về: <NR3>.

Ví dụ: CALC:SCAN:LIM:LOW 1.37,kohm → >1.3700E+3

6. CALCulate:SCAN:LIMit:UPPer
Chức năng: Cài đặt hoặc truy vấn ngưỡng trên (tuyệt đối) cho Scan.

Cú pháp: CALCulate:SCAN:LIMit:UPPer {<NRf>[,<String>]}

Tham số: Tương tự lệnh REFerence.

Trả về: <NR3>.

Ví dụ: CALC:SCAN:LIM:UPP 0.123,maohm → >0.1230E+6

7. CALCulate:SCAN:PERCent:LOWer
Chức năng: Cài đặt hoặc truy vấn ngưỡng dưới (%) cho Scan (dùng khi MODE là DPER).

Cú pháp: CALCulate:SCAN:PERCent:LOWer <NRf>

Tham số: <NRf> từ 000.00 đến 999.99.

Trả về: <NR2>.

Ví dụ: CALC:SCAN:PERC:LOW 10.00 (Đặt -10.00%).

8. CALCulate:SCAN:PERCent:UPPer
Chức năng: Cài đặt hoặc truy vấn ngưỡng trên (%) cho Scan (dùng khi MODE là DPER).

Cú pháp: CALCulate:SCAN:PERCent:UPPer <NRf>

Tham số: <NRf> từ 000.00 đến 999.99.

Trả về: <NR2>.

Ví dụ: CALC:SCAN:PERC:UPP 90.00 (Đặt +90.00%).

9. MEASure<X>?
Chức năng: Truy vấn kết quả của một kênh cụ thể trong chế độ Scan, bao gồm trạng thái (LO/IN/HI) và giá trị đo.

Cú pháp: MEASure<X>?

Tham số: <X> là số kênh từ 1 đến 100.

Trả về: Chuỗi dạng 0|1|2,<NR3>, trong đó:

0: Kết quả Thấp (LO).

1: Kết quả Đạt (IN).

2: Kết quả Cao (HI).

<NR3>: Giá trị đo được.

Ví dụ: MEAS1? → >1,+0.9978E+1 (Kênh 1 Đạt, giá trị 9.978Ω).

10. SHOW?
Chức năng: Truy vấn trạng thái phán đoán (LO/IN/HI) của tất cả các kênh (tối đa 100) trong chế độ Scan.

Cú pháp: SHOW?

Trả về: <String> gồm 100 ký tự, trong đó:

0: LO

1: IN

2: HI

_: Kênh không hoạt động.

Ví dụ: SHOW? → >1111111111___ (10 kênh đầu Đạt, các kênh còn lại không hoạt động).

Tổng Hợp Chi Tiết: Source Commands (Đã có trong danh sách tổng quan)
1. SOURce:DRY
Chức năng: Bật/tắt chế độ mạch khô (Dry Circuit), dùng để đo điện trở tiếp xúc với điện áp thấp.

Cú pháp: SOURce:DRY {<NR1>|OFF|ON} (tham khảo từ cấu trúc lệnh tương tự)

Ví dụ: SOUR:DRY ON

2. SOURce:DRIVe
Chức năng: Chọn chế độ nguồn dòng (DC hoặc PWM).

Cú pháp: SOURce:DRIVe {DC|PWM} (tham khảo)

Ví dụ: SOUR:DRIV DC

\Tổng Hợp Chi Tiết: Source Commands (Chỉ dành cho GOM-805)
1. SOURce:DRY
Chức năng: Bật/tắt chế độ mạch khô (Dry Circuit), giới hạn điện áp hở mạch ở mức thấp (thường dùng đo điện trở tiếp xúc).

Cú pháp: SOURce:DRY {<NR1> | OFF|ON}

Truy vấn: SOURce:DRY?

Tham số/Trả về:

0 hoặc OFF: Tắt.

1 hoặc ON: Bật.

Ví dụ: SOUR:DRY ON

2. SOURce:DRIVe
Chức năng: Chọn chế độ nguồn dòng cho phép đo.

Cú pháp: SOURce:DRIVe <NR1>

Truy vấn: SOURce:DRIVe?

Tham số/Trả về:

1: Chế độ DC+ (dòng một chiều dương).

2: Chế độ DC- (dòng một chiều âm).

3: Chế độ PULSE (dòng xung).

4: Chế độ PWM (điều chế độ rộng xung).

5: Chế độ ZERO (dòng về 0).

6: Chế độ STANDBY (chờ).

Ví dụ: SOUR:DRIV 3 (Đặt chế độ PULSE).

Tổng Hợp Chi Tiết: Meas. Setup Commands
1. SYSTem:AVERage:STATe
Chức năng: Bật/tắt chức năng lấy giá trị trung bình.

Cú pháp: SYSTem:AVERage:STATe <NR1> | {OFF|ON}

Truy vấn: SYSTem:AVERage:STATe?

Tham số/Trả về: 0 hoặc OFF (Tắt), 1 hoặc ON (Bật).

Ví dụ: SYST:AVER:STAT OFF

2. SYSTem:AVERage:DATa
Chức năng: Cài đặt hoặc truy vấn số phép đo dùng để tính trung bình.

Cú pháp: SYSTem:AVERage:DATa <NR1>

Truy vấn: SYSTem:AVERage:DATa?

Tham số/Trả về: <NR1> từ 2 đến 100.

Ví dụ: SYST:AVER:DAT 5 (Tính trung bình 5 lần đo).

3. SYSTem:MDELay:STATe
Chức năng: Bật/tắt chức năng trễ phép đo (Measurement Delay).

Cú pháp: SYSTem:MDELay:STATe <NR1> | {OFF|ON}

Truy vấn: SYSTem:MDELay:STATe?

Tham số/Trả về: 0 hoặc OFF (Tắt), 1 hoặc ON (Bật).

Ví dụ: SYST:MDEL:STAT OFF

4. SYSTem:MDELay:DATa
Chức năng: Cài đặt hoặc truy vấn thời gian trễ phép đo.

Cú pháp: SYSTem:MDELay:DATa <NRf>

Truy vấn: SYSTem:MDELay:DATa?

Tham số/Trả về: <NRf> từ 0.000 đến 100.000 (Đơn vị: ms). Độ phân giải là 1ms cho giá trị dưới 1s và 0.1s cho giá trị trên 1s.

Ví dụ:

SYST:MDEL:DAT 1.105 (Đặt trễ 1.1s).

SYST:MDEL:DAT? → >001.100

5. TRIGger:DELay:STATe
Chức năng: Bật/tắt chức năng trễ kích khởi (Trigger Delay).

Cú pháp: TRIGger:DELay:STATe <NR1> | {OFF|ON}

Truy vấn: TRIGger:DELay:STATe?

Tham số/Trả về: 0 hoặc OFF (Tắt), 1 hoặc ON (Bật).

Ví dụ: TRIG:DEL:STAT ON

Lưu ý: Tài liệu gốc có vẻ bị ngược tham số <NR1> (0:ON, 1:OFF), hãy kiểm tra lại trên máy nếu cần.

6. TRIGger:DELay:DATa
Chức năng: Cài đặt hoặc truy vấn thời gian trễ kích khởi.

Cú pháp: TRIGger:DELay:DATa <NR1>

Truy vấn: TRIGger:DELay:DATa?

Tham số/Trả về: <NR1> từ 0 đến 1000 (Đơn vị: ms).

Ví dụ: TRIG:DEL:DAT 100 (Đặt trễ 100ms).

7. TRIGger:EDGE
Chức năng: Chọn cạnh kích khởi (lên hoặc xuống) cho tín hiệu Trigger ngoài.

Cú pháp: TRIGger:EDGE {RISING|FALLING}

Truy vấn: TRIGger:EDGE?

Tham số/Trả về: RISING (Sườn lên), FALLING (Sườn xuống).

Ví dụ: TRIG:EDGE FALLING

8. TEMPerature:UNIT
Chức năng: Chọn đơn vị hiển thị nhiệt độ (chỉ dùng cho hiển thị).

Cú pháp: TEMPerature:UNIT {DEGC|DEGF}

Truy vấn: TEMPerature:UNIT?

Tham số/Trả về: DEGC (Độ C), DEGF (Độ F).

Ví dụ: TEMP:UNIT DEGF

9. TEMPerature:AMBient:STATe
Chức năng: Bật/tắt chức năng sử dụng nhiệt độ môi trường do người dùng cài đặt (thay vì từ cảm biến PT-100).

Cú pháp: TEMPerature:AMBient:STATe <NR1> | {OFF|ON}

Truy vấn: TEMPerature:AMBient:STATe?

Tham số/Trả về: 0 hoặc OFF (Tắt), 1 hoặc ON (Bật).

Ví dụ: TEMP:AMB:STAT OFF

10. TEMPerature:AMBient:DATa
Chức năng: Cài đặt hoặc truy vấn giá trị nhiệt độ môi trường do người dùng đặt (dùng cho bù nhiệt và chuyển đổi nhiệt độ khi AMBient:STATe là ON).

Cú pháp: TEMPerature:AMBient:DATa <NRf>

Truy vấn: TEMPerature:AMBient:DATa?

Tham số/Trả về: <NRf> / <NR2> từ -50.0 đến 399.9 (Đơn vị: ºC).

Ví dụ: TEMP:AMB:DAT 25.6 → >25.6

11. SYSTem:LFRequency
Chức năng: Cài đặt tần số lưới điện để lọc nhiễu đường dây.

Cú pháp: SYSTem:LFRequency {AUTO | 50 | 60}

Truy vấn: SYSTem:LFRequency?

Tham số/Trả về: AUTO, 50 (Hz), 60 (Hz).

Ví dụ: SYST:LFR 60

12. SYSTem:PWM:ON (Chỉ GOM-805)
Chức năng: Cài đặt thời gian ON (duty cycle) cho chế độ nguồn dòng PWM.

Cú pháp: SYSTem:PWM:ON <NR1>

Truy vấn: SYSTem:PWM:ON?

Tham số/Trả về: <NR1> từ 3 đến 99. Mỗi đơn vị tương đương 16.6ms (với lưới 60Hz) hoặc 20.0ms (với lưới 50Hz).

Ví dụ: SYST:PWM:ON 5

13. SYSTem:PWM:OFF (Chỉ GOM-805)
Chức năng: Cài đặt thời gian OFF cho chế độ nguồn dòng PWM.

Cú pháp: SYSTem:PWM:OFF <NR1>

Truy vấn: SYSTem:PWM:OFF?

Tham số/Trả về: <NR1> từ 100 đến 9999 (Đơn vị: ms).

Ví dụ: SYST:PWM:OFF 200

Tổng Hợp Chi Tiết: System Commands
1. *IDN?
Chức năng: Truy vấn thông tin nhận dạng thiết bị.

Cú pháp: *IDN?

Trả về: <String> gồm 31 ký tự, bao gồm: Nhà sản xuất, Model, Số serial, Phiên bản firmware.

Ví dụ: *IDN? → >GWINSTEK,GOM805,GXXXXXXXX,V1.00.

2. SYSTem:SERial?
Chức năng: Truy vấn số serial của máy.

Cú pháp: SYSTem:SERial?

Trả về: <String> gồm 9 ký tự.

Ví dụ: SYST:SER? → >GXXXXXXXX

3. SYSTem:BRIGhtness
Chức năng: Cài đặt hoặc truy vấn độ sáng màn hình.

Cú pháp: SYSTem:BRIGhtness <NR1>

Truy vấn: SYSTem:BRIGhtness?

Tham số/Trả về: <NR1> từ 1 (mờ nhất) đến 5 (sáng nhất).

Ví dụ: SYST:BRIG 4

4. USERdefine<X>:ACTive
Chức năng: Cài đặt trạng thái kích hoạt đầu ra cho chân User Define (mức thấp hoặc mức cao).

Cú pháp: USERdefine<X>:ACTive <NR1>

Truy vấn: USERdefine<X>:ACTive?

Tham số:

<X>: Chân số 1 hoặc 2.

<NR1>: 1 (Active Low - tích cực mức thấp) hoặc 2 (Active High - tích cực mức cao).

Ví dụ: USER1:ACT 1 (Đặt chân User1 là Active Low).

5. USERdefine<X>:FIRStdata
Chức năng: Chọn điều kiện (toán hạng) thứ nhất cho chân User Define.

Cú pháp: USERdefine<X>:FIRStdata <NR1>

Truy vấn: USERdefine<X>:FIRStdata?

Tham số:

<X>: 1 hoặc 2.

<NR1> (Chọn một trong các trạng thái):

1-8: Trạng thái của BIN1 đến BIN8.

9: Trạng thái BIN OUT (không đạt).

10: Trạng thái HI (Cao).

11: Trạng thái LOW (Thấp).

12: Trạng thái PASS (Đạt).

13: Trạng thái FAIL (Không đạt).

Ví dụ: USER1:FIRS 12 (Đặt toán hạng 1 là trạng thái PASS).

6. USERdefine<X>:LOGic
Chức năng: Chọn phép toán logic giữa toán hạng thứ nhất và thứ hai cho chân User Define.

Cú pháp: USERdefine<X>:LOGic <NR1>

Truy vấn: USERdefine<X>:LOGic?

Tham số:

<X>: 1 hoặc 2.

<NR1>:

1: OFF (Chỉ dùng toán hạng thứ nhất).

2: AND (Và).

3: OR (Hoặc).

Ví dụ: USER1:LOG 1 (Chỉ dùng toán hạng thứ nhất để quyết định đầu ra).

7. USERdefine<X>:SEConddata
Chức năng: Chọn điều kiện (toán hạng) thứ hai cho chân User Define.

Cú pháp: USERdefine<X>:SECondata <NR1>

Truy vấn: USERdefine<X>:SECondata?

Tham số: <X> (1/2) và <NR1> tương tự như lệnh FIRStdata.

Ví dụ: USER1:SEC 3 (Đặt toán hạng 2 là trạng thái của BIN3).

Lưu ý: Cú pháp gốc có vẻ thiếu chữ 'r' (SECondata thay vì SEConddata), hãy kiểm tra lại khi sử dụng.

8. SYSTem:HANDler
Chức năng: Cài đặt chế độ cho Handler Interface.

Cú pháp: SYSTem:HANDler {CLEAR | HOLD}

Truy vấn: SYSTem:HANDler?

Tham số/Trả về:

CLEAR: Xóa kết quả cũ trước khi thực hiện phép đo mới.

HOLD: Giữ kết quả kiểm tra và chỉ thay đổi khi có kết quả khác.

Ví dụ: SYST:HAND HOLD

9. SYSTem:KEYClick:BEEPer
Chức năng: Bật/tắt tiếng bíp khi nhấn phím.

Cú pháp: SYSTem:KEYClick:BEEPer <NR1> | {OFF | ON}

Truy vấn: SYSTem:KEYClick:BEEPer?

Tham số/Trả về: 0 hoặc OFF (Tắt), 1 hoặc ON (Bật).

Ví dụ: SYST:KEYC:BEEP OFF

10. SYSTem:VOLTage:PROTect
Chức năng: Bật/tắt chức năng bảo vệ quá áp đầu vào (HVP - High Voltage Protection).

Cú pháp: SYSTem:VOLTage:PROTect <NR1> | {OFF | ON}

Truy vấn: SYSTem:VOLTage:PROTect?

Tham số/Trả về: 0 hoặc OFF (Tắt), 1 hoặc ON (Bật).

Ví dụ: SYST:VOLT:PROT OFF

11. SYSTem:ERRor?
Chức năng: Truy vấn lỗi hệ thống hiện tại.

Cú pháp: SYSTem:ERRor?

Trả về: <String> dạng "Mã lỗi","Thông báo lỗi".

Ví dụ: SYST:ERR? → >0,"No error"

Danh sách lỗi cơ bản:

0,"No error"

1,"Command error"

4,"Data out of range"

12. SYSTem:LOCal
Chức năng: Chuyển máy từ chế độ điều khiển từ xa (Remote) về chế độ điều khiển trực tiếp (Local - bằng phím bấm trên máy).

Cú pháp: SYSTem:LOCal

13. SYSTem:VERSion?
Chức năng: Truy vấn phiên bản chuẩn SCPI mà thiết bị hỗ trợ.

Cú pháp: SYSTem:VERSion?

Trả về: <String> (VD: SCPI1994.0).

Ví dụ: SYST:VERS? → >SCPI1994.0

Tổng Hợp Chi Tiết: Memory Commands
1. MEMory:SAVe
Chức năng: Lưu toàn bộ thiết lập hiện tại vào một vị trí nhớ được chọn.

Cú pháp: MEMory:SAVe <NR1>

Tham số: <NR1> từ 1 đến 20 (ô nhớ).

Ví dụ: MEM:SAV 1 (Lưu thiết lập vào ô nhớ số 1).

2. MEMory:RECall
Chức năng: Gọi lại thiết lập đã lưu từ một vị trí nhớ được chọn.

Cú pháp: MEMory:RECall <NR1>

Tham số: <NR1> từ 1 đến 20.

Ví dụ: MEM:REC 1 (Gọi thiết lập từ ô nhớ số 1).

3. MEMory:CLEar
Chức năng: Xóa dữ liệu thiết lập khỏi một vị trí nhớ được chọn.

Cú pháp: MEMory:CLEar <NR1>

Tham số: <NR1> từ 1 đến 20.

Ví dụ: MEM:CLE 1 (Xóa dữ liệu ở ô nhớ số 1).

4. MEMory:STATe?
Chức năng: Truy vấn trạng thái của tất cả 20 ô nhớ.

Cú pháp: MEMory:STATe?

Trả về: <String> gồm 23 ký tự (bao gồm cả dấu gạch ngang), trong đó:

N: Ô nhớ trống (Not used).

F: Ô nhớ đã có dữ liệu (Full).

Định dạng: NFFNN-NNNNN-NNNNN-NNNNN (Nhóm 5-5-5-5, cách nhau bằng dấu -).

Ví dụ: MEM:STAT? → >NFFNN-NNNNN-NNNNN-NNNNN (Ô nhớ 2 và 3 đã có dữ liệu, còn lại trống).

Tổng Hợp Chi Tiết: Status Commands
1. STATus:PRESet
Chức năng: Đặt lại thanh ghi cho phép sự kiện "Questionable" về 0.

Cú pháp: STATus:PRESet

2. STATus:QUEStionable:ENABle
Chức năng: Cài đặt hoặc truy vấn thanh ghi cho phép sự kiện "Questionable Data" (Questionable Data Enable Register). Xác định bit nào trong thanh ghi sự kiện được phép tác động đến bit QUES trong thanh ghi trạng thái.

Cú pháp: STATus:QUEStionable:ENABle <NR1>

Truy vấn: STATus:QUEStionable:ENABle?

Tham số/Trả về: <NR1> từ 0 đến 32767.

Ví dụ: STAT:QUES:ENAB 2560

3. STATus:QUEStionable:EVENt?
Chức năng: Truy vấn nội dung hiện tại của thanh ghi sự kiện "Questionable Data" (Questionable Data Event Register) và xóa nó sau khi đọc.

Cú pháp: STATus:QUEStionable:EVENt?

Trả về: <NR1> từ 0 đến 32767.

Ví dụ: STAT:QUES:EVEN? → >512 (Chỉ ra có lỗi "Ohms Overload" như trong sơ đồ bên dưới).

Tổng Hợp Chi Tiết: IEEE 488.2 Common Commands
1. *CLS
Chức năng: Xóa tất cả các thanh ghi trạng thái sự kiện (Event Status Registers) và hàng đợi đầu ra.

Cú pháp: *CLS

2. *ESE
Chức năng: Cài đặt hoặc truy vấn thanh ghi cho phép sự kiện chuẩn (Standard Event Status Enable Register - ESER).

Cú pháp: *ESE <NR1>

Truy vấn: *ESE?

Tham số/Trả về: <NR1> từ 0 đến 255.

Ví dụ: *ESE 65

3. *ESR?
Chức năng: Truy vấn và xóa thanh ghi sự kiện chuẩn (Standard Event Status Register - SESR).

Cú pháp: *ESR?

Trả về: <NR1> từ 0 đến 255.

Ví dụ: *ESR? → >198

4. *OPC
Chức năng: Đặt bit 0 (Operation Complete) trong thanh ghi SESR khi tất cả các lệnh treo đã hoàn tất.

Cú pháp: *OPC hoặc *OPC? (dạng truy vấn sẽ trả về 1 khi hoàn tất).

Trả về (dạng Query): <NR1> (1 nếu hoàn tất).

Ví dụ: *OPC?

5. *RST
Chức năng: Khôi phục toàn bộ thiết lập của máy về mặc định (Panel Setup Default).

Cú pháp: *RST

6. *SRE
Chức năng: Cài đặt hoặc truy vấn thanh ghi cho phép yêu cầu dịch vụ (Service Request Enable Register - SRER).

Cú pháp: *SRE <NR1>

Truy vấn: *SRE?

Tham số/Trả về: <NR1> từ 0 đến 255.

Ví dụ: *SRE 7

7. *STB?
Chức năng: Truy vấn thanh ghi trạng thái chính (Status Byte Register - SBR).

Cú pháp: *STB?

Trả về: <NR1> từ 0 đến 255.

Ví dụ: *STB? → >81

8. *TRG
Chức năng: Kích khởi thủ công một phép đo (tương đương nhấn nút Trigger trên mặt máy).

Cú pháp: *TRG

Sơ đồ Hệ thống Trạng thái (Status System)
Sơ đồ này minh họa cấu trúc phân cấp của các thanh ghi trạng thái. Đây là "bản đồ" để bạn đọc và xử lý lỗi/lịch trình qua giao tiếp từ xa.

Questionable Data Event Register (16 bit):

Bit 0: (Không dùng)

Bit 1: Voltage Overload (Quá tải điện áp)

Bit 2 đến Bit 7: (Không dùng)

Bit 8: Temp Overload (Quá tải nhiệt độ)

Bit 9: Ohms Overload (Quá tải điện trở)

Bit 10: (Không dùng)

Bit 11: Limit Test Fail LO (So sánh thất bại - Dưới ngưỡng)

Bit 12: Limit Test Fail HI (So sánh thất bại - Trên ngưỡng)

Bit 13 đến Bit 15: (Không dùng)

Status Byte Register (8 bit):

Bit 0, 1: (Không dùng)

Bit 2: QUES (Tóm tắt từ Questionable Data)

Bit 3: (Không dùng)

Bit 4: MAV (Message Available - Có dữ liệu trả về)

Bit 5: ESB (Tóm tắt từ Standard Event)

Bit 6: MSS (Master Summary Status - Tóm tắt yêu cầu dịch vụ)

Bit 7: (Không dùng)


