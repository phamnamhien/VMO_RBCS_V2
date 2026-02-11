# HƯỚNG DẪN SỬ DỤNG - Mạch đọc Pin BSS V2

**Tên sản phẩm:** VMO RBCS Battery Station Slave V2
**Phiên bản firmware:** 2.0
**Ngày cập nhật:** 10/02/2026

---

## 1. Giới thiệu

### 1.1. Mô tả sản phẩm

Mạch BSS V2 là bộ đọc và điều khiển Pin dùng trong trạm sạc/thay pin tự động. Mạch thực hiện hai chức năng chính:

- **Đọc dữ liệu Pack Pin** qua giao tiếp CAN Bus (1 Mbps) — tự động nhận toàn bộ thông tin từ BMS (trạng thái, điện áp, dòng điện, nhiệt độ, điện áp từng cell, SOC, SOH, v.v.)
- **Cung cấp dữ liệu cho Master Controller** qua giao tiếp RS485 Modbus RTU — Master đọc dữ liệu Pin và gửi lệnh điều khiển (bật sạc, dừng khẩn cấp)

### 1.2. Sơ đồ hệ thống

```
                                CAN Bus (1 Mbps)
    +-------------+         CANH ──────────── CANH          +-------------+
    |             | ◄────── CANL ──────────── CANL ──────►  |             |
    |  Mạch BSS   |                                         |  Pack Pin   |
    |  (Slave)    |                                         |  (BMS)      |
    |             |         RS485 Modbus RTU                +-------------+
    |             |          A ────────────── A
    |             | ◄──────► B ────────────── B ──────────► +-------------+
    +-------------+                                         |  Master     |
                                                            |  Controller |
                                                            +-------------+
```

### 1.3. Thông số kỹ thuật

| Thông số | Giá trị |
|----------|---------|
| Nguồn cấp | Jack nguồn DC riêng (onboard) |
| Giao tiếp Pin | CAN Bus 2.0A, 1 Mbps |
| Giao tiếp Master | RS485 Half-duplex, Modbus RTU |
| Baudrate RS485 | Cấu hình được (1200 - 115200 bps) |
| Địa chỉ Modbus | Cấu hình được (1 - 31) |
| Số thanh ghi Modbus | 76 thanh ghi 16-bit |
| Cảm biến slot | 2x Limit Switch (phát hiện Pin) |
| Ngõ ra điều khiển | 1x Điều khiển sạc, 1x Dừng khẩn cấp |
| Chỉ thị | 3x LED (RUN, FAULT, STATUS) |

---

## 2. Mô tả phần cứng

### 2.1. Các cổng kết nối

#### Cổng CAN Bus (kết nối Pack Pin)

| Tín hiệu | Mô tả |
|-----------|-------|
| CANH | CAN High |
| CANL | CAN Low |

#### Cổng RS485 (kết nối Master Controller)

| Tín hiệu | Mô tả |
|-----------|-------|
| A | RS485 Data+ |
| B | RS485 Data- |

#### Cổng Limit Switch (phát hiện Pin trong slot)

| Đầu vào | Mô tả |
|----------|-------|
| LIMIT_SWITCH0 | Công tắc hành trình 1 — tích cực mức thấp (nối GND khi Pin có mặt) |
| LIMIT_SWITCH1 | Công tắc hành trình 2 — tích cực mức thấp (nối GND khi Pin có mặt) |

Chỉ cần **một trong hai** công tắc kích hoạt là thiết bị nhận biết có Pin trong slot.

#### Ngõ ra điều khiển

| Ngõ ra | Mô tả |
|--------|-------|
| CHARGE_CTRL | Điều khiển relay sạc. HIGH = bật sạc, LOW = tắt sạc |
| EMERGENCY | Tín hiệu dừng khẩn cấp. HIGH = kích hoạt dừng khẩn, LOW = bình thường |

### 2.2. DIP Switch cấu hình (5 bit)

DIP Switch 5 bit dùng để cài đặt **địa chỉ Modbus** và **baudrate RS485**.

| Công tắc | Nhãn | Ghi chú |
|-----------|------|---------|
| SW1 | ADDR4 | Bit 0 (LSB) |
| SW2 | ADDR3 | Bit 1 |
| SW3 | ADDR2 | Bit 2 |
| SW4 | ADDR1 | Bit 3 |
| SW5 | ADDR0 | Bit 4 (MSB) |

**Logic đảo:** Công tắc gạt sang ON (nối GND) = bit 1. Công tắc ở OFF = bit 0.

### 2.3. Đèn LED chỉ thị

| LED | Màu | Chức năng |
|-----|------|-----------|
| LED_RUN (PB6) | Xanh | Báo thiết bị đang hoạt động bình thường |
| LED_FAULT (PB7) | Đỏ | Báo lỗi Pin hoặc mất kết nối CAN |
| LED_STT (PB5) | Vàng | Nhấp nháy khi nhận được lệnh từ Master |

---

## 3. Lắp đặt

### 3.1. Bước 1 — Kết nối nguồn

Cắm nguồn DC vào jack nguồn riêng trên board. Board có mạch ổn áp onboard.

### 3.2. Bước 2 — Kết nối CAN Bus với Pack Pin

1. Nối **CANH** của mạch BSS với **CANH** của Pack Pin (BMS)
2. Nối **CANL** của mạch BSS với **CANL** của Pack Pin (BMS)

```
Mạch BSS                              Pack Pin (BMS)
  CANH ──────────────── dây ──────────────── CANH
  CANL ──────────────── dây ──────────────── CANL
```

### 3.3. Bước 3 — Kết nối RS485 với Master Controller

1. Nối **A** của mạch BSS với **A** của Master Controller
2. Nối **B** của mạch BSS với **B** của Master Controller

### 3.4. Bước 4 — Kết nối Limit Switch

Nối Limit Switch giữa chân LIMIT_SWITCH0 (hoặc LIMIT_SWITCH1) và GND. Khi Pin được đặt vào slot, công tắc đóng lại (nối GND), thiết bị nhận biết có Pin.

### 3.5. Bước 5 — Kết nối ngõ ra điều khiển

- **CHARGE_CTRL** → Relay/MOSFET điều khiển mạch sạc
- **EMERGENCY** → Mạch dừng khẩn cấp hệ thống

---

## 4. Cấu hình thiết bị

### 4.1. Cài đặt địa chỉ Modbus

Mỗi mạch BSS trên bus RS485 cần có một **địa chỉ duy nhất** (1 - 31). Dùng DIP Switch 5 bit để đặt địa chỉ.

**Bảng tra địa chỉ (một số giá trị thường dùng):**

| Địa chỉ | SW1 | SW2 | SW3 | SW4 | SW5 |
|----------|-----|-----|-----|-----|-----|
| 1 | ON | OFF | OFF | OFF | OFF |
| 2 | OFF | ON | OFF | OFF | OFF |
| 3 | ON | ON | OFF | OFF | OFF |
| 4 | OFF | OFF | ON | OFF | OFF |
| 5 | ON | OFF | ON | OFF | OFF |
| 10 | OFF | ON | OFF | ON | OFF |
| 15 | ON | ON | ON | ON | OFF |
| 16 | OFF | OFF | OFF | OFF | ON |
| 20 | OFF | OFF | ON | OFF | ON |
| 31 | ON | ON | ON | ON | ON |

> **Lưu ý:** Nếu tất cả công tắc ở OFF (địa chỉ = 0), thiết bị sẽ vào **chế độ cài đặt baudrate** thay vì hoạt động bình thường.

### 4.2. Cài đặt Baudrate RS485

Baudrate RS485 được lưu trong bộ nhớ EEPROM (không mất khi tắt nguồn). Mặc định khi xuất xưởng là **115200 bps**.

**Quy trình cài đặt:**

1. **Tắt nguồn** thiết bị
2. **Gạt tất cả DIP Switch về OFF** (địa chỉ = 0)
3. **Bật nguồn** — LED_RUN sẽ nhấp nháy mỗi 500ms, báo hiệu đang ở chế độ cài đặt
4. **Gạt DIP Switch** đến mã baudrate mong muốn theo bảng dưới
5. **Chờ 5 giây** — LED_RUN sáng liên tục, nghĩa là đã lưu thành công
6. **Tắt nguồn**
7. **Gạt DIP Switch về địa chỉ Modbus thực tế** (1 - 31)
8. **Bật nguồn** — thiết bị hoạt động với baudrate mới

**Bảng mã baudrate:**

| Mã (DIP Switch) | Baudrate | Mã (DIP Switch) | Baudrate |
|------------------|----------|------------------|----------|
| 1 | 1200 bps | 6 | 19200 bps |
| 2 | 2400 bps | 7 | 38400 bps |
| 3 | 4800 bps | 8 | 56000 bps |
| 4 | 9600 bps | 9 | 57600 bps |
| 5 | 14400 bps | **10** | **115200 bps (mặc định)** |

---

## 5. Vận hành

### 5.1. Khởi động

1. Đảm bảo DIP Switch đã đặt đúng địa chỉ Modbus (1 - 31)
2. Bật nguồn thiết bị
3. **LED_RUN sáng liên tục** — thiết bị sẵn sàng hoạt động

### 5.2. Hoạt động bình thường

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

### 5.3. Khi Pin được tháo ra

Khi cả hai Limit Switch đều không kích hoạt:
- Thiết bị nhận biết **slot trống**
- Tất cả thanh ghi dữ liệu Pin được xóa về 0
- Ngõ ra CHARGE_CTRL và EMERGENCY đều tắt
- Thiết bị quay về trạng thái chờ Pin

---

## 6. Chỉ thị LED

### 6.1. Bảng trạng thái LED

| Trạng thái thiết bị | LED_RUN (Xanh) | LED_FAULT (Đỏ) | LED_STT (Vàng) |
|----------------------|----------------|-----------------|-----------------|
| Chế độ cài đặt baudrate | Nhấp nháy (500ms) | Tắt | Tắt |
| Hoạt động bình thường | **Sáng liên tục** | Tắt | Nhấp nháy khi nhận lệnh Master |
| Lỗi Pin (BMS fault) | Tắt | **Sáng liên tục** | Tắt |
| Mất kết nối CAN | Tắt | **Nhấp nháy (500ms)** | Tắt |

### 6.2. Ý nghĩa chi tiết

- **LED_RUN sáng liên tục:** Thiết bị đang hoạt động bình thường, sẵn sàng đọc Pin và phục vụ Master.

- **LED_RUN nhấp nháy:** Thiết bị đang ở chế độ cài đặt baudrate (DIP Switch = 0). Xem mục 4.2.

- **LED_FAULT sáng liên tục:** Pack Pin đang báo lỗi (ví dụ: quá áp, quá dòng, quá nhiệt). Kiểm tra trạng thái Pin. Khi lỗi được khắc phục, LED tự tắt.

- **LED_FAULT nhấp nháy:** Thiết bị không nhận được dữ liệu CAN từ Pack Pin sau 5 lần thử (mỗi lần 1 giây). Kiểm tra kết nối CAN Bus.

- **LED_STT nhấp nháy ngắn:** Master Controller vừa đọc/ghi dữ liệu thành công.

---

## 7. Giao tiếp Modbus RTU

### 7.1. Thông số giao tiếp

| Thông số | Giá trị |
|----------|---------|
| Giao thức | Modbus RTU |
| Giao diện vật lý | RS485 Half-duplex |
| Baudrate | Cấu hình được (mặc định 115200 bps) |
| Data bits | 8 |
| Parity | None |
| Stop bits | 1 |
| Địa chỉ Slave | Cấu hình được (1 - 31) |

### 7.2. Function Code hỗ trợ

| Function Code | Tên | Mô tả |
|---------------|-----|-------|
| 0x03 | Read Holding Registers | Đọc thanh ghi dữ liệu |
| 0x06 | Write Single Register | Ghi một thanh ghi điều khiển |
| 0x10 | Write Multiple Registers | Ghi nhiều thanh ghi điều khiển |

### 7.3. Bảng thanh ghi (Register Map)

Dữ liệu từ 18 CAN message (VMO_SinglePack_DBC_V0_1) được lưu dạng raw 16-bit big-endian (4 thanh ghi / frame).
Một số thanh ghi chứa nhiều signal ghép (ký hiệu `[High byte] [Low byte]`). Công thức chuyển đổi: `Giá trị thực = Raw × Factor`.

#### 0x300 — PACK_ControlSystem (địa chỉ 0-3)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 0 | CTRL_SYS_MAX_CHG_CURRENT | Dòng sạc tối đa cho phép (int16, ×0.02) | -655 ~ 655 A |
| 1 | CTRL_SYS_MAX_DSG_CURRENT | Dòng xả tối đa cho phép (int16, ×0.02) | -655 ~ 655 A |
| 2 | CTRL_SYS_MAX_CHG_POWER | Công suất sạc tối đa (int16, ×1) | -32767 ~ 32767 W |
| 3 | CTRL_SYS_MAX_DSG_POWER | Công suất xả tối đa (int16, ×1) | -32767 ~ 32767 W |

#### 0x301 — PACK_InfoCharging (địa chỉ 4-7)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 4 | CHARGING_VOLTAGE_LIMIT | Giới hạn điện áp sạc (uint16, ×0.002) | 0 ~ 131.07 V |
| 5 | CHARGING_CURRENT_LIMIT | Giới hạn dòng sạc (int16, ×0.02). Bằng 0 nếu có lỗi | -655 ~ 655 A |
| 6 | CHARGING_STATUS | [FullyChargedSts(4b) &#124; ControlMode(4b)] [EndCurrentCharge(8b, ×0.01)] | FullyCharged: 0=N/A, 1=Đang sạc, 2=Đầy. Mode: 0=OFF, 1=Normal, 2=Trickle. EndCurrent: 0~2.55A |
| 7 | CHARGING_DIAG | [TrickleCurrentCharger(8b, ×0.01)] [DiagCharger(8b)] | Trickle: 0~2.55A. Diag: 0~255 |

#### 0x303 — PACK_InfoBms (địa chỉ 8-11)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 8 | BMS_STATUS_1 | [IgnitionRecognition(4b) &#124; RollingCounter(4b)] [BmsUserMode(4b) &#124; Emergency(4b)] | Ignition: 0=Low, 1=High. Rolling: 0~15. Mode: 0=Standby, 1=Working. Emergency: 0~15 |
| 9 | BMS_CURRENT_DIR | [CurrentDirection(8b)] [DiagCellTopPriorResult(8b)] | Direction: 0=Rest, 1=Charging, 2=Discharging. DiagTop: 0~255 |
| 10 | BMS_RESERVED | Dự phòng | — |
| 11 | BMS_WAKEUP | [Reserved(8b)] [WakeupSource(8b)] | Wakeup: 0=RTC, 1=BMS SW |

#### 0x304 — PACK_InfoCellBalancing (địa chỉ 12-15)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 12 | CB_STATUS_1_4 | [CB02(4b) &#124; CB01(4b)] [CB04(4b) &#124; CB03(4b)] | Mỗi CB: 0=Off, 1=On |
| 13 | CB_STATUS_5_8 | [CB06(4b) &#124; CB05(4b)] [CB08(4b) &#124; CB07(4b)] | Mỗi CB: 0=Off, 1=On |
| 14 | CB_STATUS_9_12 | [CB10(4b) &#124; CB09(4b)] [CB12(4b) &#124; CB11(4b)] | Mỗi CB: 0=Off, 1=On |
| 15 | CB_STATUS_13 | [CB13(4b) &#124; Reserved(4b)] [Reserved(8b)] | CB13: 0=Off, 1=On |

#### 0x306 — PACK_InfoDemCell (địa chỉ 16-19) — Chẩn đoán cell

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 16 | DEM_CELL_VOLTAGE | [OverVoltageCell(4b) &#124; OverVoltagePack(4b)] [UnderVoltageCell(4b) &#124; UnderVoltagePack(4b)] | 0=Normal, 1=Warn1, 2=Warn2, 3=Fault, 4=Failure |
| 17 | DEM_CELL_CURRENT | [OverCurrentChg(4b) &#124; OverCurrentDis(4b)] [OverTempChg(4b) &#124; OverTempDis(4b)] | 0=Normal, 1=Warn1, 2=Warn2, 3=Fault, 4=Failure |
| 18 | DEM_CELL_TEMP | [UnderTempChg(4b) &#124; UnderTempDis(4b)] [ImbCellChg(4b) &#124; ImbCellRest(4b)] | 0=Normal, 1=Warn1, 2=Warn2, 3=Fault, 4=Failure |
| 19 | DEM_CELL_IMB | [ImbCellTemp(4b) &#124; ImbCellSOC(4b)] [Reserved(8b)] | 0=Normal, 1=Warn1, 2=Warn2, 3=Fault, 4=Failure |

#### 0x309 — PACK_InfoPack (địa chỉ 20-23)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 20 | PACK_FET_STATUS | [CFET(4b) &#124; DFET(4b)] [LowCapAlarm(4b) &#124; PrechargeFET(4b)] | FET: 0=Off, 1=On. Alarm: 0=Clear, 1=Set |
| 21 | PACK_VOLTAGE_BAT | Điện áp pack đo từ cell (uint16, ×0.002) | 0 ~ 131.07 V |
| 22 | PACK_CURRENT_AVG | Dòng điện pack trung bình (int16, ×0.02). Dương=sạc, Âm=xả | -655.36 ~ 655.34 A |
| 23 | PACK_CURRENT | Dòng điện pack tức thời (int16, ×0.02). Dương=sạc, Âm=xả | -655.36 ~ 655.34 A |

#### 0x30A — PACK_InfoSox (địa chỉ 24-27)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 24 | SOX_CAPACITY | [FullChargeCapacity(8b, ×0.4 Ah)] [RemainingCapacity(8b, ×0.4 Ah)] | FCC: 0~102 Ah. Remain: 0~102 Ah |
| 25 | SOX_RSOC_SOH | [RSOC(8b)] [SOH(8b)] | RSOC: 0~255%. SOH: 0~255% |
| 26 | SOX_CYCLE_COUNT | Số chu kỳ sạc/xả (uint16) | 0 ~ 65535 |
| 27 | SOX_REMAINING_CHG_TIME | Thời gian sạc còn lại (uint16) | 0 ~ 65535 phút |

#### 0x30B — PACK_AccumDsgChgCapacity (địa chỉ 28-31)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 28 | ACCUM_CHARGE_HIGH | Dung lượng sạc tích lũy — 16 bit cao (uint32 ghép với reg 29) | 0 ~ 4294967295 Ah |
| 29 | ACCUM_CHARGE_LOW | Dung lượng sạc tích lũy — 16 bit thấp | (ghép với reg 28) |
| 30 | ACCUM_DISCHARGE_HIGH | Dung lượng xả tích lũy — 16 bit cao (uint32 ghép với reg 31) | 0 ~ 4294967295 Ah |
| 31 | ACCUM_DISCHARGE_LOW | Dung lượng xả tích lũy — 16 bit thấp | (ghép với reg 30) |

#### 0x30E — PACK_InfoContactor (địa chỉ 32-35)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 32 | ADC_BATT_VOLT | Điện áp battery đo bằng ADC (uint16, ×0.002) | 0 ~ 131.07 V |
| 33 | ADC_PACK_VOLT | Điện áp pack đo bằng ADC (uint16, ×0.002) | 0 ~ 131.07 V |
| 34 | ADC_FUSE_VOLT | Điện áp sau cầu chì đo bằng ADC (uint16, ×0.002) | 0 ~ 131.07 V |
| 35 | CHARGE_TIME | Thời gian đã sạc (uint16) | 0 ~ 65535 |

#### 0x310 — PACK_InfoVoltageCell (địa chỉ 36-39)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 36 | VCELL_AVG | Điện áp cell trung bình (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 37 | VCELL_MIN | Điện áp cell thấp nhất (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 38 | VCELL_MIN_NO | [VCellMinNo(8b)] [VCellMax high byte] — Số thứ tự cell min + byte cao VCellMax | MinNo: 0~255 |
| 39 | VCELL_MAX | [VCellMax low byte] [VCellMaxNo(8b)] — Byte thấp VCellMax + số thứ tự cell max | MaxNo: 0~255. VCellMax(ghép 38-39): 0~6.5535V |

#### 0x311 — PACK_InfoVoltageCell1 (địa chỉ 40-43)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 40 | VCELL_01 | Điện áp Cell 1 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 41 | VCELL_02 | Điện áp Cell 2 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 42 | VCELL_03 | Điện áp Cell 3 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 43 | VCELL_04 | Điện áp Cell 4 (uint16, ×0.0001) | 0 ~ 6.5535 V |

#### 0x312 — PACK_InfoVoltageCell2 (địa chỉ 44-47)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 44 | VCELL_05 | Điện áp Cell 5 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 45 | VCELL_06 | Điện áp Cell 6 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 46 | VCELL_07 | Điện áp Cell 7 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 47 | VCELL_08 | Điện áp Cell 8 (uint16, ×0.0001) | 0 ~ 6.5535 V |

#### 0x313 — PACK_InfoVoltageCell3 (địa chỉ 48-51)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 48 | VCELL_09 | Điện áp Cell 9 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 49 | VCELL_10 | Điện áp Cell 10 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 50 | VCELL_11 | Điện áp Cell 11 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 51 | VCELL_12 | Điện áp Cell 12 (uint16, ×0.0001) | 0 ~ 6.5535 V |

#### 0x314 — PACK_InfoVoltageCell4 (địa chỉ 52-55, DLC=2)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 52 | VCELL_13 | Điện áp Cell 13 (uint16, ×0.0001) | 0 ~ 6.5535 V |
| 53 | VCELL4_RSVD_1 | Dự phòng (DLC=2, không có dữ liệu) | — |
| 54 | VCELL4_RSVD_2 | Dự phòng | — |
| 55 | VCELL4_RSVD_3 | Dự phòng | — |

#### 0x315 — PACK_InfoDemBMS (địa chỉ 56-59, DLC=4) — Chẩn đoán BMS

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 56 | DEM_BMS_0 | [demCFET(4b) &#124; demDFET(4b)] [demPFET(4b) &#124; demASICComm(4b)] | 0=Normal, 3=Fault |
| 57 | DEM_BMS_1 | [demASICShutdown(4b) &#124; demShortCurrent(4b)] [demFETTemp(4b) &#124; demFuseBlow(4b)] | 0=Normal, 3=Fault |
| 58 | DEM_BMS_RSVD_0 | Dự phòng (DLC=4, không có dữ liệu) | — |
| 59 | DEM_BMS_RSVD_1 | Dự phòng | — |

#### 0x320 — PACK_InfoTemperatureCell (địa chỉ 60-63)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 60 | TEMP_CELL_AVG_MIN | [TempCellAvg(8b signed)] [TempCellMin(8b signed)] | -127 ~ 127 °C mỗi trường |
| 61 | TEMP_CELL_MIN_NO | [TempCellMinNo(8b)] [TempCellMax(8b signed)] | MinNo: 0~255. Max: -127~127 °C |
| 62 | TEMP_CELL_MAX_NO | [TempCellMaxNo(8b)] [Reserved(8b)] | MaxNo: 0~255 |
| 63 | TEMP_CELL_RSVD | Dự phòng | — |

#### 0x322 — PACK_InfoTemperatureCB (địa chỉ 64-67)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 64 | TEMP_CB1_CB2 | [TempCB1(8b signed)] [TempCB2(8b signed)] — Nhiệt độ mạch cân bằng 1, 2 | -127 ~ 127 °C mỗi trường |
| 65 | TEMP_FET | [TempFET(8b signed)] [Reserved(8b)] — Nhiệt độ FET | -127 ~ 127 °C |
| 66 | TEMP_CB_RSVD_0 | Dự phòng | — |
| 67 | TEMP_CB_RSVD_1 | Dự phòng | — |

#### 0x32F — PACK_InfoPackVersion (địa chỉ 68-71)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 68 | VERSION_SW | [SWMinor(4b) &#124; SWSubminor(4b)] [BLMajor(4b) &#124; SWMajor(4b)] | Mỗi trường: 0~15 |
| 69 | VERSION_BL | [BLMinor(4b) &#124; BLSubminor(4b)] [HWMajor(4b) &#124; Reserved(4b)] | Mỗi trường: 0~15 |
| 70 | VERSION_MFG_DATE | Ngày sản xuất (uint16) | 0 ~ 65535 |
| 71 | VERSION_OTP | Mã OTP BQ (uint16) | 0 ~ 65535 |

#### Thanh ghi trạng thái Station (chỉ đọc, địa chỉ 72-73)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 72 | IS_PIN_IN_SLOT | Trạng thái slot — phát hiện bởi Limit Switch | 0 = Trống, 1 = Có Pin |
| 73 | IS_PIN_TIMEOUT | Trạng thái kết nối CAN với Pack Pin | 0 = Bình thường, 1 = Mất kết nối |

#### Thanh ghi điều khiển (đọc/ghi, địa chỉ 74-75)

| Địa chỉ | Tên | Mô tả | Khoảng giá trị |
|----------|-----|-------|----------------|
| 74 | IS_EMERGENCY_STOP | Lệnh dừng khẩn cấp — Master ghi để kích hoạt/hủy | 0 = Bình thường, 1 = Dừng khẩn cấp |
| 75 | CHRG_CTRL | Lệnh điều khiển sạc — Master ghi để bật/tắt relay sạc | 0 = Tắt sạc, 1 = Bật sạc |

### 7.4. Ví dụ giao tiếp

**Đọc toàn bộ dữ liệu Pin (72 thanh ghi, bắt đầu từ địa chỉ 0):**

```
Master gửi:  [Addr] [03] [00 00] [00 48] [CRC16]
                │     │     │        │
                │     │     │        └─ Số thanh ghi: 72 (0x0048)
                │     │     └────────── Địa chỉ bắt đầu: 0
                │     └──────────────── Function Code: Read Holding Registers
                └────────────────────── Địa chỉ Slave (1-31)
```

**Bật sạc (ghi thanh ghi 75 = 1):**

```
Master gửi:  [Addr] [06] [00 4B] [00 01] [CRC16]
                │     │     │        │
                │     │     │        └─ Giá trị: 1 (bật sạc)
                │     │     └────────── Địa chỉ thanh ghi: 75 (0x004B)
                │     └──────────────── Function Code: Write Single Register
                └────────────────────── Địa chỉ Slave
```

**Dừng khẩn cấp (ghi thanh ghi 74 = 1):**

```
Master gửi:  [Addr] [06] [00 4A] [00 01] [CRC16]
```

**Đọc trạng thái slot và timeout (2 thanh ghi từ địa chỉ 72):**

```
Master gửi:  [Addr] [03] [00 48] [00 02] [CRC16]
```

---

## 8. Giao tiếp CAN Bus

### 8.1. Thông số CAN

| Thông số | Giá trị |
|----------|---------|
| Chuẩn | CAN 2.0A (Standard ID 11-bit) |
| Tốc độ | 1 Mbps |
| Sample point | 77.8% |
| Chiều dài bus tối đa | ~25m (dây xoắn đôi có bọc) |

### 8.2. Định dạng dữ liệu từ Pack Pin

Pack Pin (BMS) phát quảng bá dữ liệu qua **18 CAN message** (theo DBC VMO_SinglePack_DBC_V0_1), mỗi frame chứa **4 thanh ghi 16-bit** theo thứ tự big-endian:

| CAN ID | Thanh ghi | Tên Message | Nội dung |
|--------|-----------|-------------|----------|
| 0x300 | 0-3 | PACK_ControlSystem | Dòng & công suất sạc/xả tối đa |
| 0x301 | 4-7 | PACK_InfoCharging | Giới hạn V/I sạc, trạng thái sạc |
| 0x303 | 8-11 | PACK_InfoBms | Khẩn cấp, chế độ BMS, rolling counter |
| 0x304 | 12-15 | PACK_InfoCellBalancing | Trạng thái cân bằng cell 1-13 |
| 0x306 | 16-19 | PACK_InfoDemCell | Chẩn đoán cell (quá áp, dòng, nhiệt) |
| 0x309 | 20-23 | PACK_InfoPack | Trạng thái FET, điện áp & dòng pack |
| 0x30A | 24-27 | PACK_InfoSox | SOC, SOH, số chu kỳ, thời gian sạc còn lại |
| 0x30B | 28-31 | PACK_AccumDsgChgCapacity | Dung lượng sạc/xả tích lũy (32-bit) |
| 0x30E | 32-35 | PACK_InfoContactor | Điện áp ADC (batt, pack, cầu chì), thời gian sạc |
| 0x310 | 36-39 | PACK_InfoVoltageCell | Điện áp cell TB/Min/Max |
| 0x311 | 40-43 | PACK_InfoVoltageCell1 | Điện áp Cell 1-4 |
| 0x312 | 44-47 | PACK_InfoVoltageCell2 | Điện áp Cell 5-8 |
| 0x313 | 48-51 | PACK_InfoVoltageCell3 | Điện áp Cell 9-12 |
| 0x314 | 52-55 | PACK_InfoVoltageCell4 | Điện áp Cell 13 (DLC=2) |
| 0x315 | 56-59 | PACK_InfoDemBMS | Chẩn đoán BMS (FET, ASIC, cầu chì) |
| 0x320 | 60-63 | PACK_InfoTemperatureCell | Nhiệt độ cell TB/Min/Max |
| 0x322 | 64-67 | PACK_InfoTemperatureCB | Nhiệt độ FET & CB |
| 0x32F | 68-71 | PACK_InfoPackVersion | Phiên bản SW/HW, ngày sản xuất |

**Cấu trúc mỗi frame (8 bytes):**

```
Byte 0-1: Thanh ghi thứ 1 (High byte, Low byte)
Byte 2-3: Thanh ghi thứ 2 (High byte, Low byte)
Byte 4-5: Thanh ghi thứ 3 (High byte, Low byte)
Byte 6-7: Thanh ghi thứ 4 (High byte, Low byte)
```

> **Lưu ý:** Một số frame có DLC < 8 (VoltageCell4: 2 bytes, DemBMS: 4 bytes). Các thanh ghi ngoài DLC sẽ giữ giá trị 0.

Thiết bị coi dữ liệu hợp lệ khi nhận đủ cả 18 frame. Nếu không nhận được dữ liệu trong **1 giây**, tính là một lần timeout. Sau **5 lần timeout liên tiếp**, thiết bị chuyển sang trạng thái mất kết nối.

---

## 9. Các trạng thái hoạt động

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

| Trạng thái | Điều kiện vào | Điều kiện thoát |
|------------|---------------|-----------------|
| **Hoạt động bình thường** | Khởi động với địa chỉ hợp lệ | Phát hiện lỗi BMS hoặc mất kết nối CAN |
| **Lỗi Pin** | Nhận dữ liệu Pin có cờ FAULT | Pin được tháo ra, hoặc nhận lại dữ liệu không có lỗi |
| **Mất kết nối** | Không nhận CAN sau 5 lần timeout | Pin được tháo ra, hoặc nhận lại dữ liệu thành công |

---

## 10. Xử lý sự cố

### LED_FAULT sáng liên tục

| Nguyên nhân | Cách xử lý |
|-------------|------------|
| Pack Pin báo lỗi (quá áp, quá dòng, quá nhiệt, ...) | Tháo Pin, kiểm tra tình trạng Pin. Lắp lại hoặc thay Pin khác. |
| BMS gặp sự cố nội bộ | Đọc thanh ghi DEM_CELL_VOLTAGE (địa chỉ 16) và DEM_BMS_0 (địa chỉ 56) để xác định loại lỗi. |

### LED_FAULT nhấp nháy

| Nguyên nhân | Cách xử lý |
|-------------|------------|
| Đứt dây CAN (CANH hoặc CANL) | Kiểm tra và nối lại dây CAN |
| Thiếu điện trở termination 120 Ohm | Gắn điện trở 120 Ohm ở hai đầu bus |
| CAN transceiver mất nguồn | Kiểm tra nguồn cấp cho CAN transceiver |
| Pack Pin không hoạt động | Kiểm tra Pin có đang bật và phát CAN không |
| Sai tốc độ CAN | Đảm bảo cả hai đầu đều cấu hình 1 Mbps |
| Bus CAN quá dài | Rút ngắn bus (tối đa ~25m ở 1 Mbps) |

### Master không đọc được dữ liệu

| Nguyên nhân | Cách xử lý |
|-------------|------------|
| Sai địa chỉ Modbus | Kiểm tra DIP Switch, đảm bảo khớp với cấu hình Master |
| Sai baudrate | Cài lại baudrate (xem mục 4.2) |
| Đứt dây RS485 | Kiểm tra kết nối A, B |
| Đấu ngược A/B | Đổi chéo dây A và B |
| Nhiều thiết bị cùng địa chỉ | Đảm bảo mỗi mạch BSS có địa chỉ duy nhất |

### LED_RUN không sáng, chỉ nhấp nháy

| Nguyên nhân | Cách xử lý |
|-------------|------------|
| DIP Switch đang ở vị trí = 0 | Đặt DIP Switch về địa chỉ mong muốn (1-31), rồi reset thiết bị |

### Thiết bị không phản hồi gì (không LED nào sáng)

| Nguyên nhân | Cách xử lý |
|-------------|------------|
| Mất nguồn | Kiểm tra jack nguồn DC và nguồn cấp |
| MCU bị treo | Nhấn nút Reset hoặc tắt/bật nguồn |
| Firmware lỗi | Nạp lại firmware qua ST-Link |

---

## Phụ lục A — Thông số CAN Bus chi tiết

| Thông số | Giá trị |
|----------|---------|
| Baud rate | 1,000,000 bps |
| Sample point | 77.8% |
| Prescaler | 2 |
| Time Segment 1 (BS1) | 13 TQ |
| Time Segment 2 (BS2) | 4 TQ |
| Sync Jump Width (SJW) | 2 TQ |
| Auto Bus-Off Recovery | Có |
| Auto Retransmission | Có |
| CAN ID nhận (RX) | 0x300 - 0x32F (18 message, xem bảng 8.2) |
| CAN ID gửi (TX) | 0x200 |
