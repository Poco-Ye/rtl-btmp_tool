#define LOG_TAG "btif_mp_build"

#include "bluetoothmp.h"
#include "bt_mp_device_efuse_base.h"
#include "bt_mp_build.h"

int
BuildBluetoothDevice(
        BASE_INTERFACE_MODULE *pBaseInterface,
        BT_DEVICE             **ppBtDeviceBase,
        BT_DEVICE             *pDeviceBasememory
        )
{
    BT_DEVICE *pBtDevice = pDeviceBasememory;
    *ppBtDeviceBase = pDeviceBasememory;

    pBtDevice->InterfaceType = pBaseInterface->InterfaceType;
    pBtDevice->pBaseInterface = pBaseInterface;

    pBtDevice->pBTInfo = &pBtDevice->BaseBTInfoMemory ;

    pBtDevice->SetTxGainTable           =   BTDevice_SetTxGainTable;
    pBtDevice->SetTxDACTable            =   BTDevice_SetTxDACTable;
    // Register Read/Write
    pBtDevice->SetMdRegMaskBits         =   BTDevice_SetMDRegMaskBits;
    pBtDevice->GetMdRegMaskBits         =   BTDevice_GetMDRegMaskBits;
    pBtDevice->SetRfRegMaskBits         =   BTDevice_SetRFRegMaskBits;
    pBtDevice->GetRfRegMaskBits         =   BTDevice_GetRFRegMaskBits;
    pBtDevice->SetSysRegMaskBits        =   BTDevice_SetSysRegMaskBits;
    pBtDevice->GetSysRegMaskBits        =   BTDevice_GetSysRegMaskBits;
    pBtDevice->SetBBRegMaskBits         =   BTDevice_SetBBRegMaskBits;
    pBtDevice->GetBBRegMaskBits         =   BTDevice_GetBBRegMaskBits;
    // HCI Command & Event
    pBtDevice->SendHciCommandWithEvent  =   BTDevice_SendHciCommandWithEvent;
    pBtDevice->RecvAnyHciEvent          =   BTDevice_RecvAnyHciEvent;
    // Register Control
    pBtDevice->SetPowerGainIndex        =   BTDevice_SetPowerGainIndex;
    pBtDevice->SetPowerGain             =   BTDevice_SetPowerGain;
    pBtDevice->SetPowerDac              =   BTDevice_SetPowerDac;
    pBtDevice->SetRestMDCount           =   BTDevice_SetResetMDCount;

    pBtDevice->TestModeEnable           =   BTDevice_TestModeEnable;
    pBtDevice->SetRtl8761Xtal           =   BTDevice_SetRTL8761Xtal;
    pBtDevice->GetRtl8761Xtal           =   BTDevice_GetRTL8761Xtal;
    pBtDevice->ReadThermal              =   BTDevice_ReadThermal;

    pBtDevice->GetStage                 =   BTDevice_GetStage;
    // Vendor HCI Command Control
    pBtDevice->SetHoppingMode           =   BTDevice_SetHoppingMode;
    pBtDevice->SetHciReset              =   BTDevice_SetHciReset;

    pBtDevice->TxTriggerPktCnt          =   0;

    //CON-TX
    pBtDevice->SetContinueTxBegin       =   BTDevice_SetContinueTxBegin;
    pBtDevice->SetContinueTxStop        =   BTDevice_SetContinueTxStop;
    pBtDevice->SetContinueTxUpdate      =   BTDevice_SetContinueTxUpdate;
    //LE Test
    pBtDevice->LeTxTestCmd              =   BTDevice_LeTxTestCmd;
    pBtDevice->LeRxTestCmd              =   BTDevice_LeRxTestCmd;
    pBtDevice->LeTestEndCmd             =   BTDevice_LeTestEndCmd;
    //PKT-TX
    pBtDevice->SetPktTxBegin            =   BTDevice_SetPktTxBegin;
    pBtDevice->SetPktTxStop             =   BTDevice_SetPktTxStop;
    pBtDevice->SetPktTxUpdate           =   BTDevice_SetPktTxUpdate;
    //PKT-RX
    pBtDevice->SetPktRxBegin            =   BTDevice_SetPktRxBegin;
    pBtDevice->SetPktRxStop             =   BTDevice_SetPktRxStop;
    pBtDevice->SetPktRxUpdate           =   BTDevice_SetPktRxUpdate;
    //Base Function
    pBtDevice->GetChipVersionInfo       =   BTDevice_GetBTChipVersionInfo;
    pBtDevice->BTDlFW                   =   BTDevice_BTDlFW;
    pBtDevice->BTDlMERGERFW             =   BTDevice_BTDlMergerFW;

    pBtDevice->BTSetConfigFileData = BTDevice_SetConfigFileData;

    //PG Logical Efuse
    pBtDevice->BT_WriteEfuseLogicalData    =   BTDevice_WriteEfuseLogicalData;
    pBtDevice->BT_ReadEfuseLogicalData     =   BTDevice_ReadEfuseLogicalData;

    pBtDevice->LeContTxCmd_8822b        =   BTDevice_8822b_LeContTxCmd;

    pBtDevice->FwPacketTxStart          =   BTDevice_fw_packet_tx_start;
    pBtDevice->FwPacketTxStop           =   BTDevice_fw_packet_tx_stop;
    pBtDevice->FwPacketTxReport         =   BTDevice_fw_packet_tx_report;

    pBtDevice->FwPacketRxStart          =   BTDevice_fw_packet_rx_start;
    pBtDevice->FwPacketRxStop           =   BTDevice_fw_packet_rx_stop;
    pBtDevice->FwPacketRxReport         =   BTDevice_fw_packet_rx_report;

    pBtDevice->FwContTxStart            =   BTDevice_fw_cont_tx_start;
    pBtDevice->FwContTxStop             =   BTDevice_fw_cont_tx_stop;
    pBtDevice->FwContTxReport           =   BTDevice_fw_cont_tx_report;

    pBtDevice->FwReadTxPowerInfo        =   BTDevice_fw_read_tx_power_info;

    pBtDevice->SetGPIO3_0               =   BTDevice_SetGpio3_0;
    pBtDevice->GetGPIO3_0               =   BTDevice_GetGpio3_0;

    pBtDevice->MpDebugMessageReport     =   BTDevice_MpDebugMessageReport;
    pBtDevice->MpFTValueReport          =   BTDevice_MpFTValueReport;

    pBtDevice->SetAntInfo = BTDevice_SetAntInfo;
    pBtDevice->SetAntDiffS0S1 = BTDevice_SetAntDiffS0S1;
    pBtDevice->TxPowerTracking = BTDevice_TxPowerTracking;
    pBtDevice->SetKTxChPwr = BTDevice_SetKTxChPwr;

    BuildEfuseLogicUnit(pBtDevice, &(pBtDevice->pSysEfuse), &(pBtDevice->SysEfuseMemory), SYS_EFUSE,  128, 128, 0, 1);
    BuildEfuseLogicUnit(pBtDevice, &(pBtDevice->pBtEfuse), &(pBtDevice->BtEfuseMemory), BT_EFUSE, 1024, 512, 1, 2);

    return 0;
}



int
BuildBluetoothModule(
        BASE_INTERFACE_MODULE *pBaseInterfaceModule,
        BT_MODULE             *pBtModule
        )
{

    pBtModule->pBtParam             =       &pBtModule->BaseBtParamMemory;
    pBtModule->pBtDevice            =       &pBtModule->BaseBtDeviceMemory;
    pBtModule->pModuleBtReport      =       &pBtModule->BaseModuleBtReportMemory;

    pBtModule->UpDataParameter      =       BTModule_UpDataParameter;
    pBtModule->ActionControlExcute  =       BTModule_ActionControlExcute;
    pBtModule->ActionReport         =       BTModule_ActionReport ;
    pBtModule->DownloadPatchCode    =       BTModule_DownloadPatchCode;

    pBtModule->SetRfRegMaskBits     =       BTModule_SetRFRegMaskBits;
    pBtModule->GetRfRegMaskBits     =       BTModule_GetRFRegMaskBits;
    pBtModule->SetMdRegMaskBits     =       BTModule_SetMDRegMaskBits;
    pBtModule->GetMdRegMaskBits     =       BTModule_GetMDRegMaskBits;

    pBtModule->SendHciCommandWithEvent  =   BTModule_SendHciCommandWithEvent;
    pBtModule->RecvAnyHciEvent      =       BTModule_RecvAnyHciEvent;

    pBtModule->SetSysRegMaskBits    =       BTModule_SetSysRegMaskBits;
    pBtModule->GetSysRegMaskBits    =       BTModule_GetSysRegMaskBits;
    pBtModule->SetBBRegMaskBits     =       BTModule_SetBBRegMaskBits;
    pBtModule->GetBBRegMaskBits     =       BTModule_GetBBRegMaskBits;
    pBtModule->SetRegMaskBits       =       BTModule_SetRegMaskBits;
    pBtModule->GetRegMaskBits       =       BTModule_GetRegMaskBits;

    BuildBluetoothDevice(
            pBaseInterfaceModule,
            &pBtModule->pBtDevice,
            &pBtModule->BaseBtDeviceMemory
            );

    return  BT_FUNCTION_SUCCESS;
}
