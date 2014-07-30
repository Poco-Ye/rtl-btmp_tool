#define LOG_TAG "bt_mp_module_base"

#include <utils/Log.h>

#include "bluetoothmp.h"
#include "bt_mp_build.h"
#include "bt_mp_module_base.h"

extern uint32_t PktRxCount;
extern uint32_t PktRxErrBits;

int BTModule_ActionReport(
        BT_MODULE *pBtModule,
        int ActiceItem,
        BT_DEVICE_REPORT *pReport
        )
{
    int rtn = BT_FUNCTION_SUCCESS;
    BT_DEVICE *pModuleBtDevice = pBtModule->pBtDevice;
    BT_PARAMETER *pModuleBtParam = pBtModule->pBtParam;
    BT_DEVICE_REPORT *pModuleBtReport = pBtModule->pModuleBtReport;
    uint32_t i;

    switch (ActiceItem) {
    case REPORT_PKT_TX:
        pModuleBtDevice->SetPktTxUpdate(pModuleBtDevice, pModuleBtParam, pModuleBtReport);
        pReport->TotalTXBits = pModuleBtReport->TotalTXBits;
        pReport->TotalTxCounts = pModuleBtReport->TotalTxCounts;
        break;

    case REPORT_CONT_TX:
        pModuleBtDevice->SetContinueTxUpdate(pModuleBtDevice, pModuleBtParam, pModuleBtReport);
        pReport->TotalTXBits = pModuleBtReport->TotalTXBits;
        pReport->TotalTxCounts = pModuleBtReport->TotalTxCounts;
        break;

    case REPORT_PKT_RX:
        pModuleBtDevice->SetPktRxUpdate(pModuleBtDevice, pModuleBtParam, pModuleBtReport);
        pReport->TotalRXBits = pModuleBtReport->TotalRXBits;
        pReport->TotalRxCounts = pModuleBtReport->TotalRxCounts;
        pReport->TotalRxErrorBits = pModuleBtReport->TotalRxErrorBits;
        pReport->ber = pModuleBtReport->ber;
        pReport->RxRssi = pModuleBtReport->RxRssi;
        pReport->RXRecvPktCnts = pModuleBtReport->RXRecvPktCnts;
        pReport->Cfo=pModuleBtReport->Cfo;
        ALOGI("BTModule_ActionReport[REPORT_RX]: RxRssi %d, TotalRXBits %u, TotalRxCounts %u, "
              "TotalRxErrorBits %u, ber %f, RXRecvPktCnts %u, Cfo %f",
              pReport->RxRssi, pReport->TotalRXBits, pReport->TotalRxCounts,
              pReport->TotalRxErrorBits, pReport->ber, pReport->RXRecvPktCnts, pReport->Cfo);
        break;

    case REPORT_CHIP:
        if (pModuleBtDevice->GetChipVersionInfo(pModuleBtDevice) != BT_FUNCTION_SUCCESS) {
            rtn = FUNCTION_ERROR;
        } else {
            pReport->pBTInfo = &(pReport->BTInfoMemory);

            pReport->pBTInfo->ChipType = pModuleBtDevice->pBTInfo->ChipType;
            pReport->pBTInfo->HCI_SubVersion = pModuleBtDevice->pBTInfo->HCI_SubVersion;
            pReport->pBTInfo->HCI_Version = pModuleBtDevice->pBTInfo->HCI_Version;
            pReport->pBTInfo->Is_After_PatchCode = pModuleBtDevice->pBTInfo->Is_After_PatchCode;
            pReport->pBTInfo->LMP_SubVersion = pModuleBtDevice->pBTInfo->LMP_SubVersion;
            pReport->pBTInfo->LMP_Version = pModuleBtDevice->pBTInfo->LMP_Version;
            pReport->pBTInfo->Version = pModuleBtDevice->pBTInfo->Version;
        }
        break;

    case REPORT_ALL:
        pReport->TotalTXBits = pModuleBtReport->TotalTXBits;
        pReport->TotalTxCounts = pModuleBtReport->TotalTxCounts;
        pReport->TotalRXBits = pModuleBtReport->TotalRXBits;
        pReport->TotalRxCounts = pModuleBtReport->TotalRxCounts;
        pReport->TotalRxErrorBits = pModuleBtReport->TotalRxErrorBits;
        pReport->ber = pModuleBtReport->ber;
        pReport->RxRssi = pModuleBtReport->RxRssi;
        pReport->RXRecvPktCnts = pModuleBtReport->RXRecvPktCnts;
        pReport->Cfo=pModuleBtReport->Cfo;
        for( i = 0 ; i <MAX_TXGAIN_TABLE_SIZE; i++)
        {
            pReport->CurrTXGainTable[i] = pModuleBtDevice->TXGainTable[i];
        }
        for(i = 0 ; i <MAX_TXDAC_TABLE_SIZE; i++)
        {
            pReport->CurrTXDACTable[i]= pModuleBtDevice->TXDACTable[i];
        }
        break;

    case REPORT_TX_GAIN_TABLE:
        for( i = 0 ; i <MAX_TXGAIN_TABLE_SIZE; i++) {
            pReport->CurrTXGainTable[i] = pModuleBtDevice->TXGainTable[i];
        }
        break;

    case REPORT_TX_DAC_TABLE:
        for(i = 0 ; i <MAX_TXDAC_TABLE_SIZE; i++) {
            pReport->CurrTXDACTable[i]= pModuleBtDevice->TXDACTable[i];
        }
        break;

    case REPORT_XTAL:
        rtn = pModuleBtDevice->GetRtl8761Xtal(pModuleBtDevice, &pReport->CurrRtl8761Xtal);
        break;

    case REPORT_THERMAL:
        rtn = pModuleBtDevice->ReadThermal(pModuleBtDevice, pBtModule->pBtParam, &pReport->CurrThermalValue);
        break;

    case REPORT_BT_STAGE:
        rtn = pModuleBtDevice->GetStage(pModuleBtDevice, &pReport->CurrStage);
        break;

    default:
        rtn = FUNCTION_ERROR;
        break;
    }

    return rtn;
}

int BTModule_UpDataParameter(
        BT_MODULE *pBtModule,
        BT_PARAMETER *pParam
        )
{
    int rtn = BT_FUNCTION_SUCCESS;
    BT_PARAMETER *pBtModuleParam = pBtModule->pBtParam;
    int n = 0;

    pBtModuleParam->ParameterIndex          = pParam->ParameterIndex;
    pBtModuleParam->mChannelNumber          = pParam->mChannelNumber;
    pBtModuleParam->mPacketType             = pParam->mPacketType;
    pBtModuleParam->mTxGainIndex            = pParam->mTxGainIndex;
    pBtModuleParam->mTxGainValue            = pParam->mTxGainValue;
    pBtModuleParam->mTxPacketCount          = pParam->mTxPacketCount;
    pBtModuleParam->mPayloadType            = pParam->mPayloadType;
    pBtModuleParam->mPacketHeader           = pParam->mPacketHeader;
    pBtModuleParam->mWhiteningCoeffValue    = pParam->mWhiteningCoeffValue;
    pBtModuleParam->mTxDAC                  = pParam->mTxDAC;
    pBtModuleParam->mHitTarget              = pParam->mHitTarget;
    pBtModuleParam->mHoppingFixChannel      = pParam->mHoppingFixChannel;
    pBtModuleParam->Rtl8761Xtal             = pParam->Rtl8761Xtal;

    for (n = 0; n < MAX_TXGAIN_TABLE_SIZE; n++)
    {
        pBtModuleParam->TXGainTable[n] = pParam->TXGainTable[n];
    }

    for (n = 0; n < MAX_TXDAC_TABLE_SIZE; n++)
    {
        pBtModuleParam->TXDACTable[n] = pParam->TXDACTable[n];
    }

    for (n = 0; n < MAX_USERAWDATA_SIZE; n++)
    {
        pBtModuleParam->mPGRawData[n] = pParam->mPGRawData[n];
    }

    for (n = 0; n < MAX_USERAWDATA_SIZE; n++)
    {
        pBtModuleParam->mParamData[n] = pParam->mParamData[n];
    }

    return rtn;
}

int BTModule_ActionControlExcute(BT_MODULE *pBtModule)
{
    int rtn = BT_FUNCTION_SUCCESS;
    int Item = pBtModule->pBtParam->ParameterIndex;
    BT_DEVICE *pModuleBtDevice = pBtModule->pBtDevice;
    BT_PARAMETER *pModuleBtParam = pBtModule->pBtParam;
    BT_DEVICE_REPORT *pModuleBtReport = pBtModule->pModuleBtReport;

    ALOGI("BTModule_ActionControlExcute: pBtModule 0x%p, pBtDevice 0x%p, pBtParam 0x%p, "
           "pModuleBtReport 0x%p, ParameterIndex %d", pBtModule, pModuleBtDevice, pModuleBtParam,
            pModuleBtReport, Item);

    switch (Item) {
    case HCI_RESET:
        rtn = pModuleBtDevice->SetHciReset(pModuleBtDevice, 700);
        pModuleBtReport->TotalTXBits = 0;
        pModuleBtReport->TotalTxCounts = 0;
        pModuleBtReport->TotalRXBits = 0;
        pModuleBtReport->TotalRxCounts = 0;
        pModuleBtReport->TotalRxErrorBits = 0;
        pModuleBtReport->RxRssi = -90;
        pModuleBtReport->RXRecvPktCnts = 0;
        PktRxCount = 0;
        PktRxErrBits = 0;
        rtn = pModuleBtDevice->SetRestMDCount(pModuleBtDevice);
        break;

    case TEST_MODE_ENABLE:
        rtn = pModuleBtDevice->TestModeEnable(pModuleBtDevice);
        break;

    case PG_EFUSE_RAWDATA:
        rtn = BTModule_ExecRawData(pBtModule, pModuleBtParam);
        break;

    case SET_TX_GAIN_TABLE:
        rtn = pModuleBtDevice->SetTxGainTable(pModuleBtDevice, pModuleBtParam->TXGainTable);
        break;

    case SET_TX_DAC_TABLE:
        rtn = pModuleBtDevice->SetTxDACTable(pModuleBtDevice, pModuleBtParam->TXDACTable);
        break;

    case SET_DEFAULT_TX_GAIN_TABLE:
        rtn = pModuleBtDevice->SetTxGainTable(pModuleBtDevice, NULL);
        break;

    case SET_DEFAULT_TX_DAC_TABLE:
        rtn = pModuleBtDevice->SetTxDACTable(pModuleBtDevice, NULL);
        break;

    case SET_POWER_GAIN_INDEX:
        rtn = pModuleBtDevice->SetPowerGainIndex(pModuleBtDevice, pModuleBtParam->mTxGainIndex);
        break;

    case SET_POWER_GAIN:
        rtn = pModuleBtDevice->SetPowerGain(pModuleBtDevice, pModuleBtParam->mTxGainValue);
        break;

    case SET_POWER_DAC:
        rtn = pModuleBtDevice->SetPowerDac(pModuleBtDevice, pModuleBtParam->mTxDAC);
        break;

    case SET_XTAL:
        rtn = pModuleBtDevice->SetRtl8761Xtal(pModuleBtDevice, pModuleBtParam->Rtl8761Xtal);
        break;

    case REPORT_CLEAR:
        pModuleBtReport->TotalTXBits = 0;
        pModuleBtReport->TotalTxCounts = 0;
        pModuleBtReport->TotalRXBits = 0;
        pModuleBtReport->TotalRxCounts = 0;
        pModuleBtReport->TotalRxErrorBits = 0;
        pModuleBtReport->RxRssi = -90;
        pModuleBtReport->RXRecvPktCnts = 0;
        PktRxCount = 0;
        PktRxErrBits = 0;

        rtn = pModuleBtDevice->SetRestMDCount(pModuleBtDevice);
        break;

    case PACKET_TX_START:
        rtn = pModuleBtDevice->SetPktTxBegin(pModuleBtDevice, pModuleBtParam, pModuleBtReport);
        break;

    case PACKET_TX_UPDATE:
        rtn = pModuleBtDevice->SetPktTxUpdate(pModuleBtDevice, pModuleBtParam, pModuleBtReport);
        break;

    case PACKET_TX_STOP:
        rtn = pModuleBtDevice->SetPktTxStop(pModuleBtDevice, pModuleBtParam, pModuleBtReport);
        break;

    case CONTINUE_TX_START:
        rtn = pModuleBtDevice->SetContinueTxBegin(pModuleBtDevice, pModuleBtParam, pModuleBtReport);
        break;

    case CONTINUE_TX_STOP:
        rtn = pModuleBtDevice->SetContinueTxStop(pModuleBtDevice, pModuleBtParam, pModuleBtReport);
        break;

    case CONTINUE_TX_UPDATE:
        rtn = pModuleBtDevice->SetContinueTxUpdate(pModuleBtDevice, pModuleBtParam, pModuleBtReport);
        break;

    case PACKET_RX_START:
        rtn = pModuleBtDevice->SetPktRxBegin(pModuleBtDevice, pModuleBtParam, pModuleBtReport);
        break;

    case PACKET_RX_UPDATE:
        rtn = pModuleBtDevice->SetPktRxUpdate(pModuleBtDevice, pModuleBtParam, pModuleBtReport);
        break;

    case PACKET_RX_STOP:
        rtn = pModuleBtDevice->SetPktRxStop(pModuleBtDevice, pModuleBtParam, pModuleBtReport);
        break;

    case HOPPING_DWELL_TIME:
        rtn = pModuleBtDevice->SetHoppingMode(pModuleBtDevice,
                pModuleBtParam->mChannelNumber,
                pModuleBtParam->mPacketType,
                pModuleBtParam->mPayloadType,
                pModuleBtParam->mTxGainValue,
                pModuleBtParam->mWhiteningCoeffValue,
                pModuleBtParam->mTxGainIndex,
                pModuleBtParam->mTxDAC,
                pModuleBtParam->mHoppingFixChannel
                );
        break;

    case LE_TX_DUT_TEST_CMD:
        rtn = pModuleBtDevice->LeTxTestCmd(pModuleBtDevice, pModuleBtParam, pModuleBtReport);
        break;

    case LE_RX_DUT_TEST_CMD:
        rtn = pModuleBtDevice->LeRxTestCmd(pModuleBtDevice, pModuleBtParam, pModuleBtReport);
        break;

    case LE_DUT_TEST_END_CMD:
        rtn = pModuleBtDevice->LeTestEndCmd(pModuleBtDevice, pModuleBtParam, pModuleBtReport);
        break;

    default:
        rtn = FUNCTION_ERROR;
        break;
    }

    return rtn;
}

int
BTModule_DownloadPatchCode(
        BT_MODULE *pBtModule,
        uint8_t *pPatchcode,
        int patchLength,
        int Mode
        )
{
    int rtn = BT_FUNCTION_SUCCESS;

    BT_DEVICE *pModuleBtDevice = pBtModule->pBtDevice;

    if (Mode)
    {
        if (pModuleBtDevice->GetChipVersionInfo(pModuleBtDevice) != BT_FUNCTION_SUCCESS)
        {
            rtn = FUNCTION_ERROR;
            goto exit;
        }

        rtn=pModuleBtDevice->BTDlMERGERFW(pModuleBtDevice, pPatchcode, patchLength);
    }
    else
    {
        rtn=pModuleBtDevice->BTDlFW(pModuleBtDevice, pPatchcode, patchLength);
    }

exit:
    return rtn;
}

int
BTModule_RecvAnyHciEvent(
        BT_MODULE *pBtModule,
        uint8_t *pEvent
        )
{
    BT_DEVICE *pBtDevice = pBtModule->pBtDevice;

    return pBtDevice->RecvAnyHciEvent(pBtDevice, pEvent);
}

int
BTModule_SendHciCommandWithEvent(
        BT_MODULE *pBtModule,
        uint16_t OpCode,
        uint8_t PayLoadLength,
        uint8_t *pPayLoad,
        uint8_t EventType,
        uint8_t *pEvent,
        uint32_t *pEventLen
        )
{
    BT_DEVICE *pBtDevice = pBtModule->pBtDevice;

    return pBtDevice->SendHciCommandWithEvent(pBtDevice,OpCode,PayLoadLength,pPayLoad,EventType,pEvent, pEventLen);
}

int
BTModule_GetMDRegMaskBits(
        BT_MODULE *pBtModule,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        )
{
    BT_DEVICE *pBtDevice = pBtModule->pBtDevice;
    return pBtDevice->GetMdRegMaskBits(pBtDevice,Addr,Msb,Lsb,pUserValue);
}

int
BTModule_SetMDRegMaskBits(
        BT_MODULE *pBtModule,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        const uint16_t UserValue
        )
{
    BT_DEVICE *pBtDevice = pBtModule->pBtDevice;
    return pBtDevice->SetMdRegMaskBits(pBtDevice,Addr,Msb,Lsb,UserValue);
}

int
BTModule_GetRFRegMaskBits(
        BT_MODULE *pBtModule,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        )
{
    BT_DEVICE *pBtDevice = pBtModule->pBtDevice;
    return pBtDevice->GetRfRegMaskBits(pBtDevice,Addr,Msb,Lsb,pUserValue);
}

int
BTModule_SetRFRegMaskBits(
        BT_MODULE *pBtModule,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        const uint16_t UserValue
        )
{
    BT_DEVICE *pBtDevice = pBtModule->pBtDevice;

    return pBtDevice->SetRfRegMaskBits(pBtDevice,Addr,Msb,Lsb,UserValue);
}

int
BTModule_GetSysRegMaskBits(
        BT_MODULE *pBtModule,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint32_t *pUserValue
        )
{
    BT_DEVICE *pBtDevice = pBtModule->pBtDevice;
    return pBtDevice->GetSysRegMaskBits(pBtDevice, Addr, Msb, Lsb, pUserValue);
}

int
BTModule_SetSysRegMaskBits(
        BT_MODULE *pBtModule,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        const uint32_t UserValue
        )
{
    BT_DEVICE *pBtDevice = pBtModule->pBtDevice;

    return pBtDevice->SetSysRegMaskBits(pBtDevice, Addr, Msb, Lsb, UserValue);
}

int
BTModule_GetBBRegMaskBits(
        BT_MODULE *pBtModule,
        uint8_t Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint32_t *pUserValue
        )
{
    BT_DEVICE *pBtDevice = pBtModule->pBtDevice;
    return pBtDevice->GetBBRegMaskBits(pBtDevice, Page, Addr, Msb, Lsb, pUserValue);
}

int
BTModule_SetBBRegMaskBits(
        BT_MODULE *pBtModule,
        uint8_t Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint32_t UserValue
        )
{
    BT_DEVICE *pBtDevice = pBtModule->pBtDevice;

    return pBtDevice->SetBBRegMaskBits(pBtDevice, Page, Addr, Msb, Lsb, UserValue);
}

int
BTModule_GetRegMaskBits(
        BT_MODULE *pBtModule,
        uint8_t Type,
        uint8_t Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint32_t *pUserValue
        )
{
    BT_DEVICE *pBtDevice = pBtModule->pBtDevice;

    switch (Type) {
    case MD_REG:
        return pBtDevice->GetMdRegMaskBits(pBtDevice, Addr, Msb, Lsb, (uint16_t *)pUserValue);

    case RF_REG:
        return pBtDevice->GetRfRegMaskBits(pBtDevice, Addr, Msb, Lsb, (uint16_t *)pUserValue);

    case SYS_REG:
        return pBtDevice->GetSysRegMaskBits(pBtDevice, Addr, Msb, Lsb, pUserValue);

    case BB_REG:
        return pBtDevice->GetBBRegMaskBits(pBtDevice, Page, Addr, Msb, Lsb, pUserValue);

    default:
        return FUNCTION_PARAMETER_ERROR;
    }

    return BT_FUNCTION_SUCCESS;
}

int
BTModule_SetRegMaskBits(
        BT_MODULE *pBtModule,
        uint8_t Type,
        uint8_t Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        const uint32_t UserValue
        )
{
    BT_DEVICE *pBtDevice = pBtModule->pBtDevice;

    switch (Type) {
    case MD_REG:
        return pBtDevice->SetMdRegMaskBits(pBtDevice, Addr, Msb, Lsb, UserValue);

    case RF_REG:
        return pBtDevice->SetRfRegMaskBits(pBtDevice, Addr, Msb, Lsb, UserValue);

    case SYS_REG:
        return pBtDevice->SetSysRegMaskBits(pBtDevice, Addr, Msb, Lsb, UserValue);

    case BB_REG:
        return pBtDevice->SetBBRegMaskBits(pBtDevice, Page, Addr, Msb, Lsb, UserValue);

    default:
        return FUNCTION_PARAMETER_ERROR;
    }

    return BT_FUNCTION_SUCCESS;
}

int
BTModule_ExecRawData(
        BT_MODULE *pBtModule,
        BT_PARAMETER *pParam
        )
{
    int rtn = BT_FUNCTION_SUCCESS;
    BT_DEVICE *pModuleBtDevice = pBtModule->pBtDevice;
    uint8_t Command = pParam->mPGRawData[0];

    switch (Command) {
    case PG_BTMAP_RAWDATA:
        rtn = pModuleBtDevice->BT_PGEfuseRawData(pModuleBtDevice,BT_EFUSE,&pParam->mPGRawData[2],pParam->mPGRawData[1]);
        break;
    case PG_SYAMAP_RAWDATA:
        rtn = pModuleBtDevice->BT_PGEfuseRawData(pModuleBtDevice,SYS_EFUSE,&pParam->mPGRawData[2],pParam->mPGRawData[1]);
        break;
    case EXEC_NOTTNING:
    default:
        break;
    }

    return rtn;
}
