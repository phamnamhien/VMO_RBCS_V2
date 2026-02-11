/*
 * app_states.h
 *
 *  Created on: Dec 4, 2025
 *      Author: admin
 */

#ifndef APP_STATES_H_
#define APP_STATES_H_

#include "main.h"

#include "hsm_timer.h"
#include "hsm.h"
#include "ee.h"
#include "Modbus.h"
#include "can_battery.h"


#define MB_COM_UART	huart2

#define MB_COM_PORT	RS4851_TXEN_GPIO_Port
#define MB_COM_PIN	RS4851_TXEN_Pin

#define SETTING_WAIT_DONE_MS		5000
#define LED_STATUS_ON_MS			100



typedef enum {
	LED_OFF = GPIO_PIN_RESET,
	LED_ON = GPIO_PIN_SET,
} LED_State_t;

typedef enum {
	LS_ACTIVE = GPIO_PIN_RESET,
	LS_IDLE = GPIO_PIN_SET,
} LS_State_t;

typedef enum {
	SLOT_EMPTY = 0,
	SLOT_FULL,
} Slot_State_t;

typedef enum {
	CHRG_OFF = GPIO_PIN_RESET,
	CHRG_ON = GPIO_PIN_SET,
} Charge_Control_State_t;

typedef enum {
	EM_PASSTIVE = GPIO_PIN_RESET,
	EM_ACTIVE = GPIO_PIN_SET,
} Emergency_State_t;

typedef enum {
	HSME_LOOP = HSME_START,

	HSME_SET_INVALID_BAUD,
	HSME_SET_BAUD_CHANGE_VALUE,

	HSME_SWITCH_LIMIT_ACTIVE,
	HSME_SWITCH_LIMIT_PASSTIVE,
	HSME_BAT_RECEIVED_OK,
	HSME_BAT_RECEIVED_TIMEOUT,
	HSME_COMM_RECEIVED_OK,

	HSME_CUSTOM_TICK_UPDATE,
	HSME_TURN_OFF_LED_STT_TICK_UPDATE,
	HSME_SETTING_DONE_TICK_UPDATE,
} HSMEvent_t;

/* Battery registers mapped from VMO_SinglePack_DBC_V0_1
 * Each CAN frame = 4 x 16-bit registers (big-endian raw data)
 * 18 frames x 4 = 72 registers total
 */
typedef enum {
	/* Frame 0: 0x300 PACK_ControlSystem */
	BAT_REG_CTRL_SYS_MAX_CHG_CURRENT = 0,  // pack_controlSysMaxChgCurrent
	BAT_REG_CTRL_SYS_MAX_DSG_CURRENT,       // pack_controlSysMaxDsgCurrent
	BAT_REG_CTRL_SYS_MAX_CHG_POWER,         // pack_controlSysMaxChgPower
	BAT_REG_CTRL_SYS_MAX_DSG_POWER,         // pack_controlSysMaxDsgPower

	/* Frame 1: 0x301 PACK_InfoCharging */
	BAT_REG_CHARGING_VOLTAGE_LIMIT,          // pack_chargingVoltageLimit
	BAT_REG_CHARGING_CURRENT_LIMIT,          // pack_chargingCurrentLimit
	BAT_REG_CHARGING_STATUS,                 // FullyChargedSts + ControlMode + EndCurrent
	BAT_REG_CHARGING_DIAG,                   // TrickleCurrent + DiagCharger

	/* Frame 2: 0x303 PACK_InfoBms */
	BAT_REG_BMS_STATUS_1,                    // Ignition + Rolling + Emergency + UserMode
	BAT_REG_BMS_CURRENT_DIR,                 // CurrentDirection + DiagCellTopPriorResult
	BAT_REG_BMS_RESERVED,                    // Reserved
	BAT_REG_BMS_WAKEUP,                      // WakeupSource

	/* Frame 3: 0x304 PACK_InfoCellBalancing */
	BAT_REG_CB_STATUS_1_4,                   // CB01-CB04
	BAT_REG_CB_STATUS_5_8,                   // CB05-CB08
	BAT_REG_CB_STATUS_9_12,                  // CB09-CB12
	BAT_REG_CB_STATUS_13,                    // CB13

	/* Frame 4: 0x306 PACK_InfoDemCell */
	BAT_REG_DEM_CELL_VOLTAGE,                // OverVoltageCell/Pack + UnderVoltageCell/Pack
	BAT_REG_DEM_CELL_CURRENT,                // OverCurrentChg/Dis + OverTempChg/Dis
	BAT_REG_DEM_CELL_TEMP,                   // UnderTempChg/Dis + ImbCellChg/Rest
	BAT_REG_DEM_CELL_IMB,                    // ImbCellTemp + ImbCellSOC

	/* Frame 5: 0x309 PACK_InfoPack */
	BAT_REG_PACK_FET_STATUS,                 // CFET + DFET + LowCapAlarm + PrechargeFET
	BAT_REG_PACK_VOLTAGE_BAT,                // pack_statusPackVoltageBAT
	BAT_REG_PACK_CURRENT_AVG,                // pack_statusPackCurrentAvg
	BAT_REG_PACK_CURRENT,                    // pack_statusPackCurrent

	/* Frame 6: 0x30A PACK_InfoSox */
	BAT_REG_SOX_CAPACITY,                    // FullChargeCapacityAh + RemainingCapacityAh
	BAT_REG_SOX_RSOC_SOH,                    // RSOC + SOH
	BAT_REG_SOX_CYCLE_COUNT,                 // CycleCount
	BAT_REG_SOX_REMAINING_CHG_TIME,          // RemainingChargingTime

	/* Frame 7: 0x30B PACK_AccumDsgChgCapacity */
	BAT_REG_ACCUM_CHARGE_HIGH,               // AccumChargeCapacityAh [31:16]
	BAT_REG_ACCUM_CHARGE_LOW,                // AccumChargeCapacityAh [15:0]
	BAT_REG_ACCUM_DISCHARGE_HIGH,            // AccumDischargeCapacityAh [31:16]
	BAT_REG_ACCUM_DISCHARGE_LOW,             // AccumDischargeCapacityAh [15:0]

	/* Frame 8: 0x30E PACK_InfoContactor */
	BAT_REG_ADC_BATT_VOLT,                   // pack_statusAdcBattVolt
	BAT_REG_ADC_PACK_VOLT,                   // pack_statusAdcPackVolt
	BAT_REG_ADC_FUSE_VOLT,                   // pack_statusAdcFuseVolt
	BAT_REG_CHARGE_TIME,                     // pack_statuschargetime

	/* Frame 9: 0x310 PACK_InfoVoltageCell */
	BAT_REG_VCELL_AVG,                       // pack_statusVoltageCellAvg
	BAT_REG_VCELL_MIN,                       // pack_statusVoltageCellMin
	BAT_REG_VCELL_MIN_NO,                    // VoltageCellMinNo + VoltageCellMax[high]
	BAT_REG_VCELL_MAX,                       // VoltageCellMax[low] + VoltageCellMaxNo

	/* Frame 10: 0x311 PACK_InfoVoltageCell1 */
	BAT_REG_VCELL_01,                        // Cell01
	BAT_REG_VCELL_02,                        // Cell02
	BAT_REG_VCELL_03,                        // Cell03
	BAT_REG_VCELL_04,                        // Cell04

	/* Frame 11: 0x312 PACK_InfoVoltageCell2 */
	BAT_REG_VCELL_05,                        // Cell05
	BAT_REG_VCELL_06,                        // Cell06
	BAT_REG_VCELL_07,                        // Cell07
	BAT_REG_VCELL_08,                        // Cell08

	/* Frame 12: 0x313 PACK_InfoVoltageCell3 */
	BAT_REG_VCELL_09,                        // Cell09
	BAT_REG_VCELL_10,                        // Cell10
	BAT_REG_VCELL_11,                        // Cell11
	BAT_REG_VCELL_12,                        // Cell12

	/* Frame 13: 0x314 PACK_InfoVoltageCell4 */
	BAT_REG_VCELL_13,                        // Cell13
	BAT_REG_VCELL4_RSVD_1,
	BAT_REG_VCELL4_RSVD_2,
	BAT_REG_VCELL4_RSVD_3,

	/* Frame 14: 0x315 PACK_InfoDemBMS */
	BAT_REG_DEM_BMS_0,                       // CFET + DFET + PFET + ASICComm
	BAT_REG_DEM_BMS_1,                       // ASICShutdown + ShortCurrent + FETTemp + FuseBlow
	BAT_REG_DEM_BMS_RSVD_0,
	BAT_REG_DEM_BMS_RSVD_1,

	/* Frame 15: 0x320 PACK_InfoTemperatureCell */
	BAT_REG_TEMP_CELL_AVG_MIN,               // TempCellAvg + TempCellMin
	BAT_REG_TEMP_CELL_MIN_NO,                // TempCellMinNo + TempCellMax
	BAT_REG_TEMP_CELL_MAX_NO,                // TempCellMaxNo
	BAT_REG_TEMP_CELL_RSVD,

	/* Frame 16: 0x322 PACK_InfoTemperatureCB */
	BAT_REG_TEMP_CB1_CB2,                    // TempCB1 + TempCB2
	BAT_REG_TEMP_FET,                        // TempFET
	BAT_REG_TEMP_CB_RSVD_0,
	BAT_REG_TEMP_CB_RSVD_1,

	/* Frame 17: 0x32F PACK_InfoPackVersion */
	BAT_REG_VERSION_SW,                      // SWMajor + SWMinor + SWSubminor + BLMajor
	BAT_REG_VERSION_BL,                      // BLMinor + BLSubminor + HWMajor
	BAT_REG_VERSION_MFG_DATE,                // ManufacturerDate
	BAT_REG_VERSION_OTP,                     // OtpBq

	TOTAL_BAT_REGISTERS,                     /* = 72 */
} BatertyRegister_t;

/* Modbus Slave registers: mirrors battery CAN data + station control registers
 * Registers 0-71: CAN battery data (copied from dataBattery[])
 * Registers 72-75: Station-specific status/control
 */
typedef enum {
	/* Frame 0: 0x300 PACK_ControlSystem */
	REG_STA_CTRL_SYS_MAX_CHG_CURRENT = 0,
	REG_STA_CTRL_SYS_MAX_DSG_CURRENT,
	REG_STA_CTRL_SYS_MAX_CHG_POWER,
	REG_STA_CTRL_SYS_MAX_DSG_POWER,

	/* Frame 1: 0x301 PACK_InfoCharging */
	REG_STA_CHARGING_VOLTAGE_LIMIT,
	REG_STA_CHARGING_CURRENT_LIMIT,
	REG_STA_CHARGING_STATUS,
	REG_STA_CHARGING_DIAG,

	/* Frame 2: 0x303 PACK_InfoBms */
	REG_STA_BMS_STATUS_1,
	REG_STA_BMS_CURRENT_DIR,
	REG_STA_BMS_RESERVED,
	REG_STA_BMS_WAKEUP,

	/* Frame 3: 0x304 PACK_InfoCellBalancing */
	REG_STA_CB_STATUS_1_4,
	REG_STA_CB_STATUS_5_8,
	REG_STA_CB_STATUS_9_12,
	REG_STA_CB_STATUS_13,

	/* Frame 4: 0x306 PACK_InfoDemCell */
	REG_STA_DEM_CELL_VOLTAGE,
	REG_STA_DEM_CELL_CURRENT,
	REG_STA_DEM_CELL_TEMP,
	REG_STA_DEM_CELL_IMB,

	/* Frame 5: 0x309 PACK_InfoPack */
	REG_STA_PACK_FET_STATUS,
	REG_STA_PACK_VOLTAGE_BAT,
	REG_STA_PACK_CURRENT_AVG,
	REG_STA_PACK_CURRENT,

	/* Frame 6: 0x30A PACK_InfoSox */
	REG_STA_SOX_CAPACITY,
	REG_STA_SOX_RSOC_SOH,
	REG_STA_SOX_CYCLE_COUNT,
	REG_STA_SOX_REMAINING_CHG_TIME,

	/* Frame 7: 0x30B PACK_AccumDsgChgCapacity */
	REG_STA_ACCUM_CHARGE_HIGH,
	REG_STA_ACCUM_CHARGE_LOW,
	REG_STA_ACCUM_DISCHARGE_HIGH,
	REG_STA_ACCUM_DISCHARGE_LOW,

	/* Frame 8: 0x30E PACK_InfoContactor */
	REG_STA_ADC_BATT_VOLT,
	REG_STA_ADC_PACK_VOLT,
	REG_STA_ADC_FUSE_VOLT,
	REG_STA_CHARGE_TIME,

	/* Frame 9: 0x310 PACK_InfoVoltageCell */
	REG_STA_VCELL_AVG,
	REG_STA_VCELL_MIN,
	REG_STA_VCELL_MIN_NO,
	REG_STA_VCELL_MAX,

	/* Frame 10: 0x311 PACK_InfoVoltageCell1 */
	REG_STA_VCELL_01,
	REG_STA_VCELL_02,
	REG_STA_VCELL_03,
	REG_STA_VCELL_04,

	/* Frame 11: 0x312 PACK_InfoVoltageCell2 */
	REG_STA_VCELL_05,
	REG_STA_VCELL_06,
	REG_STA_VCELL_07,
	REG_STA_VCELL_08,

	/* Frame 12: 0x313 PACK_InfoVoltageCell3 */
	REG_STA_VCELL_09,
	REG_STA_VCELL_10,
	REG_STA_VCELL_11,
	REG_STA_VCELL_12,

	/* Frame 13: 0x314 PACK_InfoVoltageCell4 */
	REG_STA_VCELL_13,
	REG_STA_VCELL4_RSVD_1,
	REG_STA_VCELL4_RSVD_2,
	REG_STA_VCELL4_RSVD_3,

	/* Frame 14: 0x315 PACK_InfoDemBMS */
	REG_STA_DEM_BMS_0,
	REG_STA_DEM_BMS_1,
	REG_STA_DEM_BMS_RSVD_0,
	REG_STA_DEM_BMS_RSVD_1,

	/* Frame 15: 0x320 PACK_InfoTemperatureCell */
	REG_STA_TEMP_CELL_AVG_MIN,
	REG_STA_TEMP_CELL_MIN_NO,
	REG_STA_TEMP_CELL_MAX_NO,
	REG_STA_TEMP_CELL_RSVD,

	/* Frame 16: 0x322 PACK_InfoTemperatureCB */
	REG_STA_TEMP_CB1_CB2,
	REG_STA_TEMP_FET,
	REG_STA_TEMP_CB_RSVD_0,
	REG_STA_TEMP_CB_RSVD_1,

	/* Frame 17: 0x32F PACK_InfoPackVersion */
	REG_STA_VERSION_SW,
	REG_STA_VERSION_BL,
	REG_STA_VERSION_MFG_DATE,
	REG_STA_VERSION_OTP,

	/* Station-specific registers (read-only status) */
	REG_STA_IS_PIN_IN_SLOT,          // 72: 0=Empty, 1=Pin present
	REG_STA_IS_PIN_TIMEOUT,          // 73: 0=Normal, 1=CAN timeout

	/* Station-specific registers (read/write control) */
	REG_STA_IS_EMERGENCY_STOP,       // 74: 0=Normal, 1=Emergency stop
	REG_STA_CHRG_CTRL,               // 75: 0=Charge off, 1=Charge on

	TOTAL_STA_REGISTERS,             /* = 76 */
} StationRegister_t;

/* Fault detection alias - maps to PACK_InfoDemCell first register */
#define REG_STA_FAULTS REG_STA_DEM_CELL_VOLTAGE

typedef struct {
	uint8_t baudrate;
	uint16_t magic;
} Storage_t;

typedef struct {
	HSM parent;

	uint8_t modbus_address;

	/* CAN Battery - replaces Modbus Master */
	CAN_BAT_Handle_t canBatHandle;
	uint16_t dataBattery[TOTAL_BAT_REGISTERS];

	/* Modbus Slave (RS485 communication with external master) */
	modbusHandler_t handlerModbusSlave;
	uint16_t dataModbusSlave[TOTAL_STA_REGISTERS];

	Storage_t storage;
} DeviceHSM_t;




void app_states_hsm_init(DeviceHSM_t *me);

void default_parameter_init(DeviceHSM_t* hsm);

uint32_t BaudrateCodeToValue(uint8_t code);
uint8_t BaudrateValueToCode(uint32_t baudrate);
HAL_StatusTypeDef UART_Reconfigure(UART_HandleTypeDef *huart, uint32_t baudrate);
HAL_StatusTypeDef UART_ReconfigureByCode(UART_HandleTypeDef *huart, uint8_t code);



#endif /* APP_STATES_H_ */
