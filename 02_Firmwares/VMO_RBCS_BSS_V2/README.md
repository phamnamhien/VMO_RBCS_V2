# VMO_RBCS_BSS_V2 - Battery Station Slave Firmware

Firmware cho mạch đọc dữ liệu Pin (BMS) và điều khiển trạm sạc/thay pin.
MCU STM32F103C8T6, chạy FreeRTOS, giao tiếp CAN Bus với Pack Pin và Modbus RTU RS485 với Master Controller.

## Muc luc

- [Tong quan he thong](#tong-quan-he-thong)
- [Yeu cau phan cung](#yeu-cau-phan-cung)
- [So do chan GPIO](#so-do-chan-gpio)
- [Cai dat moi truong phat trien](#cai-dat-moi-truong-phat-trien)
- [Cau truc thu muc](#cau-truc-thu-muc)
- [Cau hinh CAN Bus](#cau-hinh-can-bus)
- [Cau hinh Modbus Slave RS485](#cau-hinh-modbus-slave-rs485)
- [Huong dan su dung thiet bi](#huong-dan-su-dung-thiet-bi)
- [Mo hinh trang thai HSM](#mo-hinh-trang-thai-hsm)
- [Bang thanh ghi du lieu](#bang-thanh-ghi-du-lieu)
- [Thu vien CAN Battery API](#thu-vien-can-battery-api)
- [Xay dung va nap firmware](#xay-dung-va-nap-firmware)
- [Xu ly su co](#xu-ly-su-co)

---

## Tong quan he thong

### Thong so ky thuat

| Thong so | Gia tri |
|----------|---------|
| MCU | STM32F103C8T6 (ARM Cortex-M3) |
| Flash | 64 KB |
| RAM | 20 KB |
| Clock | 72 MHz (HSE 8MHz + PLL x9) |
| RTOS | FreeRTOS V10 (CMSIS-RTOS V2) |
| IDE | STM32CubeIDE |
| HAL | STM32Cube FW_F1 V1.8.6 |

### Kien truc giao tiep

```
+-------------------+          CAN Bus 1Mbps           +------------------+
|                   |  <------------------------------  |                  |
|   BSS Slave       |    (Pack Pin broadcast data)      |    Pack Pin      |
|   (STM32F103)     |                                   |    (BMS)         |
|                   |                                   +------------------+
|                   |
|                   |        RS485 Modbus RTU            +------------------+
|                   |  <------------------------------>  |                  |
+-------------------+    (Slave, dia chi 1-31)          | Master Controller|
                                                        +------------------+
```

**Luong du lieu:**
1. Pack Pin (BMS) tu dong phat quang ba du lieu qua CAN Bus
2. BSS Slave nhan va luu tru du lieu Pin
3. Master Controller doc du lieu tu BSS Slave qua Modbus RTU RS485
4. Master Controller gui lenh dieu khien (sac, dung khan cap) qua Modbus

---

## Yeu cau phan cung

### Linh kien chinh
- 1x Board STM32F103C8T6 (Blue Pill hoac PCB custom)
- 1x Thach anh 8MHz
- 1x CAN Transceiver (MCP2551, TJA1050, hoac SN65HVD230)
- 1x RS485 Transceiver (MAX485, SP3485)
- 1x DIP Switch 5 bit (dat dia chi Modbus)
- 2x Limit Switch (phat hien Pin trong slot)
- 3x LED (RUN, FAULT, STATUS)
- Dien tro termination 120 Ohm cho CAN bus (2 dau bus)

### Ket noi

```
CAN Bus:   PA11 (CAN_RX) ---|CAN        |--- CANH ---> Pack Pin BMS
           PA12 (CAN_TX) ---|Transceiver |--- CANL --->
                             |            |--- GND  --->

RS485:     PA2 (USART2_TX) --|RS485      |--- A -----> Master Controller
           PA3 (USART2_RX) --|Transceiver|--- B ----->
           PA4 (TX Enable)  --|           |--- GND --->
```

---

## So do chan GPIO

### Giao tiep

| Chan | Ten | Chuc nang | Huong |
|------|-----|-----------|-------|
| PA2 | RS4851_DI | USART2 TX (RS485 COM) | Output AF |
| PA3 | RS4851_RO | USART2 RX (RS485 COM) | Input |
| PA4 | RS4851_TXEN | RS485 TX Enable | Output |
| PA11 | CAN_RX | CAN nhan du lieu tu Pack | Input |
| PA12 | CAN_TX | CAN phat du lieu | Output AF |

### Dieu khien

| Chan | Ten | Chuc nang | Huong |
|------|-----|-----------|-------|
| PA8 | CHARGE_CTRL | Dieu khien sac (HIGH = bat) | Output |
| PA10 | EMERGENCY | Dung khan cap (HIGH = kich hoat) | Output |

### DIP Switch (dia chi Modbus)

| Chan | Ten | Bit | Ghi chu |
|------|-----|-----|---------|
| PB0 | ADDR4 | bit 0 | Pull-up, LOW = 1 |
| PB1 | ADDR3 | bit 1 | Pull-up, LOW = 1 |
| PB2 | ADDR2 | bit 2 | Pull-up, LOW = 1 |
| PB3 | ADDR1 | bit 3 | Pull-up, LOW = 1 |
| PB4 | ADDR0 | bit 4 | Pull-up, LOW = 1 |

Dia chi = 0 -> mac dinh la 1. Dai dia chi: 1-31.

### LED chi thi

| Chan | Ten | Chuc nang |
|------|-----|-----------|
| PB5 | LED_STT | Nhap nhay khi nhan du lieu tu Master |
| PB6 | LED_RUN | Sang khi hoat dong binh thuong |
| PB7 | LED_FAULT | Sang khi co loi hoac mat ket noi |

### Cam bien

| Chan | Ten | Chuc nang |
|------|-----|-----------|
| PB8 | LIMIT_SWITCH0 | Phat hien Pin trong slot |
| PB9 | LIMIT_SWITCH1 | Phat hien Pin trong slot |
| PA6 | RD_VOLTAGE | Doc dien ap (Analog) |

---

## Cai dat moi truong phat trien

### Yeu cau

1. **STM32CubeIDE** >= 1.15.0 (hoac Keil/IAR voi HAL)
2. **ST-Link V2** hoac **J-Link** de nap firmware
3. **STM32CubeMX** >= 6.15.0 (neu muon sua cau hinh .ioc)

### Cac buoc

```bash
# 1. Clone repository
git clone <repository-url>
cd VMO_RBCS_V2/02_Firmwares/VMO_RBCS_BSS_V2

# 2. Mo project trong STM32CubeIDE
#    File -> Import -> Existing Projects into Workspace
#    Chon thu muc VMO_RBCS_BSS_V2

# 3. Build
#    Project -> Build Project (Ctrl+B)

# 4. Nap firmware
#    Run -> Debug As -> STM32 C/C++ Application
```

---

## Cau truc thu muc

```
VMO_RBCS_BSS_V2/
|-- Core/
|   |-- Inc/                    # Header files (main.h, can.h, usart.h, ...)
|   |-- Src/                    # Source files
|   |   |-- main.c              # Entry point, clock config, peripheral init
|   |   |-- can.c               # CAN init, bit timing, RX callback
|   |   |-- usart.c             # USART2 init (RS485 Modbus Slave)
|   |   |-- freertos.c          # FreeRTOS tasks (defaultTask, readBatteryTask, commTask)
|   |   |-- gpio.c              # GPIO init
|   |   |-- dma.c               # DMA init cho USART2
|   |   |-- tim.c               # Timer2 init (HSM timer)
|   |   |-- stm32f1xx_it.c      # Interrupt handlers
|   |   +-- ...
|   +-- Startup/
|
|-- APP/                        # Ung dung chinh
|   |-- app_states.h            # Dinh nghia struct, enum, register map
|   |-- app_states.c            # HSM state machine (run, fault, bat_not_connected)
|   +-- app_params.c            # EEPROM init, baudrate config
|
|-- CAN-BAT/                    # Thu vien CAN Battery (MOI)
|   |-- can_battery.h           # Header - API, CAN ID config, struct
|   +-- can_battery.c           # Implementation - init, filter, RX callback, data access
|
|-- MODBUS-LIB/                 # Thu vien Modbus RTU (chi con Slave)
|   |-- Modbus.h / Modbus.c
|   |-- ModbusConfig.h          # Cau hinh: MAX_M_HANDLERS=1 (Slave only)
|   +-- UARTCallback.c
|
|-- HSM/                        # Hierarchical State Machine library
|-- EE/                         # EEPROM emulation library
|-- Drivers/                    # STM32 HAL drivers
|-- Middlewares/                 # FreeRTOS middleware
|
|-- RBCS_READ_BATTERY_DATA.ioc  # STM32CubeMX project file
|-- STM32F103C8TX_FLASH.ld      # Linker script
+-- README.md                   # File nay
```

---

## Cau hinh CAN Bus

### Thong so bit timing

| Thong so | Gia tri | Ghi chu |
|----------|---------|---------|
| APB1 Clock | 36 MHz | SYSCLK/2 |
| Prescaler | 2 | |
| Time Quantum (TQ) | 55.56 ns | 36MHz / 2 = 18MHz |
| Sync Seg | 1 TQ | Co dinh |
| BS1 (Time Seg 1) | 13 TQ | |
| BS2 (Time Seg 2) | 4 TQ | |
| SJW | 2 TQ | |
| Tong bit time | 18 TQ | 1 + 13 + 4 |
| **Baud rate** | **1 Mbps** | 18MHz / 18 = 1MHz |
| **Sample point** | **77.8%** | (1+13)/18 |

### Cac tham so khac

| Thong so | Gia tri | Y nghia |
|----------|---------|---------|
| Mode | Normal | Giao tiep binh thuong |
| AutoBusOff | Enable | Tu phuc hoi khi Bus-Off |
| AutoWakeUp | Enable | Tu danh thuc |
| AutoRetransmission | Enable | Tu gui lai khi loi |

### Bo loc CAN (Filter)

- **Filter Bank 0**, che do Mask 32-bit
- Chap nhan CAN ID: `0x100` - `0x10F`
- Gan vao RX FIFO0
- Ngat `CAN_IT_RX_FIFO0_MSG_PENDING` duoc bat

### Giao thuc CAN tu Pack Pin

BMS phat quang ba du lieu qua 12 CAN frame. Moi frame chua 4 thanh ghi 16-bit (big-endian):

| CAN ID | Noi dung | Thanh ghi |
|--------|----------|-----------|
| 0x100 | Frame 0 | registers[0..3] |
| 0x101 | Frame 1 | registers[4..7] |
| 0x102 | Frame 2 | registers[8..11] |
| ... | ... | ... |
| 0x10B | Frame 11 | registers[44..47] |

**Dinh dang byte trong moi frame (8 bytes):**
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

> **Luu y:** Neu giao thuc CAN cua BMS khac (vi du DBC `VMO_SinglePack_DBC_V0_1`),
> ban can sua cac `#define` trong `CAN-BAT/can_battery.h` va ham parse trong
> `CAN_BAT_RxCallback()` (file `CAN-BAT/can_battery.c`).

---

## Cau hinh Modbus Slave RS485

### Thong so ket noi

| Thong so | Gia tri |
|----------|---------|
| UART | USART2 (PA2-TX, PA3-RX) |
| TX Enable | PA4 (RS4851_TXEN) |
| Dia chi Slave | Tu DIP Switch (1-31) |
| Baudrate | Tu EEPROM (mac dinh 115200) |
| Data format | 8-N-1 |
| Giao thuc | Modbus RTU |
| Che do | DMA |
| Timeout | 1000 ms |
| So thanh ghi | 50 (TOTAL_STA_REGISTERS) |

### Bang ma Baudrate

Ma baudrate duoc luu trong EEPROM, cai dat qua DIP Switch:

| Ma | Baudrate | Ma | Baudrate |
|----|----------|----|----------|
| 1 | 1200 | 6 | 19200 |
| 2 | 2400 | 7 | 38400 |
| 3 | 4800 | 8 | 56000 |
| 4 | 9600 | 9 | 57600 |
| 5 | 14400 | 10 | 115200 (mac dinh) |

---

## Huong dan su dung thiet bi

### 1. Ket noi phan cung

1. Cap nguon cho board (3.3V hoac 5V tuy thiet ke PCB)
2. Ket noi CAN transceiver:
   - PA11 -> CAN_RX cua transceiver
   - PA12 -> CAN_TX cua transceiver
   - CANH, CANL -> Bus CAN cua Pack Pin
   - Gan dien tro 120 Ohm o hai dau bus CAN
3. Ket noi RS485 transceiver:
   - PA2 -> DI (Data In)
   - PA3 -> RO (Receiver Out)
   - PA4 -> DE/RE (Direction Enable)
   - A, B -> Bus RS485 cua Master Controller
4. Ket noi Limit Switch vao PB8, PB9 (noi GND khi kich hoat)
5. Ket noi DIP Switch 5 bit vao PB0-PB4

### 2. Cai dat dia chi Modbus

Dung DIP Switch 5 bit (ADDR4..ADDR0) de dat dia chi:

```
Vi du: Dia chi = 5
  ADDR4 (PB0) = OFF  (bit0 = 1)
  ADDR3 (PB1) = ON   (bit1 = 0)
  ADDR2 (PB2) = OFF  (bit2 = 1)
  ADDR1 (PB3) = OFF  (bit3 = 0)
  ADDR0 (PB4) = OFF  (bit4 = 0)
  -> addr = 0b00101 = 5
```

**Luu y:** Logic dao - cong tac BAT (noi GND) = bit 1.

### 3. Cai dat Baudrate (lan dau)

1. Dat tat ca DIP Switch ve `OFF` (dia chi = 0) -> vao che do cai dat
2. LED_RUN se nhap nhay (500ms)
3. Gat DIP Switch den gia tri baudrate mong muon (1-10)
4. Giu nguyen 5 giay, LED_RUN sang lien -> da luu vao EEPROM
5. Reset thiet bi
6. Dat lai DIP Switch ve dia chi Modbus thuc te

### 4. Hoat dong binh thuong

Sau khi khoi dong voi dia chi hop le (1-31):

1. **LED_RUN sang lien** -> Thiet bi hoat dong binh thuong
2. Khi Pin duoc dat vao slot -> Limit Switch kich hoat
3. Thiet bi tu dong nhan du lieu tu Pack Pin qua CAN Bus
4. Master Controller doc du lieu qua Modbus RTU (Function Code 0x03)
5. **LED_STT nhap nhay nhanh** -> Dang giao tiep voi Master
6. Master co the gui lenh:
   - `REG_STA_CHRG_CTRL = 1` -> Bat sac
   - `REG_STA_IS_EMERGENCY_STOP = 1` -> Dung khan cap

### 5. Chi thi LED

| Trang thai | LED_RUN | LED_FAULT | LED_STT |
|------------|---------|-----------|---------|
| Cai dat baudrate | Nhap nhay 500ms | Tat | Tat |
| Binh thuong | Sang | Tat | Nhap nhay khi nhan lenh |
| Loi BMS | Tat | Sang | Tat |
| Mat ket noi Pin | Tat | Nhap nhay 500ms | Tat |

---

## Mo hinh trang thai HSM

```
                +--------------------+
                |  Cai dat Baudrate  |  (dia chi = 0)
                +--------------------+
                         |
                    dia chi != 0
                         v
                  +-------------+
           +----->|    Chay     |<------+
           |      +-------------+       |
           |         |       |          |
           |    co FAULT  timeout>=5    |
           |         v       v          |
           |   +-------+ +----------+  |
           +---|  Loi  | | Mat ket  |--+
           |   +-------+ | noi Pin |   |
           |      ^      +----------+  |
           |      |          |         |
           |      +----------+         |
           |       co FAULT            |
           +---------------------------+
              Limit Switch tat hoac
              nhan OK + khong loi
```

### Chi tiet chuyen trang thai

| Tu | Den | Dieu kien |
|----|-----|-----------|
| Chay | Loi | Nhan du lieu OK nhung co FAULT |
| Chay | Mat ket noi | CAN timeout >= 5 lan lien tiep |
| Loi | Chay | Limit Switch tat HOAC nhan OK + khong FAULT |
| Mat ket noi | Chay | Limit Switch tat HOAC nhan OK + khong FAULT |
| Mat ket noi | Loi | Nhan OK nhung co FAULT |

---

## Bang thanh ghi du lieu

### Thanh ghi Pin (Battery Registers) - Doc tu CAN

48 thanh ghi 16-bit, thu tu trong `dataBattery[]`:

| Chi so | Ten | Mo ta |
|--------|-----|-------|
| 0 | BMS_STATE | Trang thai BMS |
| 1 | CTRL_REQUEST | Yeu cau dieu khien |
| 2 | CTRL_RESPONSE | Phan hoi dieu khien |
| 3 | FET_CTRL_PIN | Chan dieu khien FET |
| 4 | FET_STATUS | Trang thai FET |
| 5 | ALARM_BITS | Bit canh bao |
| 6 | FAULTS | Ma loi |
| 7 | PACK_VOLT | Dien ap Pack |
| 8 | STACK_VOLT | Dien ap Stack |
| 9-10 | PACK_CURRENT | Dong dien Pack (High/Low) |
| 11 | ID_VOLT | Dien ap nhan dang |
| 12-17 | TEMP1/2/3 | Nhiet do 3 cam bien (High/Low) |
| 18-29 | CELL1-CELL12 | Dien ap Cell 1-12 |
| 30-32 | CELL_NONE | Du phong |
| 33 | CELL13 | Dien ap Cell 13 |
| 34-36 | SAFETY_A/B/C | Thanh ghi an toan |
| 37-38 | ACCU_INT | Tich luy (nguyen) H/L |
| 39-40 | ACCU_FRAC | Tich luy (thap phan) H/L |
| 41-42 | ACCU_TIME | Thoi gian tich luy H/L |
| 43 | PIN_PERCENT | Phan tram Pin |
| 44 | PERCENT_TARGET | Phan tram muc tieu |
| 45 | CELL_RESISTANCE | Dien tro Cell |
| 46 | SOC_PERCENT | SOC (%) |
| 47 | SOH_VALUE | SOH (%) |

### Thanh ghi Station (Modbus Slave Registers) - Master doc/ghi

50 thanh ghi, bao gom toan bo thanh ghi Pin + 4 thanh ghi bo sung:

| Chi so | Ten | Mo ta | Doc/Ghi |
|--------|-----|-------|---------|
| 0-47 | (nhu tren) | Du lieu Pin copy tu CAN | Doc |
| 48 | IS_PIN_IN_SLOT | Pin trong slot (0=Trong, 1=Day) | Doc |
| 49 | IS_PIN_TIMEOUT | CAN timeout (0=OK, 1=Timeout) | Doc |
| 50 | IS_EMERGENCY_STOP | Lenh dung khan cap | Ghi |
| 51 | CHRG_CTRL | Lenh dieu khien sac | Ghi |

---

## Thu vien CAN Battery API

File: `CAN-BAT/can_battery.h` va `CAN-BAT/can_battery.c`

### Cau hinh (sua trong can_battery.h)

```c
#define CAN_BAT_BASE_ID         0x100   // CAN ID dau tien cua BMS
#define CAN_BAT_FRAME_COUNT     12      // So frame CAN (48 regs / 4 per frame)
#define CAN_BAT_REGS_PER_FRAME  4       // So thanh ghi 16-bit moi frame
#define CAN_BAT_TX_CMD_ID       0x200   // CAN ID gui lenh den BMS
#define CAN_BAT_TIMEOUT_MS      1000    // Timeout (ms)
```

### Cac ham API

```c
// Khoi tao driver: cau hinh filter, start CAN, bat ngat
CAN_BAT_Status_t CAN_BAT_Init(CAN_BAT_Handle_t *handle, CAN_HandleTypeDef *hcan);

// Kiem tra timeout
uint8_t CAN_BAT_IsTimeout(CAN_BAT_Handle_t *handle);

// Kiem tra da nhan du du lieu chua
uint8_t CAN_BAT_IsDataReady(CAN_BAT_Handle_t *handle);

// Copy du lieu Pin ra buffer (thread-safe)
CAN_BAT_Status_t CAN_BAT_GetData(CAN_BAT_Handle_t *handle, uint16_t *dest, uint16_t count);

// Gui lenh den BMS (toi da 8 byte)
CAN_BAT_Status_t CAN_BAT_SendCommand(CAN_BAT_Handle_t *handle, uint8_t *data, uint8_t len);

// Dat task nhan notification khi du lieu san sang
void CAN_BAT_SetNotifyTask(CAN_BAT_Handle_t *handle, TaskHandle_t taskHandle);

// Goi tu HAL_CAN_RxFifo0MsgPendingCallback (da duoc goi tu can.c)
void CAN_BAT_RxCallback(CAN_BAT_Handle_t *handle);
```

### Cach hoat dong

1. **`CAN_BAT_Init()`** cau hinh filter CAN, bat CAN, bat ngat RX FIFO0
2. Khi CAN frame den -> **ISR** goi `CAN_BAT_RxCallback()` tu dong
3. Callback phan tich CAN ID, luu 4 thanh ghi 16-bit vao buffer
4. Khi nhan du 12 frame -> dat co `dataReady` va notify task dang cho
5. **`StartReadBatteryTask`** nhan notification, goi `CAN_BAT_GetData()` de copy du lieu
6. Du lieu duoc chuyen vao `dataBattery[]` -> copy sang `dataModbusSlave[]` khi Master doc

---

## Xay dung va nap firmware

### Build voi STM32CubeIDE

1. Mo project trong STM32CubeIDE
2. Chon Build Configuration: **Debug** hoac **Release**
3. `Project -> Build Project` (Ctrl+B)
4. Kiem tra output: `Debug/RBCS_READ_BATTERY_DATA.elf`

### Nap firmware

**Qua ST-Link:**
```
Run -> Debug As -> STM32 C/C++ Application
```

**Qua command line (dung STM32CubeProgrammer):**
```bash
STM32_Programmer_CLI -c port=SWD -w Debug/RBCS_READ_BATTERY_DATA.elf -v -rst
```

### Sua cau hinh CubeMX

Neu muon thay doi cau hinh ngoai vi:
1. Mo file `RBCS_READ_BATTERY_DATA.ioc` bang STM32CubeMX
2. Sua cau hinh
3. Generate Code
4. **Luu y:** Code trong vung `USER CODE BEGIN/END` se duoc giu lai

---

## Xu ly su co

### LED_FAULT sang lien (trang thai Loi)

- **Nguyen nhan:** BMS bao loi (thanh ghi FAULTS != 0)
- **Xu ly:** Kiem tra trang thai Pin, thao Pin ra va lap lai. Neu loi van con, kiem tra BMS.

### LED_FAULT nhap nhay (trang thai Mat ket noi)

- **Nguyen nhan:** Khong nhan duoc du lieu CAN tu Pack Pin trong 5 giay (5 x 1s timeout)
- **Xu ly:**
  1. Kiem tra ket noi CAN (CANH, CANL, GND)
  2. Kiem tra dien tro termination 120 Ohm
  3. Kiem tra CAN transceiver co nguon chua
  4. Kiem tra Pack Pin co hoat dong (co phat CAN) khong
  5. Kiem tra baud rate CAN phai khop (1 Mbps)

### Master khong doc duoc du lieu Modbus

- **Xu ly:**
  1. Kiem tra dia chi DIP Switch (phai khop voi cau hinh Master)
  2. Kiem tra baudrate (xem muc "Cai dat Baudrate")
  3. Kiem tra ket noi RS485 (A, B, GND)
  4. Kiem tra TX Enable (PA4)

### LED_RUN khong sang khi khoi dong

- **Nguyen nhan:** DIP Switch dat dia chi = 0 -> vao che do cai dat baudrate
- **Xu ly:** LED_RUN se nhap nhay. Dat DIP Switch ve dia chi mong muon (1-31), doi 5 giay, reset thiet bi.

### Khong nhan duoc CAN du da ket noi dung

- Kiem tra sample point va baud rate phai khop giua 2 node
- Hien tai cau hinh: **1 Mbps, sample point 77.8%**
- Chieu dai bus CAN toi da o 1 Mbps: khoang **25m** (day twisted pair co shield)
- Neu bus dai hon, giam baud rate (sua Prescaler trong CubeMX)

---

## Tham khao

- DBC file: `04_Documents/CANdb/VMO_SinglePack_DBC_V0_1.dbc`
- Flowchart chi tiet: `VMO_RBCS_BSS_V2_Flowchart.md`
- STM32F103 Reference Manual: RM0008
- STM32 HAL CAN documentation
- Modbus RTU specification
