# VMO_RBCS_BSS_V2 - Battery Station Slave Firmware

Firmware cho mạch đọc dữ liệu Pin (BMS) và điều khiển trạm sạc/thay pin.
MCU STM32F103C8T6, chạy FreeRTOS, giao tiếp CAN Bus với Pack Pin và Modbus RTU RS485 với Master Controller.

## Mục lục

- [Tổng quan hệ thống](#tổng-quan-hệ-thống)
- [Yêu cầu phần cứng](#yêu-cầu-phần-cứng)
- [Sơ đồ chân GPIO](#sơ-đồ-chân-gpio)
- [Cài đặt môi trường phát triển](#cài-đặt-môi-trường-phát-triển)
- [Cấu trúc thư mục](#cấu-trúc-thư-mục)
- [Cấu hình CAN Bus](#cấu-hình-can-bus)
- [Cấu hình Modbus Slave RS485](#cấu-hình-modbus-slave-rs485)
- [Hướng dẫn sử dụng thiết bị](#hướng-dẫn-sử-dụng-thiết-bị)
- [Mô hình trạng thái HSM](#mô-hình-trạng-thái-hsm)
- [Bảng thanh ghi dữ liệu](#bảng-thanh-ghi-dữ-liệu)
- [Thư viện CAN Battery API](#thư-viện-can-battery-api)
- [Xây dựng và nạp firmware](#xây-dựng-và-nạp-firmware)
- [Xử lý sự cố](#xử-lý-sự-cố)

---

## Tổng quan hệ thống

### Thông số kỹ thuật

| Thông số | Giá trị |
|----------|---------|
| MCU | STM32F103C8T6 (ARM Cortex-M3) |
| Flash | 64 KB |
| RAM | 20 KB |
| Clock | 72 MHz (HSE 8MHz + PLL x9) |
| RTOS | FreeRTOS V10 (CMSIS-RTOS V2) |
| IDE | STM32CubeIDE |
| HAL | STM32Cube FW_F1 V1.8.6 |

### Kiến trúc giao tiếp

```
+-------------------+          CAN Bus 1Mbps           +------------------+
|                   |  <------------------------------  |                  |
|   BSS Slave       |    (Pack Pin broadcast data)      |    Pack Pin      |
|   (STM32F103)     |                                   |    (BMS)         |
|                   |                                   +------------------+
|                   |
|                   |        RS485 Modbus RTU            +------------------+
|                   |  <------------------------------>  |                  |
+-------------------+    (Slave, địa chỉ 1-31)          | Master Controller|
                                                        +------------------+
```

**Luồng dữ liệu:**
1. Pack Pin (BMS) tự động phát quảng bá dữ liệu qua CAN Bus
2. BSS Slave nhận và lưu trữ dữ liệu Pin
3. Master Controller đọc dữ liệu từ BSS Slave qua Modbus RTU RS485
4. Master Controller gửi lệnh điều khiển (sạc, dừng khẩn cấp) qua Modbus

---

## Yêu cầu phần cứng

### Linh kiện chính
- 1x Board STM32F103C8T6 (Blue Pill hoặc PCB custom)
- 1x Thạch anh 8MHz
- 1x CAN Transceiver (MCP2551, TJA1050, hoặc SN65HVD230)
- 1x RS485 Transceiver (MAX485, SP3485)
- 1x DIP Switch 5 bit (đặt địa chỉ Modbus)
- 2x Limit Switch (phát hiện Pin trong slot)
- 3x LED (RUN, FAULT, STATUS)
- Điện trở termination 120 Ohm cho CAN bus (2 đầu bus)

### Kết nối

```
CAN Bus:   PA11 (CAN_RX) ---|CAN        |--- CANH ---> Pack Pin BMS
           PA12 (CAN_TX) ---|Transceiver |--- CANL --->
                             |            |--- GND  --->

RS485:     PA2 (USART2_TX) --|RS485      |--- A -----> Master Controller
           PA3 (USART2_RX) --|Transceiver|--- B ----->
           PA4 (TX Enable)  --|           |--- GND --->
```

---

## Sơ đồ chân GPIO

### Giao tiếp

| Chân | Tên | Chức năng | Hướng |
|------|-----|-----------|-------|
| PA2 | RS4851_DI | USART2 TX (RS485 COM) | Output AF |
| PA3 | RS4851_RO | USART2 RX (RS485 COM) | Input |
| PA4 | RS4851_TXEN | RS485 TX Enable | Output |
| PA11 | CAN_RX | CAN nhận dữ liệu từ Pack | Input |
| PA12 | CAN_TX | CAN phát dữ liệu | Output AF |

### Điều khiển

| Chân | Tên | Chức năng | Hướng |
|------|-----|-----------|-------|
| PA8 | CHARGE_CTRL | Điều khiển sạc (HIGH = bật) | Output |
| PA10 | EMERGENCY | Dừng khẩn cấp (HIGH = kích hoạt) | Output |

### DIP Switch (địa chỉ Modbus)

| Chân | Tên | Bit | Ghi chú |
|------|-----|-----|---------|
| PB0 | ADDR4 | bit 0 | Pull-up, LOW = 1 |
| PB1 | ADDR3 | bit 1 | Pull-up, LOW = 1 |
| PB2 | ADDR2 | bit 2 | Pull-up, LOW = 1 |
| PB3 | ADDR1 | bit 3 | Pull-up, LOW = 1 |
| PB4 | ADDR0 | bit 4 | Pull-up, LOW = 1 |

Địa chỉ = 0 thì mặc định là 1. Dải địa chỉ: 1-31.

### LED chỉ thị

| Chân | Tên | Chức năng |
|------|-----|-----------|
| PB5 | LED_STT | Nhấp nháy khi nhận dữ liệu từ Master |
| PB6 | LED_RUN | Sáng khi hoạt động bình thường |
| PB7 | LED_FAULT | Sáng khi có lỗi hoặc mất kết nối |

### Cảm biến

| Chân | Tên | Chức năng |
|------|-----|-----------|
| PB8 | LIMIT_SWITCH0 | Phát hiện Pin trong slot |
| PB9 | LIMIT_SWITCH1 | Phát hiện Pin trong slot |
| PA6 | RD_VOLTAGE | Đọc điện áp (Analog) |

---

## Cài đặt môi trường phát triển

### Yêu cầu

1. **STM32CubeIDE** >= 1.15.0 (hoặc Keil/IAR với HAL)
2. **ST-Link V2** hoặc **J-Link** để nạp firmware
3. **STM32CubeMX** >= 6.15.0 (nếu muốn sửa cấu hình .ioc)

### Các bước

```bash
# 1. Clone repository
git clone <repository-url>
cd VMO_RBCS_V2/02_Firmwares/VMO_RBCS_BSS_V2

# 2. Mở project trong STM32CubeIDE
#    File -> Import -> Existing Projects into Workspace
#    Chọn thư mục VMO_RBCS_BSS_V2

# 3. Build
#    Project -> Build Project (Ctrl+B)

# 4. Nạp firmware
#    Run -> Debug As -> STM32 C/C++ Application
```

---

## Cấu trúc thư mục

```
VMO_RBCS_BSS_V2/
|-- Core/
|   |-- Inc/                    # Header files (main.h, can.h, usart.h, ...)
|   |-- Src/                    # Source files
|   |   |-- main.c              # Điểm vào, cấu hình clock, khởi tạo ngoại vi
|   |   |-- can.c               # Khởi tạo CAN, bit timing, RX callback
|   |   |-- usart.c             # Khởi tạo USART2 (RS485 Modbus Slave)
|   |   |-- freertos.c          # Các task FreeRTOS (defaultTask, readBatteryTask, commTask)
|   |   |-- gpio.c              # Khởi tạo GPIO
|   |   |-- dma.c               # Khởi tạo DMA cho USART2
|   |   |-- tim.c               # Khởi tạo Timer2 (HSM timer)
|   |   |-- stm32f1xx_it.c      # Các hàm xử lý ngắt
|   |   +-- ...
|   +-- Startup/
|
|-- APP/                        # Ứng dụng chính
|   |-- app_states.h            # Định nghĩa struct, enum, register map
|   |-- app_states.c            # HSM state machine (run, fault, bat_not_connected)
|   +-- app_params.c            # Khởi tạo EEPROM, cấu hình baudrate
|
|-- CAN-BAT/                    # Thư viện CAN Battery (MỚI)
|   |-- can_battery.h           # Header - API, cấu hình CAN ID, struct
|   +-- can_battery.c           # Implementation - init, filter, RX callback, truy xuất dữ liệu
|
|-- MODBUS-LIB/                 # Thư viện Modbus RTU (chỉ còn Slave)
|   |-- Modbus.h / Modbus.c
|   |-- ModbusConfig.h          # Cấu hình: MAX_M_HANDLERS=1 (chỉ Slave)
|   +-- UARTCallback.c
|
|-- HSM/                        # Thư viện Hierarchical State Machine
|-- EE/                         # Thư viện EEPROM emulation
|-- Drivers/                    # STM32 HAL drivers
|-- Middlewares/                 # FreeRTOS middleware
|
|-- RBCS_READ_BATTERY_DATA.ioc  # File project STM32CubeMX
|-- STM32F103C8TX_FLASH.ld      # Linker script
+-- README.md                   # File này
```

---

## Cấu hình CAN Bus

### Thông số bit timing

| Thông số | Giá trị | Ghi chú |
|----------|---------|---------|
| APB1 Clock | 36 MHz | SYSCLK/2 |
| Prescaler | 2 | |
| Time Quantum (TQ) | 55.56 ns | 36MHz / 2 = 18MHz |
| Sync Seg | 1 TQ | Cố định |
| BS1 (Time Seg 1) | 13 TQ | |
| BS2 (Time Seg 2) | 4 TQ | |
| SJW | 2 TQ | |
| Tổng bit time | 18 TQ | 1 + 13 + 4 |
| **Baud rate** | **1 Mbps** | 18MHz / 18 = 1MHz |
| **Sample point** | **77.8%** | (1+13)/18 |

### Các tham số khác

| Thông số | Giá trị | Ý nghĩa |
|----------|---------|---------|
| Mode | Normal | Giao tiếp bình thường |
| AutoBusOff | Enable | Tự phục hồi khi Bus-Off |
| AutoWakeUp | Enable | Tự đánh thức |
| AutoRetransmission | Enable | Tự gửi lại khi lỗi |

### Bộ lọc CAN (Filter)

- **Filter Bank 0**, chế độ Mask 32-bit
- Chấp nhận CAN ID: `0x100` - `0x10F`
- Gán vào RX FIFO0
- Ngắt `CAN_IT_RX_FIFO0_MSG_PENDING` được bật

### Giao thức CAN từ Pack Pin

BMS phát quảng bá dữ liệu qua 12 CAN frame. Mỗi frame chứa 4 thanh ghi 16-bit (big-endian):

| CAN ID | Nội dung | Thanh ghi |
|--------|----------|-----------|
| 0x100 | Frame 0 | registers[0..3] |
| 0x101 | Frame 1 | registers[4..7] |
| 0x102 | Frame 2 | registers[8..11] |
| ... | ... | ... |
| 0x10B | Frame 11 | registers[44..47] |

**Định dạng byte trong mỗi frame (8 bytes):**
```
Byte[0] = reg[N] >> 8     (high byte)
Byte[1] = reg[N] & 0xFF   (low byte)
Byte[2] = reg[N+1] >> 8
Byte[3] = reg[N+1] & 0xFF
Byte[4] = reg[N+2] >> 8
Byte[5] = reg[N+2] & 0xFF
Byte[6] = reg[N+3] >> 8
Byte[7] = reg[N+3] & 0xFF
```

> **Lưu ý:** Nếu giao thức CAN của BMS khác (ví dụ DBC `VMO_SinglePack_DBC_V0_1`),
> bạn cần sửa các `#define` trong `CAN-BAT/can_battery.h` và hàm parse trong
> `CAN_BAT_RxCallback()` (file `CAN-BAT/can_battery.c`).

---

## Cấu hình Modbus Slave RS485

### Thông số kết nối

| Thông số | Giá trị |
|----------|---------|
| UART | USART2 (PA2-TX, PA3-RX) |
| TX Enable | PA4 (RS4851_TXEN) |
| Địa chỉ Slave | Từ DIP Switch (1-31) |
| Baudrate | Từ EEPROM (mặc định 115200) |
| Data format | 8-N-1 |
| Giao thức | Modbus RTU |
| Chế độ | DMA |
| Timeout | 1000 ms |
| Số thanh ghi | 52 (TOTAL_STA_REGISTERS) |

### Bảng mã Baudrate

Mã baudrate được lưu trong EEPROM, cài đặt qua DIP Switch:

| Mã | Baudrate | Mã | Baudrate |
|----|----------|----|----------|
| 1 | 1200 | 6 | 19200 |
| 2 | 2400 | 7 | 38400 |
| 3 | 4800 | 8 | 56000 |
| 4 | 9600 | 9 | 57600 |
| 5 | 14400 | 10 | 115200 (mặc định) |

---

## Hướng dẫn sử dụng thiết bị

### 1. Kết nối phần cứng

1. Cấp nguồn cho board (3.3V hoặc 5V tùy thiết kế PCB)
2. Kết nối CAN transceiver:
   - PA11 -> CAN_RX của transceiver
   - PA12 -> CAN_TX của transceiver
   - CANH, CANL -> Bus CAN của Pack Pin
   - Gắn điện trở 120 Ohm ở hai đầu bus CAN
3. Kết nối RS485 transceiver:
   - PA2 -> DI (Data In)
   - PA3 -> RO (Receiver Out)
   - PA4 -> DE/RE (Direction Enable)
   - A, B -> Bus RS485 của Master Controller
4. Kết nối Limit Switch vào PB8, PB9 (nối GND khi kích hoạt)
5. Kết nối DIP Switch 5 bit vào PB0-PB4

### 2. Cài đặt địa chỉ Modbus

Dùng DIP Switch 5 bit (ADDR4..ADDR0) để đặt địa chỉ:

```
Ví dụ: Địa chỉ = 5
  ADDR4 (PB0) = OFF  (bit0 = 1)
  ADDR3 (PB1) = ON   (bit1 = 0)
  ADDR2 (PB2) = OFF  (bit2 = 1)
  ADDR1 (PB3) = OFF  (bit3 = 0)
  ADDR0 (PB4) = OFF  (bit4 = 0)
  -> addr = 0b00101 = 5
```

**Lưu ý:** Logic đảo - công tắc BẬT (nối GND) = bit 1.

### 3. Cài đặt Baudrate (lần đầu)

1. Đặt tất cả DIP Switch về `OFF` (địa chỉ = 0) -> vào chế độ cài đặt
2. LED_RUN sẽ nhấp nháy (500ms)
3. Gạt DIP Switch đến giá trị baudrate mong muốn (1-10)
4. Giữ nguyên 5 giây, LED_RUN sáng liên -> đã lưu vào EEPROM
5. Reset thiết bị
6. Đặt lại DIP Switch về địa chỉ Modbus thực tế

### 4. Hoạt động bình thường

Sau khi khởi động với địa chỉ hợp lệ (1-31):

1. **LED_RUN sáng liên** -> Thiết bị hoạt động bình thường
2. Khi Pin được đặt vào slot -> Limit Switch kích hoạt
3. Thiết bị tự động nhận dữ liệu từ Pack Pin qua CAN Bus
4. Master Controller đọc dữ liệu qua Modbus RTU (Function Code 0x03)
5. **LED_STT nhấp nháy nhanh** -> Đang giao tiếp với Master
6. Master có thể gửi lệnh:
   - `REG_STA_CHRG_CTRL = 1` -> Bật sạc
   - `REG_STA_IS_EMERGENCY_STOP = 1` -> Dừng khẩn cấp

### 5. Chỉ thị LED

| Trạng thái | LED_RUN | LED_FAULT | LED_STT |
|------------|---------|-----------|---------|
| Cài đặt baudrate | Nhấp nháy 500ms | Tắt | Tắt |
| Bình thường | Sáng | Tắt | Nhấp nháy khi nhận lệnh |
| Lỗi BMS | Tắt | Sáng | Tắt |
| Mất kết nối Pin | Tắt | Nhấp nháy 500ms | Tắt |

---

## Mô hình trạng thái HSM

```
                +--------------------+
                | Cài đặt Baudrate   |  (địa chỉ = 0)
                +--------------------+
                         |
                   địa chỉ != 0
                         v
                  +-------------+
           +----->|    Chạy     |<------+
           |      +-------------+       |
           |         |       |          |
           |    có FAULT  timeout>=5    |
           |         v       v          |
           |   +-------+ +----------+  |
           +---|  Lỗi  | | Mất kết  |--+
           |   +-------+ | nối Pin  |  |
           |      ^      +----------+  |
           |      |          |         |
           |      +----------+         |
           |       có FAULT            |
           +---------------------------+
              Limit Switch tắt hoặc
              nhận OK + không lỗi
```

### Chi tiết chuyển trạng thái

| Từ | Đến | Điều kiện |
|----|-----|-----------|
| Chạy | Lỗi | Nhận dữ liệu OK nhưng có FAULT |
| Chạy | Mất kết nối | CAN timeout >= 5 lần liên tiếp |
| Lỗi | Chạy | Limit Switch tắt HOẶC nhận OK + không FAULT |
| Mất kết nối | Chạy | Limit Switch tắt HOẶC nhận OK + không FAULT |
| Mất kết nối | Lỗi | Nhận OK nhưng có FAULT |

---

## Bảng thanh ghi dữ liệu

### Thanh ghi Pin (Battery Registers) - Đọc từ CAN

48 thanh ghi 16-bit, thứ tự trong `dataBattery[]`:

| Chỉ số | Tên | Mô tả |
|--------|-----|-------|
| 0 | BMS_STATE | Trạng thái BMS |
| 1 | CTRL_REQUEST | Yêu cầu điều khiển |
| 2 | CTRL_RESPONSE | Phản hồi điều khiển |
| 3 | FET_CTRL_PIN | Chân điều khiển FET |
| 4 | FET_STATUS | Trạng thái FET |
| 5 | ALARM_BITS | Bit cảnh báo |
| 6 | FAULTS | Mã lỗi |
| 7 | PACK_VOLT | Điện áp Pack |
| 8 | STACK_VOLT | Điện áp Stack |
| 9-10 | PACK_CURRENT | Dòng điện Pack (High/Low) |
| 11 | ID_VOLT | Điện áp nhận dạng |
| 12-17 | TEMP1/2/3 | Nhiệt độ 3 cảm biến (High/Low) |
| 18-29 | CELL1-CELL12 | Điện áp Cell 1-12 |
| 30-32 | CELL_NONE | Dự phòng |
| 33 | CELL13 | Điện áp Cell 13 |
| 34-36 | SAFETY_A/B/C | Thanh ghi an toàn |
| 37-38 | ACCU_INT | Tích lũy (nguyên) H/L |
| 39-40 | ACCU_FRAC | Tích lũy (thập phân) H/L |
| 41-42 | ACCU_TIME | Thời gian tích lũy H/L |
| 43 | PIN_PERCENT | Phần trăm Pin |
| 44 | PERCENT_TARGET | Phần trăm mục tiêu |
| 45 | CELL_RESISTANCE | Điện trở Cell |
| 46 | SOC_PERCENT | SOC (%) |
| 47 | SOH_VALUE | SOH (%) |

### Thanh ghi Station (Modbus Slave Registers) - Master đọc/ghi

52 thanh ghi, bao gồm toàn bộ thanh ghi Pin + 4 thanh ghi bổ sung:

| Chỉ số | Tên | Mô tả | Đọc/Ghi |
|--------|-----|-------|---------|
| 0-47 | (như trên) | Dữ liệu Pin copy từ CAN | Đọc |
| 48 | IS_PIN_IN_SLOT | Pin trong slot (0=Trống, 1=Đầy) | Đọc |
| 49 | IS_PIN_TIMEOUT | CAN timeout (0=OK, 1=Timeout) | Đọc |
| 50 | IS_EMERGENCY_STOP | Lệnh dừng khẩn cấp | Ghi |
| 51 | CHRG_CTRL | Lệnh điều khiển sạc | Ghi |

---

## Thư viện CAN Battery API

File: `CAN-BAT/can_battery.h` và `CAN-BAT/can_battery.c`

### Cấu hình (sửa trong can_battery.h)

```c
#define CAN_BAT_BASE_ID         0x100   // CAN ID đầu tiên của BMS
#define CAN_BAT_FRAME_COUNT     12      // Số frame CAN (48 regs / 4 per frame)
#define CAN_BAT_REGS_PER_FRAME  4       // Số thanh ghi 16-bit mỗi frame
#define CAN_BAT_TX_CMD_ID       0x200   // CAN ID gửi lệnh đến BMS
#define CAN_BAT_TIMEOUT_MS      1000    // Timeout (ms)
```

### Các hàm API

```c
// Khởi tạo driver: cấu hình filter, start CAN, bật ngắt
CAN_BAT_Status_t CAN_BAT_Init(CAN_BAT_Handle_t *handle, CAN_HandleTypeDef *hcan);

// Kiểm tra timeout
uint8_t CAN_BAT_IsTimeout(CAN_BAT_Handle_t *handle);

// Kiểm tra đã nhận đủ dữ liệu chưa
uint8_t CAN_BAT_IsDataReady(CAN_BAT_Handle_t *handle);

// Copy dữ liệu Pin ra buffer (thread-safe)
CAN_BAT_Status_t CAN_BAT_GetData(CAN_BAT_Handle_t *handle, uint16_t *dest, uint16_t count);

// Gửi lệnh đến BMS (tối đa 8 byte)
CAN_BAT_Status_t CAN_BAT_SendCommand(CAN_BAT_Handle_t *handle, uint8_t *data, uint8_t len);

// Đặt task nhận notification khi dữ liệu sẵn sàng
void CAN_BAT_SetNotifyTask(CAN_BAT_Handle_t *handle, TaskHandle_t taskHandle);

// Gọi từ HAL_CAN_RxFifo0MsgPendingCallback (đã được gọi từ can.c)
void CAN_BAT_RxCallback(CAN_BAT_Handle_t *handle);
```

### Cách hoạt động

1. **`CAN_BAT_Init()`** cấu hình filter CAN, bật CAN, bật ngắt RX FIFO0
2. Khi CAN frame đến -> **ISR** gọi `CAN_BAT_RxCallback()` tự động
3. Callback phân tích CAN ID, lưu 4 thanh ghi 16-bit vào buffer
4. Khi nhận đủ 12 frame -> đặt cờ `dataReady` và notify task đang chờ
5. **`StartReadBatteryTask`** nhận notification, gọi `CAN_BAT_GetData()` để copy dữ liệu
6. Dữ liệu được chuyển vào `dataBattery[]` -> copy sang `dataModbusSlave[]` khi Master đọc

---

## Xây dựng và nạp firmware

### Build với STM32CubeIDE

1. Mở project trong STM32CubeIDE
2. Chọn Build Configuration: **Debug** hoặc **Release**
3. `Project -> Build Project` (Ctrl+B)
4. Kiểm tra output: `Debug/RBCS_READ_BATTERY_DATA.elf`

### Nạp firmware

**Qua ST-Link:**
```
Run -> Debug As -> STM32 C/C++ Application
```

**Qua command line (dùng STM32CubeProgrammer):**
```bash
STM32_Programmer_CLI -c port=SWD -w Debug/RBCS_READ_BATTERY_DATA.elf -v -rst
```

### Sửa cấu hình CubeMX

Nếu muốn thay đổi cấu hình ngoại vi:
1. Mở file `RBCS_READ_BATTERY_DATA.ioc` bằng STM32CubeMX
2. Sửa cấu hình
3. Generate Code
4. **Lưu ý:** Code trong vùng `USER CODE BEGIN/END` sẽ được giữ lại

---

## Xử lý sự cố

### LED_FAULT sáng liên (trạng thái Lỗi)

- **Nguyên nhân:** BMS báo lỗi (thanh ghi FAULTS != 0)
- **Xử lý:** Kiểm tra trạng thái Pin, tháo Pin ra và lắp lại. Nếu lỗi vẫn còn, kiểm tra BMS.

### LED_FAULT nhấp nháy (trạng thái Mất kết nối)

- **Nguyên nhân:** Không nhận được dữ liệu CAN từ Pack Pin trong 5 giây (5 x 1s timeout)
- **Xử lý:**
  1. Kiểm tra kết nối CAN (CANH, CANL, GND)
  2. Kiểm tra điện trở termination 120 Ohm
  3. Kiểm tra CAN transceiver có nguồn chưa
  4. Kiểm tra Pack Pin có hoạt động (có phát CAN) không
  5. Kiểm tra baud rate CAN phải khớp (1 Mbps)

### Master không đọc được dữ liệu Modbus

- **Xử lý:**
  1. Kiểm tra địa chỉ DIP Switch (phải khớp với cấu hình Master)
  2. Kiểm tra baudrate (xem mục "Cài đặt Baudrate")
  3. Kiểm tra kết nối RS485 (A, B, GND)
  4. Kiểm tra TX Enable (PA4)

### LED_RUN không sáng khi khởi động

- **Nguyên nhân:** DIP Switch đặt địa chỉ = 0 -> vào chế độ cài đặt baudrate
- **Xử lý:** LED_RUN sẽ nhấp nháy. Đặt DIP Switch về địa chỉ mong muốn (1-31), đợi 5 giây, reset thiết bị.

### Không nhận được CAN dù đã kết nối đúng

- Kiểm tra sample point và baud rate phải khớp giữa 2 node
- Hiện tại cấu hình: **1 Mbps, sample point 77.8%**
- Chiều dài bus CAN tối đa ở 1 Mbps: khoảng **25m** (dây twisted pair có shield)
- Nếu bus dài hơn, giảm baud rate (sửa Prescaler trong CubeMX)

---

## Tham khảo

- DBC file: `04_Documents/CANdb/VMO_SinglePack_DBC_V0_1.dbc`
- Flowchart chi tiết: `VMO_RBCS_BSS_V2_Flowchart.md`
- STM32F103 Reference Manual: RM0008
- STM32 HAL CAN documentation
- Modbus RTU specification
