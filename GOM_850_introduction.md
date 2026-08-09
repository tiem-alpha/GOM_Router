# GOM-804 / GOM-805 - Hướng dẫn sử dụng tổng hợp

> Tài liệu này được tổng hợp từ User Manual GOM-804/GOM-805.

## 1. Giới thiệu
GOM-804/GOM-805 là máy đo điện trở DC Milli-Ohm độ chính xác cao sử dụng phương pháp đo Kelvin 4 dây để đo điện trở rất nhỏ.

## 2. Đặc điểm chính
- Dải đo: 5 mΩ đến 5 MΩ
- Độ phân giải tới 0.1 µΩ
- Độ chính xác tới 0.05%
- Auto/Manual Range
- 10 hoặc 60 mẫu/giây
- Compare
- Scan
- Temperature Measurement
- Temperature Compensation
- Temperature Conversion
- Save/Recall 20 bộ cấu hình
- USB, RS232, Handler, Scan (GPIB tùy model)

## 3. Khác nhau giữa GOM-804 và GOM-805
- GOM-805 bổ sung:
  - Dry Circuit Test
  - Drive Signal (DC+/DC-/Pulse/PWM/Zero)
  - Binning

## 4. Kết nối
- Luôn dùng kết nối Kelvin 4 dây:
  - Source+
  - Source-
  - Sense+
  - Sense-
- Guard nối lớp shield để giảm nhiễu.

## 5. Quy trình đo cơ bản
1. Bật nguồn.
2. Chọn Ohm.
3. Zero (REL).
4. Chọn Range hoặc Auto.
5. Kẹp Kelvin vào DUT.
6. Đọc kết quả.

## 6. Các chức năng
- Resistance
- Compare
- Binning (805)
- Temperature
- Temperature Compensation
- Temperature Conversion
- Scan
- Trigger
- Diode

## 7. Drive Signal (805)
- DC+
- DC-
- Pulse (khử Thermo EMF)
- PWM
- Zero (±10mV voltmeter)
- Standby

## 8. Dry Circuit
Điện áp hở mạch <20mV để đo tiếp điểm theo DIN IEC512 / ASTM B539.

## 9. Measurement Settings
- Average
- Measure Delay
- Trigger Delay
- Trigger Edge
- Temperature Unit
- Ambient Temperature
- Line Frequency
- PWM Duty

## 10. Remote Interface
USB
RS232
GPIB (tùy model)

SCPI hỗ trợ đầy đủ.

## 11. Handler / Scan
Hỗ trợ PASS/FAIL/HI/LO/READY/EOT và Trigger ngoài.

## 12. Save/Recall
20 bộ nhớ.

## 13. Ứng dụng
- Contact resistance
- Relay
- PCB
- Connector
- Motor winding
- Fuse
- QA
