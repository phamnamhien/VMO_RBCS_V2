# Mạch đọc Pin BSS V2

**Tên sản phẩm:** VMO RBCS Battery Station Slave V2
**Phiên bản firmware:** 2.0
**Ngày cập nhật:** 10/02/2026

---

## 1. Hoạt động của thiết bị

### 1.1. Hoạt động bình thường

Khi có Pin được đặt vào slot:

1. Limit Switch kích hoạt → thiết bị nhận biết **có Pin trong slot**
2. Thiết bị **tự động nhận dữ liệu** từ Pack Pin qua CAN Bus
3. Dữ liệu Pin được cập nhật liên tục vào bộ nhớ thanh ghi
4. **Master Controller** đọc dữ liệu bất kỳ lúc nào qua Modbus RTU (Function Code 0x03)
5. Master Controller có thể gửi lệnh điều khiển:

| Lệnh | Thanh ghi | Giá trị | Tác dụng |
|-------|-----------|---------|----------|
| Bật sạc | 75 (CHRG_CTRL) | 1 | Kích hoạt ngõ ra CHARGE_CTRL |
| Tắt sạc | 75 (CHRG_CTRL) | 0 | Ngắt ngõ ra CHARGE_CTRL |
| Dừng khẩn cấp | 74 (EMERGENCY_STOP) | 1 | Kích hoạt ngõ ra EMERGENCY, ngắt sạc |
| Hủy dừng khẩn cấp | 74 (EMERGENCY_STOP) | 0 | Tắt tín hiệu EMERGENCY |

> **Lưu ý:** Khi lệnh dừng khẩn cấp đang hoạt động, lệnh bật sạc sẽ bị bỏ qua.

### 1.2. Khi Pin được tháo ra

Khi cả hai Limit Switch đều không kích hoạt:
- Thiết bị nhận biết **slot trống**
- Tất cả thanh ghi dữ liệu Pin được xóa về 0
- Ngõ ra CHARGE_CTRL và EMERGENCY đều tắt
- Thiết bị quay về trạng thái chờ Pin

### 1.3. Các trạng thái hoạt động

```
                ┌────────────────────────┐
                │   Cài đặt Baudrate     │ ← DIP Switch = 0
                │   (LED_RUN nhấp nháy)  │
                └────────────────────────┘
                            │
                     DIP Switch ≠ 0
                            ▼
                ┌────────────────────────┐
        ┌──────►│   Hoạt động bình       │◄──────┐
        │       │   thường               │       │
        │       │   (LED_RUN sáng)       │       │
        │       └───────┬────────┬───────┘       │
        │               │        │               │
        │          BMS lỗi   Mất CAN ≥5s         │
        │               ▼        ▼               │
        │       ┌──────────┐ ┌──────────────┐    │
        │       │ Lỗi Pin  │ │  Mất kết nối │    │
        ├───────│(LED_FAULT│ │  (LED_FAULT  │────┤
        │       │ sáng)    │ │  nhấp nháy)  │    │
        │       └────┬─────┘ └───────┬──────┘    │
        │            │               │           │
        │            └───BMS lỗi─────┘           │
        │                                        │
        └── Tháo Pin / Hết lỗi ──────────────────┘
```

| Trạng thái | LED_RUN (Xanh) | LED_FAULT (Đỏ) | LED_STT (Vàng) | Điều kiện vào | Điều kiện thoát |
|------------|----------------|-----------------|-----------------|---------------|-----------------|
| **Cài đặt baudrate** | Nhấp nháy (500ms) | Tắt | Tắt | DIP Switch = 0 | Lưu baudrate xong |
| **Hoạt động bình thường** | Sáng liên tục | Tắt | Nhấp nháy khi nhận lệnh | Khởi động với địa chỉ hợp lệ | BMS lỗi hoặc mất CAN |
| **Lỗi Pin** | Tắt | Sáng liên tục | Tắt | Nhận dữ liệu có cờ FAULT | Tháo Pin hoặc hết lỗi |
| **Mất kết nối** | Tắt | Nhấp nháy (500ms) | Tắt | Không nhận CAN sau 5s | Tháo Pin hoặc nhận lại CAN |

---

## 2. Bảng thanh ghi (Register Map)

Tổng cộng **76 thanh ghi 16-bit** (địa chỉ 0-75). Dữ liệu từ 18 CAN message được lưu dạng raw 16-bit big-endian (4 thanh ghi / frame).
Một số thanh ghi chứa nhiều signal ghép (ký hiệu `[High byte] [Low byte]`). Công thức chuyển đổi: `Giá trị thực = Raw × Factor`.

### 0x300 — PACK_ControlSystem (địa chỉ 0-3)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 0 | CTRL_SYS_MAX_CHG_CURRENT | Dòng sạc tối đa cho phép (int16, ×0.02) | -655 ~ 655 A |
| 1 | CTRL_SYS_MAX_DSG_CURRENT | Dòng xả tối đa cho phép (int16, ×0.02) | -655 ~ 655 A |
| 2 | CTRL_SYS_MAX_CHG_POWER | Công suất sạc tối đa (int16, ×1) | -32767 ~ 32767 W |
| 3 | CTRL_SYS_MAX_DSG_POWER | Công suất xả tối đa (int16, ×1) | -32767 ~ 32767 W |

### 0x301 — PACK_InfoCharging (địa chỉ 4-7)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 4 | CHARGING_VOLTAGE_LIMIT | Giới hạn điện áp sạc (uint16, ×0.002) | 0 ~ 131.07 V |
| 5 | CHARGING_CURRENT_LIMIT | Giới hạn dòng sạc (int16, ×0.02). Bằng 0 nếu có lỗi | -655 ~ 655 A |
| 6 | CHARGING_STATUS | [FullyChargedSts(4b) &#124; ControlMode(4b)] [EndCurrentCharge(8b, ×0.01)] | FullyCharged: 0=N/A, 1=Đang sạc, 2=Đầy. Mode: 0=OFF, 1=Normal, 2=Trickle. EndCurrent: 0~2.55A |
| 7 | CHARGING_DIAG | [TrickleCurrentCharger(8b, ×0.01)] [DiagCharger(8b)] | Trickle: 0~2.55A. Diag: 0~255 |

### 0x303 — PACK_InfoBms (địa chỉ 8-11)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 8 | BMS_STATUS_1 | [IgnitionRecognition(4b) &#124; RollingCounter(4b)] [BmsUserMode(4b) &#124; Emergency(4b)] | Ignition: 0=Low, 1=High. Rolling: 0~15. Mode: 0=Standby, 1=Working. Emergency: 0~15 |
| 9 | BMS_CURRENT_DIR | [CurrentDirection(8b)] [DiagCellTopPriorResult(8b)] | Direction: 0=Rest, 1=Charging, 2=Discharging. DiagTop: 0~255 |
| 10 | BMS_RESERVED | Dự phòng | — |
| 11 | BMS_WAKEUP | [Reserved(8b)] [WakeupSource(8b)] | Wakeup: 0=RTC, 1=BMS SW |

### 0x304 — PACK_InfoCellBalancing (địa chỉ 12-15)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 12 | CB_STATUS_1_4 | [CB02(4b) &#124; CB01(4b)] [CB04(4b) &#124; CB03(4b)] | Mỗi CB: 0=Off, 1=On |
| 13 | CB_STATUS_5_8 | [CB06(4b) &#124; CB05(4b)] [CB08(4b) &#124; CB07(4b)] | Mỗi CB: 0=Off, 1=On |
| 14 | CB_STATUS_9_12 | [CB10(4b) &#124; CB09(4b)] [CB12(4b) &#124; CB11(4b)] | Mỗi CB: 0=Off, 1=On |
| 15 | CB_STATUS_13 | [CB13(4b) &#124; Reserved(4b)] [Reserved(8b)] | CB13: 0=Off, 1=On |

### 0x306 — PACK_InfoDemCell (địa chỉ 16-19) — Chẩn đoán cell

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 16 | DEM_CELL_VOLTAGE | [OverVoltageCell(4b) &#124; OverVoltagePack(4b)] [UnderVoltageCell(4b) &#124; UnderVoltagePack(4b)] | 0=Normal, 1=Warn1, 2=Warn2, 3=Fault, 4=Failure |
| 17 | DEM_CELL_CURRENT | [OverCurrentChg(4b) &#124; OverCurrentDis(4b)] [OverTempChg(4b) &#124; OverTempDis(4b)] | 0=Normal, 1=Warn1, 2=Warn2, 3=Fault, 4=Failure |
| 18 | DEM_CELL_TEMP | [UnderTempChg(4b) &#124; UnderTempDis(4b)] [ImbCellChg(4b) &#124; ImbCellRest(4b)] | 0=Normal, 1=Warn1, 2=Warn2, 3=Fault, 4=Failure |
| 19 | DEM_CELL_IMB | [ImbCellTemp(4b) &#124; ImbCellSOC(4b)] [Reserved(8b)] | 0=Normal, 1=Warn1, 2=Warn2, 3=Fault, 4=Failure |

### 0x309 — PACK_InfoPack (địa chỉ 20-23)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 20 | PACK_FET_STATUS | [CFET(4b) &#124; DFET(4b)] [LowCapAlarm(4b) &#124; PrechargeFET(4b)] | FET: 0=Off, 1=On. Alarm: 0=Clear, 1=Set |
| 21 | PACK_VOLTAGE_BAT | Điện áp pack đo từ cell (uint16, ×0.002) | 0 ~ 131.07 V |
| 22 | PACK_CURRENT_AVG | Dòng điện pack trung bình (int16, ×0.02). Dương=sạc, Âm=xả | -655.36 ~ 655.34 A |
| 23 | PACK_CURRENT | Dòng điện pack tức thời (int16, ×0.02). Dương=sạc, Âm=xả | -655.36 ~ 655.34 A |

### 0x30A — PACK_InfoSox (địa chỉ 24-27)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 24 | SOX_CAPACITY | [FullChargeCapacity(8b, ×0.4 Ah)] [RemainingCapacity(8b, ×0.4 Ah)] | FCC: 0~102 Ah. Remain: 0~102 Ah |
| 25 | SOX_RSOC_SOH | [RSOC(8b)] [SOH(8b)] | RSOC: 0~255%. SOH: 0~255% |
| 26 | SOX_CYCLE_COUNT | Số chu kỳ sạc/xả (uint16) | 0 ~ 65535 |
| 27 | SOX_REMAINING_CHG_TIME | Thời gian sạc còn lại (uint16) | 0 ~ 65535 phút |

### 0x30B — PACK_AccumDsgChgCapacity (địa chỉ 28-31)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 28 | ACCUM_CHARGE_HIGH | Dung lượng sạc tích lũy — 16 bit cao (uint32 ghép với reg 29) | 0 ~ 4294967295 Ah |
| 29 | ACCUM_CHARGE_LOW | Dung lượng sạc tích lũy — 16 bit thấp | (ghép với reg 28) |
| 30 | ACCUM_DISCHARGE_HIGH | Dung lượng xả tích lũy — 16 bit cao (uint32 ghép với reg 31) | 0 ~ 4294967295 Ah |
| 31 | ACCUM_DISCHARGE_LOW | Dung lượng xả tích lũy — 16 bit thấp | (ghép với reg 30) |

### 0x30E — PACK_InfoContactor (địa chỉ 32-35)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 32 | ADC_BATT_VOLT | Điện áp battery đo bằng ADC (uint16, ×0.002) | 0 ~ 131.07 V |
| 33 | ADC_PACK_VOLT | Điện áp pack đo bằng ADC (uint16, ×0.002) | 0 ~ 131.07 V |
| 34 | ADC_FUSE_VOLT | Điện áp sau cầu chì đo bằng ADC (uint16, ×0.002) | 0 ~ 131.07 V |
| 35 | CHARGE_TIME | Thời gian đã sạc (uint16) | 0 ~ 65535 |

### 0x310 — PACK_InfoVoltageCell (địa chỉ 36-39)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 36 | VCELL_AVG | Điện áp cell trung bình (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 37 | VCELL_MIN | Điện áp cell thấp nhất (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 38 | VCELL_MIN_NO | [VCellMinNo(8b)] [VCellMax high byte] — Số thứ tự cell min + byte cao VCellMax | MinNo: 0~255 |
| 39 | VCELL_MAX | [VCellMax low byte] [VCellMaxNo(8b)] — Byte thấp VCellMax + số thứ tự cell max | MaxNo: 0~255. VCellMax(ghép 38-39): 0~6.5535V |

### 0x311 — PACK_InfoVoltageCell1 (địa chỉ 40-43)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 40 | VCELL_01 | Điện áp Cell 1 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 41 | VCELL_02 | Điện áp Cell 2 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 42 | VCELL_03 | Điện áp Cell 3 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 43 | VCELL_04 | Điện áp Cell 4 (uint16, ×0.0001) | 0 ~ 6.5535 V |

### 0x312 — PACK_InfoVoltageCell2 (địa chỉ 44-47)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 44 | VCELL_05 | Điện áp Cell 5 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 45 | VCELL_06 | Điện áp Cell 6 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 46 | VCELL_07 | Điện áp Cell 7 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 47 | VCELL_08 | Điện áp Cell 8 (uint16, ×0.0001) | 0 ~ 6.5535 V |

### 0x313 — PACK_InfoVoltageCell3 (địa chỉ 48-51)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 48 | VCELL_09 | Điện áp Cell 9 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 49 | VCELL_10 | Điện áp Cell 10 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 50 | VCELL_11 | Điện áp Cell 11 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 51 | VCELL_12 | Điện áp Cell 12 (uint16, ×0.0001) | 0 ~ 6.5535 V |

### 0x314 — PACK_InfoVoltageCell4 (địa chỉ 52-55, DLC=2)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 52 | VCELL_13 | Điện áp Cell 13 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 53 | VCELL4_RSVD_1 | Dự phòng (DLC=2, không có dữ liệu) | — |
| 54 | VCELL4_RSVD_2 | Dự phòng | — |
| 55 | VCELL4_RSVD_3 | Dự phòng | — |

### 0x315 — PACK_InfoDemBMS (địa chỉ 56-59, DLC=4) — Chẩn đoán BMS

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 56 | DEM_BMS_0 | [demCFET(4b) &#124; demDFET(4b)] [demPFET(4b) &#124; demASICComm(4b)] | 0=Normal, 3=Fault |
| 57 | DEM_BMS_1 | [demASICShutdown(4b) &#124; demShortCurrent(4b)] [demFETTemp(4b) &#124; demFuseBlow(4b)] | 0=Normal, 3=Fault |
| 58 | DEM_BMS_RSVD_0 | Dự phòng (DLC=4, không có dữ liệu) | — |
| 59 | DEM_BMS_RSVD_1 | Dự phòng | — |

### 0x320 — PACK_InfoTemperatureCell (địa chỉ 60-63)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 60 | TEMP_CELL_AVG_MIN | [TempCellAvg(8b signed)] [TempCellMin(8b signed)] | -127 ~ 127 °C mỗi trường |
| 61 | TEMP_CELL_MIN_NO | [TempCellMinNo(8b)] [TempCellMax(8b signed)] | MinNo: 0~255. Max: -127~127 °C |
| 62 | TEMP_CELL_MAX_NO | [TempCellMaxNo(8b)] [Reserved(8b)] | MaxNo: 0~255 |
| 63 | TEMP_CELL_RSVD | Dự phòng | — |

### 0x322 — PACK_InfoTemperatureCB (địa chỉ 64-67)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 64 | TEMP_CB1_CB2 | [TempCB1(8b signed)] [TempCB2(8b signed)] — Nhiệt độ mạch cân bằng 1, 2 | -127 ~ 127 °C mỗi trường |
| 65 | TEMP_FET | [TempFET(8b signed)] [Reserved(8b)] — Nhiệt độ FET | -127 ~ 127 °C |
| 66 | TEMP_CB_RSVD_0 | Dự phòng | — |
| 67 | TEMP_CB_RSVD_1 | Dự phòng | — |

### 0x32F — PACK_InfoPackVersion (địa chỉ 68-71)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 68 | VERSION_SW | [SWMinor(4b) &#124; SWSubminor(4b)] [BLMajor(4b) &#124; SWMajor(4b)] | Mỗi trường: 0~15 |
| 69 | VERSION_BL | [BLMinor(4b) &#124; BLSubminor(4b)] [HWMajor(4b) &#124; Reserved(4b)] | Mỗi trường: 0~15 |
| 70 | VERSION_MFG_DATE | Ngày sản xuất (uint16) | 0 ~ 65535 |
| 71 | VERSION_OTP | Mã OTP BQ (uint16) | 0 ~ 65535 |

### Thanh ghi trạng thái Station (chỉ đọc, địa chỉ 72-73)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 72 | IS_PIN_IN_SLOT | Trạng thái slot — phát hiện bởi Limit Switch | 0 = Trống, 1 = Có Pin |
| 73 | IS_PIN_TIMEOUT | Trạng thái kết nối CAN với Pack Pin | 0 = Bình thường, 1 = Mất kết nối |

### Thanh ghi điều khiển (đọc/ghi, địa chỉ 74-75)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 74 | IS_EMERGENCY_STOP | Lệnh dừng khẩn cấp — Master ghi để kích hoạt/hủy | 0 = Bình thường, 1 = Dừng khẩn cấp |
| 75 | CHRG_CTRL | Lệnh điều khiển sạc — Master ghi để bật/tắt relay sạc | 0 = Tắt sạc, 1 = Bật sạc |
