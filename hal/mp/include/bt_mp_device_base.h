#ifndef __BT_MP_DEVICE_BASE_H
#define __BT_MP_DEVICE_BASE_H

#include "bt_mp_base.h"

int
BT_GetStage(
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
BTBASE_HitTargetAccessCodeGen(
        BT_DEVICE *pBtDevice,
        uint64_t HitTarget,
        unsigned long *pAccessCode
        );

int
BTBASE_GetPayLoadTypeValidFlag(
        BT_DEVICE *pBtDevice,
        BT_TEST_MODE TestMode,
        BT_PKT_TYPE PKT_TYPE,
        unsigned int *ValidFlag
        );

//Device BASE Function
int
BTDevice_SetMutiRxEnable(
        BT_DEVICE *pBtDevice,
        int IsMultiPktRx
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
BTDevice_SetWhiteningCoeff(
        BT_DEVICE *pBtDevice,
        uint8_t WhiteningCoeffValue
        );

int
BTDevice_SetTxChannel(
        BT_DEVICE *pBtDevice,
        uint8_t ChannelNumber
        );

int
BTDevice_SetPacketType(
        BT_DEVICE *pBtDevice,
        BT_PKT_TYPE PktType
        );

int
BTDevice_SetFWPowerTrackEnable(
        BT_DEVICE *pBtDevice,
        uint8_t FWPowerTrackEnable
        );

int
BTDevice_SetHitTarget(
        BT_DEVICE *pBtDevice,
        uint64_t HitTarget
        );

int
BTDevice_SetHciReset(
        BT_DEVICE *pBtDevice,
        int Delay_mSec
        );

int
BTDevice_GetBTClockTime(
        BT_DEVICE *pBtDevice,
        unsigned long *btClockTime
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
BTDevice_SetLETxChannel(
        BT_DEVICE *pBtDevice,
        uint8_t ChannelNumber
        );

int
BTDevice_SetPacketHeader(
        BT_DEVICE *pBtDevice,
        uint32_t pktHeader
        );

int
BTDevice_CalculatedTxBits(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport,
        int pktTx_conTx,
        uint32_t *txbits,
        uint32_t *txpkt_cnt
        );

int
BTDevice_RecvAnyHciEvent(
        BT_DEVICE *pBtDevice,
        uint8_t *pEvent
        );

int
BTDevice_SendHciCommandWithEvent(
        BT_DEVICE *pBtDevice,
        uint16_t OpCode,
        uint8_t PayLoadLength,
        uint8_t *pPayLoad,
        uint8_t EventType,
        uint8_t *pEvent,
        uint32_t *pEventLen
        );

int
BTDevice_GetPayloadLenTable(
        BT_DEVICE *pBtDevice,
        uint8_t *pTable,
        int length
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
BTDevice_SetPktTxBegin_Channel_PacketType(
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
BTDevice_SetPktTxSendOne(
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
BTDevice_SetPktRxBegin_Channel_PacketType(
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

#endif
