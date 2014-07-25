#define LOG_TAG "btif_mp_build"

#include <utils/Log.h>

#include "bluetoothmp.h"
#include "bt_mp_build.h"

int
BuildBluetoothDevice(
        BASE_INTERFACE_MODULE   *pBaseInterface,
        BT_DEVICE               **ppBtDeviceBase,
        BT_DEVICE               *pDeviceBasememory,
        void                    *pExtra,
        uint8_t                 *pTxGainTable,
        uint8_t                 *pTxDACTable
        )
{
    BT_DEVICE *pBtDevice = pDeviceBasememory;
    *ppBtDeviceBase = pDeviceBasememory;

    pBtDevice->pExtra = pExtra;
    pBtDevice->InterfaceType = pBaseInterface->InterfaceType;
    pBtDevice->pBaseInterface = pBaseInterface;

    pBtDevice->pBTInfo = &pBtDevice->BaseBTInfoMemory ;

    pBtDevice->SetTxGainTable       =       BTDevice_SetTxGainTable;
    pBtDevice->SetTxDACTable        =       BTDevice_SetTxDACTable;
    pBtDevice->GetPayloadLenTable   =       BTDevice_GetPayloadLenTable;

    //-->Register Read/Write
    pBtDevice->SetMdRegMaskBits = bt_default_SetMDRegMaskBits;
    pBtDevice->GetMdRegMaskBits = bt_default_GetMDRegMaskBits;
    pBtDevice->SetRfRegMaskBits = bt_default_SetRFRegMaskBits;;
    pBtDevice->GetRfRegMaskBits = bt_default_GetRFRegMaskBits;
    //-->HCI command raw data
    pBtDevice->SendHciCmd = bt_default_SendHCICmd;
    pBtDevice->RecvHciEvent = bt_default_RecvHCIEvent;
    //-->HCI Command & Event
    pBtDevice->SendHciCommandWithEvent = BTDevice_SendHciCommandWithEvent;
    pBtDevice->RecvAnyHciEvent = BTDevice_RecvAnyHciEvent;

    //Device member
    //-->Register Control
    pBtDevice->SetTxChannel = BTDevice_SetTxChannel;
    pBtDevice->SetLETxChannel = BTDevice_SetLETxChannel;
    pBtDevice->SetRxChannel = BTDevice_SetRxChannel;
    pBtDevice->SetPowerGainIndex = BTDevice_SetPowerGainIndex;
    pBtDevice->SetPowerGain = BTDevice_SetPowerGain;
    pBtDevice->SetPowerDac = BTDevice_SetPowerDac;
    pBtDevice->SetPayloadType = BTDevice_SetPayloadType;
    pBtDevice->SetWhiteningCoeff = BTDevice_SetWhiteningCoeff;
    pBtDevice->SetPacketType = BTDevice_SetPacketType;
    pBtDevice->SetHitTarget = BTDevice_SetHitTarget;
    pBtDevice->SetTestMode = BTDevice_SetTestMode;
    pBtDevice->SetMutiRxEnable = BTDevice_SetMutiRxEnable;
    pBtDevice->SetRestMDCount = BTDevice_SetResetMDCount;
    pBtDevice->SetPackHeader = BTDevice_SetPackHeader;
    pBtDevice->SetPesudoOuterSetup = BTDevice_SetPesudoOuterSetup;

    pBtDevice->TestModeEnable = BTDevice_TestModeEnable;
    pBtDevice->SetRtl8761Xtal = BTDevice_SetRTL8761Xtal;
    pBtDevice->GetRtl8761Xtal = BTDevice_GetRTL8761Xtal;
    pBtDevice->ReadThermal = BTDevice_ReadThermal;

    pBtDevice->GetStage = BT_GetStage;
    pBtDevice->SetSysRegMaskBits = BT_SetSysRegMaskBits;
    pBtDevice->GetSysRegMaskBits = BT_GetSysRegMaskBits;
    pBtDevice->SetBBRegMaskBits = BT_SetBBRegMaskBits;
    pBtDevice->GetBBRegMaskBits = BT_GetBBRegMaskBits;

    //-->Vendor HCI Command Control
    pBtDevice->SetFWPowerTrackEnable =      BTDevice_SetFWPowerTrackEnable;
    pBtDevice->SetHoppingMode        =      BTDevice_SetHoppingMode;
    pBtDevice->SetHciReset           =      BTDevice_SetHciReset;
    pBtDevice->GetBTClockTime        =      BTDevice_GetBTClockTime;

    pBtDevice->TxTriggerPktCnt = 0;

    //CON-TX
    pBtDevice->SetContinueTxBegin   =       BTDevice_SetContinueTxBegin;
    pBtDevice->SetContinueTxStop    =       BTDevice_SetContinueTxStop;
    pBtDevice->SetContinueTxUpdate  =       BTDevice_SetContinueTxUpdate;
    //LE Test
    pBtDevice->LeTxTestCmd          =       BTDevice_LeTxTestCmd;
    pBtDevice->LeRxTestCmd          =       BTDevice_LeRxTestCmd;
    pBtDevice->LeTestEndCmd         =       BTDevice_LeTestEndCmd;
    //PKT-TX
    pBtDevice->SetPktTxBegin        =       BTDevice_SetPktTxBegin;
    pBtDevice->SetPktTxStop         =       BTDevice_SetPktTxStop;
    pBtDevice->SetPktTxUpdate       =       BTDevice_SetPktTxUpdate;
    //PKT-RX
    pBtDevice->SetPktRxBegin        =       BTDevice_SetPktRxBegin;
    pBtDevice->SetPktRxStop         =       BTDevice_SetPktRxStop;
    pBtDevice->SetPktRxUpdate       =       BTDevice_SetPktRxUpdate;
    //Base Function
    pBtDevice->GetPayLoadTypeValidFlag =    BTBASE_GetPayLoadTypeValidFlag;
    pBtDevice->HitTargetAccessCodeGen  =    BTBASE_HitTargetAccessCodeGen;

    pBtDevice->GetChipId            =       bt_default_GetChipId;
    pBtDevice->GetECOVersion        =       bt_default_GetECOVersion;
    pBtDevice->GetChipVersionInfo   =       bt_default_GetBTChipVersionInfo;
    pBtDevice->BTDlFW               =       bt_default_BTDlFW;
    pBtDevice->BTDlMERGERFW         =       bt_default_BTDlMergerFW;
    //PG Efuse
    pBtDevice->BT_SP_PGEfuseRawData =       BTDevice_SpecialFunction_Efuse_PGEfuseRawData;
    //Table
    pBtDevice->SetTxGainTable(pBtDevice,pTxGainTable);
    pBtDevice->SetTxDACTable(pBtDevice,pTxDACTable);

    return 0;
}

// Base Module interface builder
int
BuildBluetoothModule(
        BASE_INTERFACE_MODULE   *pBaseInterfaceModule,
        BT_MODULE               *pBtModule,
        void                    *pExtra,
        uint8_t                 *pTxGainTable,
        uint8_t                 *pTxDACTable
        )
{
    int rtn = BT_FUNCTION_SUCCESS;

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

    BuildBluetoothDevice(pBaseInterfaceModule,
            &pBtModule->pBtDevice,
            &pBtModule->BaseBtDeviceMemory,
            pExtra,
            pTxGainTable,
            pTxDACTable
            );

    return  rtn;
}
