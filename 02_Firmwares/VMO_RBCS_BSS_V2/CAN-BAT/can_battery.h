/*
 * can_battery.h
 *
 *  Created on: Feb 10, 2026
 *  Description: CAN Bus Battery Pack Communication Driver
 *               Replaces Modbus RTU over RS485 for reading BMS data
 *
 *  CAN Protocol (VMO_SinglePack_DBC_V0_1):
 *    - Baud rate: 1 Mbps
 *    - BMS broadcasts data in 18 CAN frames (non-contiguous IDs)
 *    - CAN IDs: 0x300,0x301,0x303,0x304,0x306,0x309-0x30B,0x30E,0x310-0x315,0x320,0x322,0x32F
 *    - Each frame carries 4 x 16-bit registers (big-endian)
 *    - Total: 18 frames for 72 registers
 *
 *    - TX commands use CAN_BAT_TX_CMD_ID (0x200)
 */

#ifndef CAN_BAT_CAN_BATTERY_H_
#define CAN_BAT_CAN_BATTERY_H_

#include "main.h"
#include "can.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* ---- CAN ID Configuration (VMO_SinglePack_DBC_V0_1) ---- */
#define CAN_BAT_BASE_ID         0x300   /* Lowest CAN ID in DBC (PACK_ControlSystem) */
#define CAN_BAT_FRAME_COUNT     18      /* Number of distinct CAN messages */
#define CAN_BAT_REGS_PER_FRAME  4       /* 16-bit registers per CAN frame (8 bytes / 2) */
#define CAN_BAT_TX_CMD_ID       0x200   /* CAN ID for sending commands to BMS */

/* CAN message frame indices */
#define CAN_BAT_FRM_CONTROL_SYS       0   /* 0x300 PACK_ControlSystem */
#define CAN_BAT_FRM_CHARGING           1   /* 0x301 PACK_InfoCharging */
#define CAN_BAT_FRM_BMS                2   /* 0x303 PACK_InfoBms */
#define CAN_BAT_FRM_CELL_BALANCING     3   /* 0x304 PACK_InfoCellBalancing */
#define CAN_BAT_FRM_DEM_CELL           4   /* 0x306 PACK_InfoDemCell */
#define CAN_BAT_FRM_PACK               5   /* 0x309 PACK_InfoPack */
#define CAN_BAT_FRM_SOX                6   /* 0x30A PACK_InfoSox */
#define CAN_BAT_FRM_ACCUM              7   /* 0x30B PACK_AccumDsgChgCapacity */
#define CAN_BAT_FRM_CONTACTOR          8   /* 0x30E PACK_InfoContactor */
#define CAN_BAT_FRM_VCELL_INFO         9   /* 0x310 PACK_InfoVoltageCell */
#define CAN_BAT_FRM_VCELL1            10   /* 0x311 PACK_InfoVoltageCell1 */
#define CAN_BAT_FRM_VCELL2            11   /* 0x312 PACK_InfoVoltageCell2 */
#define CAN_BAT_FRM_VCELL3            12   /* 0x313 PACK_InfoVoltageCell3 */
#define CAN_BAT_FRM_VCELL4            13   /* 0x314 PACK_InfoVoltageCell4 */
#define CAN_BAT_FRM_DEM_BMS           14   /* 0x315 PACK_InfoDemBMS */
#define CAN_BAT_FRM_TEMP_CELL         15   /* 0x320 PACK_InfoTemperatureCell */
#define CAN_BAT_FRM_TEMP_CB           16   /* 0x322 PACK_InfoTemperatureCB */
#define CAN_BAT_FRM_VERSION           17   /* 0x32F PACK_InfoPackVersion */

/* ---- Timeout ---- */
#define CAN_BAT_TIMEOUT_MS      1000    /* Max time (ms) without receiving any frame */

/* ---- Status codes ---- */
typedef enum {
    CAN_BAT_OK = 0,
    CAN_BAT_TIMEOUT,
    CAN_BAT_ERROR,
    CAN_BAT_BUSY,
} CAN_BAT_Status_t;

/* ---- CAN Battery Handle ---- */
typedef struct {
    CAN_HandleTypeDef *hcan;

    /* RX data buffer - stores all battery registers */
    volatile uint16_t rxData[CAN_BAT_FRAME_COUNT * CAN_BAT_REGS_PER_FRAME];

    /* Bitmask tracking which frames have been received in current cycle */
    volatile uint32_t rxFrameMask;

    /* Timestamp of last received frame (tick count) */
    volatile uint32_t lastRxTick;

    /* Flag: all frames received at least once */
    volatile uint8_t dataReady;

    /* Error counter */
    volatile uint16_t errorCount;

    /* Task handle for notification */
    TaskHandle_t notifyTaskHandle;
} CAN_BAT_Handle_t;


/* ---- Public API ---- */

/**
 * @brief  Initialize CAN battery driver
 *         Configures filters, starts CAN, enables RX interrupt
 * @param  handle: Pointer to CAN_BAT_Handle_t
 * @param  hcan: Pointer to HAL CAN handle (already initialized by MX_CAN_Init)
 * @retval CAN_BAT_Status_t
 */
CAN_BAT_Status_t CAN_BAT_Init(CAN_BAT_Handle_t *handle, CAN_HandleTypeDef *hcan);

/**
 * @brief  Check if battery data has timed out
 * @param  handle: Pointer to CAN_BAT_Handle_t
 * @retval 1 if timed out, 0 if data is fresh
 */
uint8_t CAN_BAT_IsTimeout(CAN_BAT_Handle_t *handle);

/**
 * @brief  Check if a complete set of data has been received
 * @param  handle: Pointer to CAN_BAT_Handle_t
 * @retval 1 if all frames received, 0 otherwise
 */
uint8_t CAN_BAT_IsDataReady(CAN_BAT_Handle_t *handle);

/**
 * @brief  Copy battery data to destination buffer (thread-safe)
 * @param  handle: Pointer to CAN_BAT_Handle_t
 * @param  dest: Destination buffer (must be at least TOTAL_REGS * sizeof(uint16_t))
 * @param  count: Number of 16-bit registers to copy
 * @retval CAN_BAT_Status_t
 */
CAN_BAT_Status_t CAN_BAT_GetData(CAN_BAT_Handle_t *handle, uint16_t *dest, uint16_t count);

/**
 * @brief  Send a command to BMS via CAN
 * @param  handle: Pointer to CAN_BAT_Handle_t
 * @param  data: Pointer to data bytes (max 8 bytes)
 * @param  len: Number of bytes to send (1-8)
 * @retval CAN_BAT_Status_t
 */
CAN_BAT_Status_t CAN_BAT_SendCommand(CAN_BAT_Handle_t *handle, uint8_t *data, uint8_t len);

/**
 * @brief  Set the task handle that will receive notification on data ready
 * @param  handle: Pointer to CAN_BAT_Handle_t
 * @param  taskHandle: FreeRTOS task handle
 */
void CAN_BAT_SetNotifyTask(CAN_BAT_Handle_t *handle, TaskHandle_t taskHandle);

/**
 * @brief  CAN RX callback - call this from HAL_CAN_RxFifo0MsgPendingCallback
 * @param  handle: Pointer to CAN_BAT_Handle_t
 */
void CAN_BAT_RxCallback(CAN_BAT_Handle_t *handle);


#endif /* CAN_BAT_CAN_BATTERY_H_ */
