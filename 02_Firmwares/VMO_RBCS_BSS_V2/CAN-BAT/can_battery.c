/*
 * can_battery.c
 *
 *  Created on: Feb 10, 2026
 *  Description: CAN Bus Battery Pack Communication Driver
 */

#include "can_battery.h"
#include <string.h>


/* All-frames-received mask: bits 0..17 = 0x3FFFF */
#define CAN_BAT_ALL_FRAMES_MASK  ((1UL << CAN_BAT_FRAME_COUNT) - 1UL)

/* Lookup: CAN ID -> frame index (0-17), or 0xFF if invalid */
static uint8_t CAN_BAT_GetFrameIndex(uint32_t canId)
{
    switch (canId) {
        case 0x300: return CAN_BAT_FRM_CONTROL_SYS;
        case 0x301: return CAN_BAT_FRM_CHARGING;
        case 0x303: return CAN_BAT_FRM_BMS;
        case 0x304: return CAN_BAT_FRM_CELL_BALANCING;
        case 0x306: return CAN_BAT_FRM_DEM_CELL;
        case 0x309: return CAN_BAT_FRM_PACK;
        case 0x30A: return CAN_BAT_FRM_SOX;
        case 0x30B: return CAN_BAT_FRM_ACCUM;
        case 0x30E: return CAN_BAT_FRM_CONTACTOR;
        case 0x310: return CAN_BAT_FRM_VCELL_INFO;
        case 0x311: return CAN_BAT_FRM_VCELL1;
        case 0x312: return CAN_BAT_FRM_VCELL2;
        case 0x313: return CAN_BAT_FRM_VCELL3;
        case 0x314: return CAN_BAT_FRM_VCELL4;
        case 0x315: return CAN_BAT_FRM_DEM_BMS;
        case 0x320: return CAN_BAT_FRM_TEMP_CELL;
        case 0x322: return CAN_BAT_FRM_TEMP_CB;
        case 0x32F: return CAN_BAT_FRM_VERSION;
        default:    return 0xFF;
    }
}


CAN_BAT_Status_t CAN_BAT_Init(CAN_BAT_Handle_t *handle, CAN_HandleTypeDef *hcan)
{
    if (handle == NULL || hcan == NULL) {
        return CAN_BAT_ERROR;
    }

    handle->hcan = hcan;
    handle->rxFrameMask = 0;
    handle->lastRxTick = 0;
    handle->dataReady = 0;
    handle->errorCount = 0;
    handle->notifyTaskHandle = NULL;
    memset((void *)handle->rxData, 0, sizeof(handle->rxData));

    /* ---- Configure CAN Filter ----
     * Accept CAN IDs from 0x300 to 0x33F (covers all DBC IDs: 0x300-0x32F)
     * 32-bit Mask mode:
     *   - ID:   0x300
     *   - Mask: 0x7C0 (bits[10:6] must match, accepts 0x300-0x33F)
     * Invalid IDs within this range are filtered in RxCallback via lookup table
     */
    CAN_FilterTypeDef filter;
    filter.FilterBank = 0;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;

    /* For Standard ID in 32-bit filter:
     * FilterIdHigh = StdId << 5
     * FilterMaskIdHigh = Mask << 5 (bits that must match)
     */
    filter.FilterIdHigh = (CAN_BAT_BASE_ID << 5);
    filter.FilterIdLow = 0x0000;
    /* Mask: 0x7C0 means bits[10:6] must match = 0b0110000xxxx, accepts 0x300-0x33F */
    filter.FilterMaskIdHigh = (0x7C0U << 5);
    filter.FilterMaskIdLow = 0x0006;  /* IDE=0 (Std), RTR=0 (Data) must match */
    filter.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter.FilterActivation = ENABLE;
    filter.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(hcan, &filter) != HAL_OK) {
        return CAN_BAT_ERROR;
    }

    /* Start CAN */
    if (HAL_CAN_Start(hcan) != HAL_OK) {
        return CAN_BAT_ERROR;
    }

    /* Enable RX FIFO0 message pending interrupt */
    if (HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        return CAN_BAT_ERROR;
    }

    return CAN_BAT_OK;
}


uint8_t CAN_BAT_IsTimeout(CAN_BAT_Handle_t *handle)
{
    if (handle == NULL) return 1;

    uint32_t now = xTaskGetTickCount();
    if (handle->lastRxTick == 0) {
        /* Never received anything */
        return 1;
    }
    if ((now - handle->lastRxTick) > pdMS_TO_TICKS(CAN_BAT_TIMEOUT_MS)) {
        return 1;
    }
    return 0;
}


uint8_t CAN_BAT_IsDataReady(CAN_BAT_Handle_t *handle)
{
    if (handle == NULL) return 0;
    return handle->dataReady;
}


CAN_BAT_Status_t CAN_BAT_GetData(CAN_BAT_Handle_t *handle, uint16_t *dest, uint16_t count)
{
    if (handle == NULL || dest == NULL) {
        return CAN_BAT_ERROR;
    }

    uint16_t maxRegs = CAN_BAT_FRAME_COUNT * CAN_BAT_REGS_PER_FRAME;
    if (count > maxRegs) {
        count = maxRegs;
    }

    taskENTER_CRITICAL();
    memcpy(dest, (const void *)handle->rxData, count * sizeof(uint16_t));
    taskEXIT_CRITICAL();

    return CAN_BAT_OK;
}


CAN_BAT_Status_t CAN_BAT_SendCommand(CAN_BAT_Handle_t *handle, uint8_t *data, uint8_t len)
{
    if (handle == NULL || data == NULL || len == 0 || len > 8) {
        return CAN_BAT_ERROR;
    }

    CAN_TxHeaderTypeDef txHeader;
    uint32_t txMailbox;

    txHeader.StdId = CAN_BAT_TX_CMD_ID;
    txHeader.ExtId = 0;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.DLC = len;
    txHeader.TransmitGlobalTime = DISABLE;

    if (HAL_CAN_AddTxMessage(handle->hcan, &txHeader, data, &txMailbox) != HAL_OK) {
        return CAN_BAT_BUSY;
    }

    return CAN_BAT_OK;
}


void CAN_BAT_SetNotifyTask(CAN_BAT_Handle_t *handle, TaskHandle_t taskHandle)
{
    if (handle != NULL) {
        handle->notifyTaskHandle = taskHandle;
    }
}


void CAN_BAT_RxCallback(CAN_BAT_Handle_t *handle)
{
    if (handle == NULL || handle->hcan == NULL) return;

    CAN_RxHeaderTypeDef rxHeader;
    uint8_t rxPayload[8];

    if (HAL_CAN_GetRxMessage(handle->hcan, CAN_RX_FIFO0, &rxHeader, rxPayload) != HAL_OK) {
        handle->errorCount++;
        return;
    }

    /* Only process standard data frames */
    if (rxHeader.IDE != CAN_ID_STD || rxHeader.RTR != CAN_RTR_DATA) {
        return;
    }

    /* Calculate frame index from CAN ID via lookup table */
    uint8_t frameIndex = CAN_BAT_GetFrameIndex(rxHeader.StdId);
    if (frameIndex >= CAN_BAT_FRAME_COUNT) {
        return;  /* Unknown CAN ID, ignore */
    }

    /* Parse 4 x 16-bit registers from 8-byte payload (big-endian) */
    uint16_t regOffset = frameIndex * CAN_BAT_REGS_PER_FRAME;
    uint16_t maxRegs = CAN_BAT_FRAME_COUNT * CAN_BAT_REGS_PER_FRAME;

    for (uint8_t i = 0; i < CAN_BAT_REGS_PER_FRAME; i++) {
        uint16_t regIdx = regOffset + i;
        if (regIdx < maxRegs && (i * 2 + 1) < rxHeader.DLC) {
            handle->rxData[regIdx] = ((uint16_t)rxPayload[i * 2] << 8) | rxPayload[i * 2 + 1];
        }
    }

    /* Update frame received bitmask */
    handle->rxFrameMask |= (1UL << frameIndex);
    handle->lastRxTick = xTaskGetTickCountFromISR();

    /* Check if all frames received */
    if ((handle->rxFrameMask & CAN_BAT_ALL_FRAMES_MASK) == CAN_BAT_ALL_FRAMES_MASK) {
        handle->dataReady = 1;
        handle->rxFrameMask = 0;  /* Reset for next cycle */

        /* Notify waiting task */
        if (handle->notifyTaskHandle != NULL) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            vTaskNotifyGiveFromISR(handle->notifyTaskHandle, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}
