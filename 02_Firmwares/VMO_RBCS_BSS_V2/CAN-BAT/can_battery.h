/*
 * can_battery.h
 *
 *  Created on: Feb 10, 2026
 *  Description: CAN Bus Battery Pack Communication Driver
 *               Replaces Modbus RTU over RS485 for reading BMS data
 *
 *  CAN Protocol:
 *    - Baud rate: 1 Mbps
 *    - BMS broadcasts data in multiple CAN frames
 *    - Base RX ID: CAN_BAT_BASE_ID (0x100)
 *    - Each frame carries 4 x 16-bit registers (big-endian)
 *    - Frame 0x100: registers[0..3], 0x101: registers[4..7], ...
 *    - Total: 12 frames for 48 registers
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

/* ---- CAN ID Configuration ---- */
#define CAN_BAT_BASE_ID         0x100   /* Base CAN ID for BMS data frames */
#define CAN_BAT_FRAME_COUNT     12      /* Number of RX frames (48 regs / 4 per frame) */
#define CAN_BAT_REGS_PER_FRAME  4       /* 16-bit registers per CAN frame (8 bytes / 2) */
#define CAN_BAT_TX_CMD_ID       0x200   /* CAN ID for sending commands to BMS */

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
    volatile uint16_t rxFrameMask;

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
