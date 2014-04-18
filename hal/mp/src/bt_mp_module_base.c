#define LOG_TAG "bt_mp_module_base"

#include <utils/Log.h>

#include "bluetoothmp.h"
#include "bt_mp_module_base.h"
//------------------------------------------------------------------------------------------------------------------


int BTModule_ActionReport(
        BT_MODULE *pBtModule,
        int ActiceItem,
        BT_DEVICE_REPORT *pReport
        )
{
    int rtn = BT_FUNCTION_SUCCESS;
    BT_DEVICE *pModuleBtDevice = pBtModule->pBtDevice;
    BT_DEVICE_REPORT *pModuleBtReport = pBtModule->pModuleBtReport;
    uint32_t i;

    switch (ActiceItem) {
    case NO_THING:
        break;

    case REPORT_TX:
        pReport->TotalTXBits = pModuleBtReport->TotalTXBits;
        pReport->TotalTxCounts = pModuleBtReport->TotalTxCounts;
        break;

    case REPORT_RX:
        pReport->TotalRXBits = pModuleBtReport->TotalRXBits;
        pReport->TotalRxCounts = pModuleBtReport->TotalRxCounts;
        pReport->TotalRxErrorBits = pModuleBtReport->TotalRxErrorBits;
        pReport->ber = pModuleBtReport->ber;
        pReport->IsRxRssi = pModuleBtReport->IsRxRssi;
        pReport->RXRecvPktCnts = pModuleBtReport->RXRecvPktCnts;
        break;

    case REPORT_CHIP:
        if (pModuleBtDevice->GetChipVersionInfo(pModuleBtDevice) != BT_FUNCTION_SUCCESS) {
            rtn = FUNCTION_ERROR;
        } else {
            pReport->pBTInfo = pModuleBtDevice->pBTInfo;
        }
        break;

    case REPORT_ALL:
    default:
        pReport->TotalTXBits = pModuleBtReport->TotalTXBits;
        pReport->TotalTxCounts = pModuleBtReport->TotalTxCounts;
        pReport->TotalRXBits = pModuleBtReport->TotalRXBits;
        pReport->TotalRxCounts = pModuleBtReport->TotalRxCounts;
        pReport->TotalRxErrorBits = pModuleBtReport->TotalRxErrorBits;
        pReport->ber = pModuleBtReport->ber;
        pReport->IsRxRssi = pModuleBtReport->IsRxRssi;
        pReport->RXRecvPktCnts = pModuleBtReport->RXRecvPktCnts;
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
        for( i = 0 ; i <MAX_TXGAIN_TABLE_SIZE; i++)
        {
            pReport->CurrTXGainTable[i] = pModuleBtDevice->TXGainTable[i];
        }
        break;

    case REPORT_TX_DAC_TABLE:
        for(i = 0 ; i <MAX_TXDAC_TABLE_SIZE; i++)
        {
            pReport->CurrTXDACTable[i]= pModuleBtDevice->TXDACTable[i];
        }
        break;

    case REPORT_RTL8761_XTAL:
        rtn = pModuleBtDevice->GetRtl8761Xtal(pModuleBtDevice, &pReport->CurrRtl8761Xtal);
        break;


    case REPORT_THERMAL:
        rtn = pModuleBtDevice->ReadThermal(pModuleBtDevice, pBtModule->pBtParam, &pReport->CurrThermalValue);
        break;

    case REPORT_BT_STAGE:
        rtn = pModuleBtDevice->GetStage(pModuleBtDevice, &pReport->CurrStage);
        break;
    }

    return rtn;
}

int BTModule_UpDataParameter(
        BT_MODULE *pBtModule,
        BT_PARAMETER *pParam
        )
{
    int rtn=BT_FUNCTION_SUCCESS;
    BT_PARAMETER *pBtModuleParam = pBtModule->pBtParam;
    int n=0;
//	memcpy(pBtModuleParam,pParam,sizeof(BT_PARAMETER));

    pBtModuleParam->ParameterIndex          = pParam->ParameterIndex;
    pBtModuleParam->mTestMode               = pParam->mTestMode;
    pBtModuleParam->mChannelNumber          = pParam->mChannelNumber;
    pBtModuleParam->mPacketType             = pParam->mPacketType;
    pBtModuleParam->mTxGainIndex            = pParam->mTxGainIndex;
    pBtModuleParam->mTxGainValue            = pParam->mTxGainValue;
    pBtModuleParam->mTxPacketCount          = pParam->mTxPacketCount;
    pBtModuleParam->mPayloadType            = pParam->mPayloadType;
    pBtModuleParam->mPacketHeader           = pParam->mPacketHeader;
    pBtModuleParam->mWhiteningCoeffEnable   = pParam->mWhiteningCoeffEnable;
    pBtModuleParam->mTxDAC                  = pParam->mTxDAC;
    pBtModuleParam->mHitTarget              = pParam->mHitTarget;
    pBtModuleParam->mMutiRxEnable           = pParam->mMutiRxEnable;
    pBtModuleParam->mHoppingFixChannel      = pParam->mHoppingFixChannel;

    for (n=0;n<MAX_TXGAIN_TABLE_SIZE;n++)
    {
        pBtModuleParam->TXGainTable[n] = pParam->TXGainTable[n];
    }
    for (n=0;n<MAX_TXDAC_TABLE_SIZE;n++)
    {
        pBtModuleParam->TXDACTable[n] = pParam->TXDACTable[n];
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
    case MODULE_INIT:
        break;

    case SETTXGAINTABLE:
        rtn=pModuleBtDevice->SetTxGainTable(pModuleBtDevice,pModuleBtParam->TXGainTable);
        break;

    case SETTXDACTABLE:
        rtn=pModuleBtDevice->SetTxDACTable(pModuleBtDevice,pModuleBtParam->TXDACTable);
        break;

    case SET_TXCHANNEL:
        rtn=pModuleBtDevice->SetTxChannel(pModuleBtDevice,pModuleBtParam->mChannelNumber);
        break;

    case SET_RXCHANNEL:
        rtn=pModuleBtDevice->SetRxChannel(pModuleBtDevice,pModuleBtParam->mChannelNumber);
        break;

    case SET_POWERGAININDEX:
        rtn=pModuleBtDevice->SetPowerGainIndex(pModuleBtDevice,pModuleBtParam->mTxGainIndex);
        break;

    case SET_POWERGAIN:
        rtn=pModuleBtDevice->SetPowerGain(pModuleBtDevice,pModuleBtParam->mTxGainValue);
        break;

    case SET_POWERDAC:
        rtn=pModuleBtDevice->SetPowerDac(pModuleBtDevice,pModuleBtParam->mTxDAC);
        break;

    case SET_PAYLOADTYPE:
        rtn=pModuleBtDevice->SetPayloadType(pModuleBtDevice,pModuleBtParam->mPayloadType);
        break;

    case SET_WHITENINGCOFFENABLE:
        rtn=pModuleBtDevice->SetWhiteningCoeffEnable(pModuleBtDevice,pModuleBtParam->mWhiteningCoeffEnable);
        break;

    case SET_PACKETTYPE:
        rtn=pModuleBtDevice->SetPacketType(pModuleBtDevice,pModuleBtParam->mPacketType);
        break;

    case SET_HITTARGET:
        rtn=pModuleBtDevice->SetHitTarget(pModuleBtDevice,pModuleBtParam->mHitTarget);
        break;

    case SET_TESTMODE:
        rtn=pModuleBtDevice->SetTestMode(pModuleBtDevice,pModuleBtParam->mTestMode);
        break;

    case SET_MUTIRXENABLE:
        rtn=pModuleBtDevice->SetMutiRxEnable(pModuleBtDevice,pModuleBtParam->mMutiRxEnable);
        break;

    case HCI_RESET:
        rtn=pModuleBtDevice->SetHciReset(pModuleBtDevice,700);
        pModuleBtReport->TotalTXBits=0;
        pModuleBtReport->TXUpdateBits=0;
        pModuleBtReport->TotalTxCounts=0;
        pModuleBtReport->TXPktUpdateCnts=0;

        pModuleBtReport->TotalRXBits=0;
        pModuleBtReport->RXUpdateBits=0;
        pModuleBtReport->RXPktUpdateCnts=0;
        pModuleBtReport->TotalRxCounts=0;
        pModuleBtReport->TotalRxErrorBits=0;
        pModuleBtReport->IsRxRssi=-90;
        pModuleBtReport->RXRecvPktCnts=0;
        rtn=pModuleBtDevice->SetRestMDCount(pModuleBtDevice);
        break;

        /////////////////////////// PACKET_TX /////////////////////////////////////////////////////////
    case PACKET_TX_START:
        rtn=pModuleBtDevice->SetPktTxBegin(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        break;

    case PACKET_TX_START_SET_CHANNEL_PKTTYPE:
        rtn=pModuleBtDevice->SetPktTxBeginChannelPacketType(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        break;

    case PACKET_TX_UPDATE:
        rtn=pModuleBtDevice->SetPktTxUpdate(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        break;

    case PACKET_TX_SEND_ONE:
        rtn=pModuleBtDevice->SetPktTxSendOne(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        break;

    case PACKET_TX_STOP:
        rtn=pModuleBtDevice->SetPktTxStop(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        break;
        ////////////////////////// PACKET_RX /////////////////////////////////////////////////////////
    case PACKET_RX_START:
        rtn=pModuleBtDevice->SetPktRxBegin(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        break;

    case PACKET_RX_START_SET_CHANNEL_PKTTYPE:
        rtn=pModuleBtDevice->SetPktRxBeginChannelPacketType(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        break;

    case PACKET_RX_UPDATE:
        rtn=pModuleBtDevice->SetPktRxUpdate(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        break;

    case PACKET_RX_STOP:
        rtn=pModuleBtDevice->SetPktRxStop(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        break;

        /////////////////////////// CONTINUE_TX /////////////////////////////////////////////////////////
    case CONTINUE_TX_LE_START:
    case CONTINUE_TX_START:
        rtn=pModuleBtDevice->SetContinueTxBegin(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        break;

    case CONTINUE_TX_LE_STOP:
    case CONTINUE_TX_STOP:
        rtn=pModuleBtDevice->SetContinueTxStop(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        break;

    case CONTINUE_TX_LE_UPDATE:
    case CONTINUE_TX_UPDATE:
        rtn=pModuleBtDevice->SetContinueTxUpdate(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        break;
    /////////////////////////// HOPPING  /////////////////////////////////////////////////////////
    case HOPPING_DWELL_TIME:
        rtn = pModuleBtDevice->SetHoppingMode(pModuleBtDevice,
                pModuleBtParam->mChannelNumber,
                pModuleBtParam->mPacketType,
                pModuleBtParam->mHoppingFixChannel,
                pModuleBtParam->mWhiteningCoeffEnable
                );
        break;

    case TEST_MODE_ENABLE:
        rtn=pModuleBtDevice->TestModeEnable(pModuleBtDevice);
        break;

    case SET_PACKET_HEADER:
        rtn=pModuleBtDevice->SetPackHeader(pModuleBtDevice, pModuleBtParam->mPacketHeader);
        break;

    /////////////////////////// REPORT /////////////////////////////////////////////////////////
    case REPORT_CLEAR:
        pModuleBtReport->TotalTXBits=0;
        pModuleBtReport->TXUpdateBits=0;
        pModuleBtReport->TotalTxCounts=0;
        pModuleBtReport->TXPktUpdateCnts=0;

        pModuleBtReport->TotalRXBits=0;
        pModuleBtReport->RXUpdateBits=0;
        pModuleBtReport->RXPktUpdateCnts=0;
        pModuleBtReport->TotalRxCounts=0;
        pModuleBtReport->TotalRxErrorBits=0;
        pModuleBtReport->IsRxRssi=-90;
        pModuleBtReport->RXRecvPktCnts=0;
        rtn=pModuleBtDevice->SetRestMDCount(pModuleBtDevice);
        break;

    case SET_DEFAULT_TX_GAIN_TABLE:
        rtn=pModuleBtDevice->SetTxGainTable(pModuleBtDevice, NULL);
        break;

    case SET_DEFAULT_TX_DAC_TABLE:
        rtn=pModuleBtDevice->SetTxDACTable(pModuleBtDevice, NULL);
        break;

    case SET_RTL8761_XTAL:
        rtn=pModuleBtDevice->SetRtl8761Xtal(pModuleBtDevice, pModuleBtParam->Rtl8761Xtal);
        break;

    default:
        break;
    }

    return rtn;
}

int BTModule_DownloadPatchCode(
        BT_MODULE *pBtModule,
        unsigned char *pPatchcode,
        int patchLength,
        int Mode
        )
{
    int rtn=BT_FUNCTION_SUCCESS;

    BT_DEVICE *pModuleBtDevice = pBtModule->pBtDevice;

    if (Mode)
    {
        if (pModuleBtDevice->GetChipVersionInfo(pModuleBtDevice) != BT_FUNCTION_SUCCESS)
        {
            rtn=FUNCTION_ERROR;
            goto exit;
        }

        rtn=pModuleBtDevice->BTDlMERGERFW(pModuleBtDevice,pPatchcode,patchLength);
    }
    else
    {
        rtn=pModuleBtDevice->BTDlFW(pModuleBtDevice,pPatchcode,patchLength);
    }

exit:
    return rtn;
}

int
BTModule_RecvAnyHciEvent(
        BT_MODULE *pBtModule,
        unsigned char *pEvent
        )
{
    BT_DEVICE *pBtDevice = pBtModule->pBtDevice;

    return pBtDevice->RecvAnyHciEvent(pBtDevice,pEvent);
}

int
BTModule_SendHciCommandWithEvent(
        BT_MODULE *pBtModule,
        unsigned int OpCode,
        unsigned char PayLoadLength,
        unsigned char *pPayLoad,
        unsigned char EventType,
        unsigned char *pEvent,
        unsigned long *pEventLen
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
        unsigned char Addr,
        unsigned char Msb,
        unsigned char Lsb,
        uint32_t *pUserValue
        )
{
    BT_DEVICE *pBtDevice = pBtModule->pBtDevice;
    return pBtDevice->GetSysRegMaskBits(pBtDevice, Addr, Msb, Lsb, pUserValue);
}

int
BTModule_SetSysRegMaskBits(
        BT_MODULE *pBtModule,
        unsigned char Addr,
        unsigned char Msb,
        unsigned char Lsb,
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
        unsigned char Addr,
        unsigned char Msb,
        unsigned char Lsb,
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
        unsigned char Addr,
        unsigned char Msb,
        unsigned char Lsb,
        const uint32_t UserValue
        )
{
    BT_DEVICE *pBtDevice = pBtModule->pBtDevice;

    return pBtDevice->SetBBRegMaskBits(pBtDevice, Page, Addr, Msb, Lsb, UserValue);
}
