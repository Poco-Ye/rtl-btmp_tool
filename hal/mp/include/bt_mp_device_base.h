#ifndef __BT_MP_DEVICE_BASE_H
#define __BT_MP_DEVICE_BASE_H

#include "bt_mp_base.h"


//define FW Process MP Vendor Command
#define HCI_VENDOR_MP_PACKET_TX_ENABLE_CONFIG     0xFCE0
#define HCI_VENDOR_MP_PACKET_TX_STOP              0xFCE1
#define HCI_VENDOR_MP_PACKET_RX_ENABLE_CONFIG     0xFCE2
#define HCI_VENDOR_MP_PACKET_RX_STOP              0xFCE3
#define HCI_VENDOR_MP_PACKET_TX_REPORT            0xFCE4
#define HCI_VENDOR_MP_PACKET_RX_REPORT            0xFCE5
#define HCI_VENDOR_MP_ENABLE_TX_POWER_TRACKING    0xFCE8
#define HCI_VENDOR_READ_THERMAL_METER_DATA        0xFC40
#define HCI_VENDOR_MP_BLE_CONT_TX                 0xFCF6
#define HCI_VENDOR_MP_CON_TX_ENABLE_CONFIG        0xFCF1
#define HCI_VENDOR_MP_CON_TX_STOP                 0xFCF2
#define HCI_VENDOR_MP_READ_TX_POWER_INFO          0xFCED
#define HCI_VENDOR_MP_DEBUG_MESSAGE_REPORT        0xFCF7
#define HCI_VENDOR_MP_FT_VALUE_REPORT             0xFCF8


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
        BT_PKT_TYPE PacketType,
        uint8_t Index
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

int
BTDevice_8822b_LeContTxCmd(
    BT_DEVICE *pBtDevice,
    unsigned char enableLeContTx,
    unsigned char Channel,
    unsigned char TxPowerIndex
    );

int
BTDevice_fw_packet_tx_start(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

int
BTDevice_fw_packet_tx_stop(
    BT_DEVICE *pBtDevice
    );

int
BTDevice_fw_packet_tx_report(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

int
BTDevice_fw_packet_rx_start(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

int
BTDevice_fw_packet_rx_stop(
    BT_DEVICE *pBtDevice
    );

int
BTDevice_fw_packet_rx_report(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

int
BTDevice_fw_cont_tx_start(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

int
BTDevice_fw_cont_tx_stop(
    BT_DEVICE *pBtDevice
    );

int
BTDevice_fw_cont_tx_report(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

int
BTDevice_fw_read_tx_power_info(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

int
BTDevice_SetGpio3_0(
    BT_DEVICE *pBtDevice,
    unsigned char GpioValue
    );

int
BTDevice_GetGpio3_0(
    BT_DEVICE *pBtDevice,
    unsigned char *pGpioValue
    );

int
BTDevice_MpDebugMessageReport(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

int
BTDevice_MpFTValueReport(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

#endif
