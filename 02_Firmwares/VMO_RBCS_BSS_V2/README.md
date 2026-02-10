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
    +-------------+         CANH ──────────── CANH         +-------------+
    |             | ◄────── CANL ──────────── CANL ──────► |             |
    |  Mạch BSS   |         GND ───────────── GND          |  Pack Pin   |
    |  (Slave)    |                                         |  (BMS)      |
    |             |         RS485 Modbus RTU                +-------------+
    |             |          A ────────────── A
    |             | ◄──────► B ────────────── B ──────────► +-------------+
    +-------------+         GND ───────────── GND           |  Master     |
                                                            |  Controller |
                                                            +-------------+
```

### 1.3. Thông số kỹ thuật

| Thông số | Giá trị |
|----------|---------|
| Nguồn cấp | 3.3V DC (hoặc 5V qua mạch ổn áp onboard) |
| Vi xử lý | STM32F103C8T6 |
| Giao tiếp Pin | CAN Bus 2.0A, 1 Mbps |
| Giao tiếp Master | RS485 Half-duplex, Modbus RTU |
| Baudrate RS485 | Cấu hình được (1200 - 115200 bps) |
| Địa chỉ Modbus | Cấu hình được (1 - 31) |
| Số thanh ghi Modbus | 52 thanh ghi 16-bit |
| Cảm biến slot | 2x Limit Switch (phát hiện Pin) |
| Ngõ ra điều khiển | 1x Điều khiển sạc, 1x Dừng khẩn cấp |
| Chỉ thị | 3x LED (RUN, FAULT, STATUS) |
| Kích thước | Theo thiết kế PCB |

---

## 2. Mô tả phần cứng

### 2.1. Các cổng kết nối

#### Cổng CAN Bus (kết nối Pack Pin)

| Tín hiệu | Mô tả |
|-----------|-------|
| CANH | CAN High |
| CANL | CAN Low |
| GND | Mass chung |

> **Yêu cầu:** Gắn điện trở termination **120 Ohm** giữa CANH và CANL ở **hai đầu bus**. Chiều dài bus tối đa ở 1 Mbps: khoảng 25m (dùng dây xoắn đôi có bọc chống nhiễu).

#### Cổng RS485 (kết nối Master Controller)

| Tín hiệu | Mô tả |
|-----------|-------|
| A | RS485 Data+ |
| B | RS485 Data- |
| GND | Mass chung |

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

Cấp nguồn 3.3V DC (hoặc 5V nếu board có mạch ổn áp) vào chân nguồn. Đảm bảo GND chung giữa mạch BSS, CAN transceiver và RS485 transceiver.

### 3.2. Bước 2 — Kết nối CAN Bus với Pack Pin

1. Nối **CANH** của mạch BSS với **CANH** của Pack Pin (BMS)
2. Nối **CANL** của mạch BSS với **CANL** của Pack Pin (BMS)
3. Nối **GND** chung
4. Gắn điện trở **120 Ohm** giữa CANH-CANL ở cả hai đầu bus

```
Mạch BSS                              Pack Pin (BMS)
  CANH ──── [120R] ──── dây ──── [120R] ──── CANH
  CANL ──────────────── dây ──────────────── CANL
  GND  ──────────────── dây ──────────────── GND
```

### 3.3. Bước 3 — Kết nối RS485 với Master Controller

1. Nối **A** của mạch BSS với **A** của Master Controller
2. Nối **B** của mạch BSS với **B** của Master Controller
3. Nối **GND** chung

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
| Bật sạc | 51 (CHRG_CTRL) | 1 | Kích hoạt ngõ ra CHARGE_CTRL |
| Tắt sạc | 51 (CHRG_CTRL) | 0 | Ngắt ngõ ra CHARGE_CTRL |
| Dừng khẩn cấp | 50 (EMERGENCY_STOP) | 1 | Kích hoạt ngõ ra EMERGENCY, ngắt sạc |
| Hủy dừng khẩn cấp | 50 (EMERGENCY_STOP) | 0 | Tắt tín hiệu EMERGENCY |

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

#### Thanh ghi dữ liệu Pin (chỉ đọc, địa chỉ 0 - 47)

| Địa chỉ | Tên | Mô tả | Đơn vị |
|----------|-----|-------|--------|
| 0 | BMS_STATE | Trạng thái BMS | — |
| 1 | CTRL_REQUEST | Yêu cầu điều khiển | — |
| 2 | CTRL_RESPONSE | Phản hồi điều khiển | — |
| 3 | FET_CTRL_PIN | Trạng thái chân điều khiển FET | — |
| 4 | FET_STATUS | Trạng thái FET (đóng/mở) | — |
| 5 | ALARM_BITS | Bit cảnh báo | Bitmask |
| 6 | FAULTS | Mã lỗi | Bitmask |
| 7 | PACK_VOLT | Điện áp tổng Pack | mV |
| 8 | STACK_VOLT | Điện áp Stack | mV |
| 9 | PACK_CURRENT_H | Dòng điện Pack (byte cao) | mA |
| 10 | PACK_CURRENT_L | Dòng điện Pack (byte thấp) | mA |
| 11 | ID_VOLT | Điện áp nhận dạng | mV |
| 12 | TEMP1_H | Nhiệt độ cảm biến 1 (byte cao) | 0.1°C |
| 13 | TEMP1_L | Nhiệt độ cảm biến 1 (byte thấp) | 0.1°C |
| 14 | TEMP2_H | Nhiệt độ cảm biến 2 (byte cao) | 0.1°C |
| 15 | TEMP2_L | Nhiệt độ cảm biến 2 (byte thấp) | 0.1°C |
| 16 | TEMP3_H | Nhiệt độ cảm biến 3 (byte cao) | 0.1°C |
| 17 | TEMP3_L | Nhiệt độ cảm biến 3 (byte thấp) | 0.1°C |
| 18 - 29 | CELL1 - CELL12 | Điện áp Cell 1 đến Cell 12 | mV |
| 30 - 32 | (Dự phòng) | Không sử dụng | — |
| 33 | CELL13 | Điện áp Cell 13 | mV |
| 34 | SAFETY_A | Thanh ghi an toàn A | Bitmask |
| 35 | SAFETY_B | Thanh ghi an toàn B | Bitmask |
| 36 | SAFETY_C | Thanh ghi an toàn C | Bitmask |
| 37 | ACCU_INT_H | Dung lượng tích lũy - nguyên (cao) | mAh |
| 38 | ACCU_INT_L | Dung lượng tích lũy - nguyên (thấp) | mAh |
| 39 | ACCU_FRAC_H | Dung lượng tích lũy - thập phân (cao) | — |
| 40 | ACCU_FRAC_L | Dung lượng tích lũy - thập phân (thấp) | — |
| 41 | ACCU_TIME_H | Thời gian tích lũy (cao) | giây |
| 42 | ACCU_TIME_L | Thời gian tích lũy (thấp) | giây |
| 43 | PIN_PERCENT | Phần trăm Pin | % |
| 44 | PERCENT_TARGET | Phần trăm mục tiêu sạc | % |
| 45 | CELL_RESISTANCE | Điện trở nội Cell | mOhm |
| 46 | SOC | State of Charge | % |
| 47 | SOH | State of Health | % |

#### Thanh ghi trạng thái Station (chỉ đọc, địa chỉ 48 - 49)

| Địa chỉ | Tên | Mô tả | Giá trị |
|----------|-----|-------|---------|
| 48 | IS_PIN_IN_SLOT | Trạng thái slot | 0 = Trống, 1 = Có Pin |
| 49 | IS_PIN_TIMEOUT | Trạng thái kết nối CAN | 0 = Bình thường, 1 = Mất kết nối |

#### Thanh ghi điều khiển (đọc/ghi, địa chỉ 50 - 51)

| Địa chỉ | Tên | Mô tả | Giá trị |
|----------|-----|-------|---------|
| 50 | IS_EMERGENCY_STOP | Lệnh dừng khẩn cấp | 0 = Bình thường, 1 = Dừng khẩn cấp |
| 51 | CHRG_CTRL | Lệnh điều khiển sạc | 0 = Tắt sạc, 1 = Bật sạc |

### 7.4. Ví dụ giao tiếp

**Đọc toàn bộ dữ liệu Pin (48 thanh ghi, bắt đầu từ địa chỉ 0):**

```
Master gửi:  [Addr] [03] [00 00] [00 30] [CRC16]
                │     │     │        │
                │     │     │        └─ Số thanh ghi: 48 (0x0030)
                │     │     └────────── Địa chỉ bắt đầu: 0
                │     └──────────────── Function Code: Read Holding Registers
                └────────────────────── Địa chỉ Slave (1-31)
```

**Bật sạc (ghi thanh ghi 51 = 1):**

```
Master gửi:  [Addr] [06] [00 33] [00 01] [CRC16]
                │     │     │        │
                │     │     │        └─ Giá trị: 1 (bật sạc)
                │     │     └────────── Địa chỉ thanh ghi: 51 (0x0033)
                │     └──────────────── Function Code: Write Single Register
                └────────────────────── Địa chỉ Slave
```

**Dừng khẩn cấp (ghi thanh ghi 50 = 1):**

```
Master gửi:  [Addr] [06] [00 32] [00 01] [CRC16]
```

**Đọc trạng thái slot và timeout (2 thanh ghi từ địa chỉ 48):**

```
Master gửi:  [Addr] [03] [00 30] [00 02] [CRC16]
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

Pack Pin (BMS) phát quảng bá dữ liệu qua **12 CAN frame**, mỗi frame chứa **4 thanh ghi 16-bit** theo thứ tự big-endian:

| CAN ID | Thanh ghi | Nội dung |
|--------|-----------|----------|
| 0x100 | 0 - 3 | BMS State, Control Request/Response, FET Control |
| 0x101 | 4 - 7 | FET Status, Alarm, Faults, Pack Voltage |
| 0x102 | 8 - 11 | Stack Voltage, Pack Current (H/L), ID Voltage |
| 0x103 | 12 - 15 | Temperature 1 (H/L), Temperature 2 (H/L) |
| 0x104 | 16 - 19 | Temperature 3 (H/L), Cell 1, Cell 2 |
| 0x105 | 20 - 23 | Cell 3, Cell 4, Cell 5, Cell 6 |
| 0x106 | 24 - 27 | Cell 7, Cell 8, Cell 9, Cell 10 |
| 0x107 | 28 - 31 | Cell 11, Cell 12, (Dự phòng) x2 |
| 0x108 | 32 - 35 | (Dự phòng), Cell 13, Safety A, Safety B |
| 0x109 | 36 - 39 | Safety C, Accumulator Integer (H/L), Accumulator Frac H |
| 0x10A | 40 - 43 | Accumulator Frac L, Accumulator Time (H/L), Pin Percent |
| 0x10B | 44 - 47 | Percent Target, Cell Resistance, SOC, SOH |

**Cấu trúc mỗi frame (8 bytes):**

```
Byte 0-1: Thanh ghi thứ 1 (High byte, Low byte)
Byte 2-3: Thanh ghi thứ 2 (High byte, Low byte)
Byte 4-5: Thanh ghi thứ 3 (High byte, Low byte)
Byte 6-7: Thanh ghi thứ 4 (High byte, Low byte)
```

Thiết bị coi dữ liệu hợp lệ khi nhận đủ cả 12 frame. Nếu không nhận được dữ liệu trong **1 giây**, tính là một lần timeout. Sau **5 lần timeout liên tiếp**, thiết bị chuyển sang trạng thái mất kết nối.

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
| BMS gặp sự cố nội bộ | Đọc thanh ghi FAULTS (địa chỉ 6) để xác định loại lỗi. |

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
| Đứt dây RS485 | Kiểm tra kết nối A, B, GND |
| Đấu ngược A/B | Đổi chéo dây A và B |
| Nhiều thiết bị cùng địa chỉ | Đảm bảo mỗi mạch BSS có địa chỉ duy nhất |

### LED_RUN không sáng, chỉ nhấp nháy

| Nguyên nhân | Cách xử lý |
|-------------|------------|
| DIP Switch đang ở vị trí = 0 | Đặt DIP Switch về địa chỉ mong muốn (1-31), rồi reset thiết bị |

### Thiết bị không phản hồi gì (không LED nào sáng)

| Nguyên nhân | Cách xử lý |
|-------------|------------|
| Mất nguồn | Kiểm tra nguồn cấp 3.3V/5V |
| MCU bị treo | Nhấn nút Reset hoặc tắt/bật nguồn |
| Firmware lỗi | Nạp lại firmware qua ST-Link |

---

## 11. Thông tin bảo trì

- **Không** có linh kiện nào cần thay thế định kỳ
- **Không** mở nắp thiết bị khi đang cấp nguồn
- Bảo quản ở nơi khô ráo, tránh ẩm ướt và bụi bẩn
- Nhiệt độ hoạt động: -20°C đến +70°C
- Kiểm tra kết nối dây CAN và RS485 định kỳ, đảm bảo không bị oxy hóa hoặc lỏng

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
| CAN ID nhận (RX) | 0x100 - 0x10B |
| CAN ID gửi (TX) | 0x200 |
