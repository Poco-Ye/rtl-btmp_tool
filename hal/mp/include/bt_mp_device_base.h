#ifndef __BT_MP_DEVICE_BASE_H
#define __BT_MP_DEVICE_BASE_H

#include "bt_mp_base.h"

int
BTDevice_SetMDRegMaskBits(
        BT_DEVICE *pBtDevice,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        );

int
BTDevice_GetMDRegMaskBits(
        BT_DEVICE *pBt,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        );

int
BTDevice_SetRFRegMaskBits(
        BT_DEVICE *pBt,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        );

int
BTDevice_GetRFRegMaskBits(
        BT_DEVICE *pBt,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        );

int
BTDevice_SetSysRegMaskBits(
        BT_DEVICE *pBtDevice,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        );

int
BTDevice_GetSysRegMaskBits(
        BT_DEVICE *pBtDevice,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        );

int
BTDevice_SetBBRegMaskBits(
        BT_DEVICE *pBtDevice,
        int Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        );

int
BTDevice_GetBBRegMaskBits(
        BT_DEVICE *pBtDevice,
        int Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        );

int
BTDevice_SendHciCommandWithEvent(
        BT_DEVICE *pBtDevice,
        uint16_t  OpCode,
        uint8_t PayLoadLength,
        uint8_t *pPayLoad,
        uint8_t  EventType,
        uint8_t  *pEvent,
        uint32_t *pEventLen
        );

int
BTDevice_RecvAnyHciEvent(
        BT_DEVICE *pBtDevice,
        uint8_t *pEvent
        );

int
BTDevice_SetPowerGainIndex(
        BT_DEVICE *pBtDevice,
        int Index
        );

int
BTDevice_SetPowerGain(
        BT_DEVICE *pBtDevice,
        uint8_t PowerGainValue
        );

int
BTDevice_SetPowerDac(
        BT_DEVICE *pBtDevice,
        uint8_t DacValue
        );

int
BTDevice_GetStage(
        BT_DEVICE *pBtDevice,
        uint8_t *pStage
        );

int
BTDevice_SetRTL8761Xtal(
        BT_DEVICE *pBtDevice,
        uint16_t Value
        );

int
BTDevice_GetRTL8761Xtal(
        BT_DEVICE *pBtDevice,
        uint16_t *pValue
        );

int
BTDevice_ReadThermal(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        uint8_t *pThermalValue
        );

int
BTDevice_TestModeEnable(
        BT_DEVICE *pBtDevice
        );

int
BTDevice_SetTxGainTable(
        BT_DEVICE *pBtDevice,
        uint8_t *pTable
        );

int
BTDevice_SetTxDACTable(
        BT_DEVICE *pBtDevice,
        uint8_t *pTable
        );

int
BTDevice_SetHciReset(
        BT_DEVICE *pBtDevice,
        int Delay_mSec
        );

int BTDevice_SetHoppingMode(
        BT_DEVICE *pBtDevice,
        uint8_t ChannelNumber,
        BT_PKT_TYPE PktType,
        BT_PAYLOAD_TYPE PayloadType,
        uint8_t TxGainValue,
        uint8_t WhiteningCoeffValue,
        uint8_t TxGainIndex,
        uint8_t TxDAC,
        uint8_t HoppingFixChannel
        );

int
BTDevice_SetResetMDCount(
        BT_DEVICE *pBtDevice
        );

int
BTDevice_SetContinueTxBegin(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        );

int
BTDevice_SetContinueTxStop(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        );

int
BTDevice_SetContinueTxUpdate(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        );

int
BTDevice_LeTxTestCmd(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        );

int
BTDevice_LeRxTestCmd(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        );

int
BTDevice_LeTestEndCmd(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        );

int
BTDevice_SetPktTxBegin(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        );

int
BTDevice_SetPktTxUpdate(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        );

int
BTDevice_SetPktTxStop(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        );

int
BTDevice_SetPktRxBegin(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        );

int
BTDevice_SetPktRxUpdate(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        );

int
BTDevice_SetPktRxStop(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        );

int
BTDevice_GetBTChipVersionInfo(
        BT_DEVICE *pBtDevice
        );

int
BTDevice_BTDlFW(
        BT_DEVICE *pBtDevice,
        uint8_t *pPatchcode,
        int patchLength
        );

int
BTDevice_BTDlMergerFW(
        BT_DEVICE *pBtDevice,
        uint8_t *pPatchcode,
        int patchLength
        );

int
BTDevice_PGEfuseRawData(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam
        );

int
BTDevice_WriteEfuseLogicalData(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam
        );

int
BTDevice_ReadEfuseLogicalData(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        );

#endif
