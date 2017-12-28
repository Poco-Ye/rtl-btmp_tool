#define LOG_TAG "bt_mp_module_base"

#include "bt_syslog.h"
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
    BT_DEVICE *pModuleBtDevice = pBtModule->pBtDevice;
    BT_PARAMETER *pModuleBtParam = pBtModule->pBtParam;
    BT_DEVICE_REPORT *pModuleBtReport = pBtModule->pModuleBtReport;
    unsigned int ChipType;
    int i;

    ChipType = pModuleBtDevice->pBTInfo->ChipType;

    switch (ActiceItem)
    {
    case REPORT_PKT_TX:
        if(ChipType <RTK_BT_CHIP_ID_RTL8822B)
        {
            pModuleBtDevice->SetPktTxUpdate(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        else
        {
            pModuleBtDevice->FwPacketTxReport(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        pReport->TotalTXBits = pModuleBtReport->TotalTXBits;
        pReport->TotalTxCounts = pModuleBtReport->TotalTxCounts;
        break;

    case REPORT_CON_TX:
        if(ChipType <RTK_BT_CHIP_ID_RTL8822B)
        {
            pModuleBtDevice->SetContinueTxUpdate(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        else
        {
            pModuleBtDevice->FwContTxReport(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }

        pReport->TotalTXBits = pModuleBtReport->TotalTXBits;
        pReport->TotalTxCounts = pModuleBtReport->TotalTxCounts;
        break;

    case REPORT_RKT_RX:
        if(ChipType <RTK_BT_CHIP_ID_RTL8822B)
        {
            pModuleBtDevice->SetPktRxUpdate(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        else
        {
            pModuleBtDevice->FwPacketRxReport(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        pReport->TotalRXBits = pModuleBtReport->TotalRXBits;
        pReport->TotalRxCounts = pModuleBtReport->TotalRxCounts;
        pReport->TotalRxErrorBits = pModuleBtReport->TotalRxErrorBits;
        pReport->ber = pModuleBtReport->ber;
        pReport->RxRssi = pModuleBtReport->RxRssi;
        pReport->RXRecvPktCnts = pModuleBtReport->RXRecvPktCnts;
        pReport->Cfo=pModuleBtReport->Cfo;
        SYSLOGI("BTModule_ActionReport[REPORT_RX]: RxRssi %d, TotalRXBits %u, TotalRxCounts %u, "
              "TotalRxErrorBits %u, ber %f, RXRecvPktCnts %u, Cfo %f",
              pReport->RxRssi, pReport->TotalRXBits, pReport->TotalRxCounts,
              pReport->TotalRxErrorBits, pReport->ber, pReport->RXRecvPktCnts, pReport->Cfo);
        break;

    case REPORT_CHIP:
        if (pModuleBtDevice->GetChipVersionInfo(pModuleBtDevice) != BT_FUNCTION_SUCCESS)
        {
            goto error;
        }
        else
        {
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
        for (i = 0; i < MAX_TXGAIN_TABLE_SIZE; i++)
        {
            pReport->CurrTXGainTable[i] = pModuleBtDevice->TXGainTable[i];
        }
        for (i = 0; i < MAX_TXDAC_TABLE_SIZE; i++)
        {
            pReport->CurrTXDACTable[i] = pModuleBtDevice->TXDACTable[i];
        }
        break;

    case REPORT_TX_GAIN_TABLE:
        for (i = 0; i < MAX_TXGAIN_TABLE_SIZE; i++)
        {
            pReport->CurrTXGainTable[i] = pModuleBtDevice->TXGainTable[i];
        }
        break;

    case REPORT_TX_DAC_TABLE:
        for (i = 0; i < MAX_TXDAC_TABLE_SIZE; i++)
        {
            pReport->CurrTXDACTable[i] = pModuleBtDevice->TXDACTable[i];
        }
        break;


    case REPORT_XTAL:
        pModuleBtDevice->GetRtl8761Xtal(pModuleBtDevice, &pReport->CurrRtl8761Xtal);
        break;

    case REPORT_THERMAL:
        pModuleBtDevice->ReadThermal(pModuleBtDevice, pBtModule->pBtParam, &pReport->CurrThermalValue);
        break;

    case REPORT_BT_STAGE:
        pModuleBtDevice->GetStage(pModuleBtDevice, &pReport->CurrStage);
        break;

    case REPORT_LOGICAL_EFUSE:
        for (i = 0; i < pModuleBtReport->ReportData[3] + LEN_4_BYTE; i++)
        {
            pReport->ReportData[i] = pModuleBtReport->ReportData[i];
        }
        break;

    case REPORT_LE_RX:
        pReport->TotalRXBits=pModuleBtReport->TotalRXBits;
        pReport->TotalRxCounts=pModuleBtReport->TotalRxCounts;
        pReport->TotalRxErrorBits=pModuleBtReport->TotalRxErrorBits;
        pReport->ber=pModuleBtReport->ber;
        pReport->RxRssi=pModuleBtReport->RxRssi;
        pReport->RXRecvPktCnts=pModuleBtReport->RXRecvPktCnts;
        pReport->Cfo=pModuleBtReport->Cfo;
        break;

    case REPORT_LE_CONTINUE_TX:
        if(ChipType <RTK_BT_CHIP_ID_RTL8822B)
        {
            pModuleBtDevice->SetContinueTxUpdate(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        else
        {
            pModuleBtDevice->FwContTxReport(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        pReport->TotalTXBits=pModuleBtReport->TotalTXBits;
        pReport->TotalTxCounts=pModuleBtReport->TotalTxCounts;
        break;

    case REPORT_FW_PACKET_TX:
        pModuleBtDevice->FwPacketTxReport(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        pReport->TotalTXBits=pModuleBtReport->TotalTXBits;
        pReport->TotalTxCounts=pModuleBtReport->TotalTxCounts;
        break;

    case REPORT_FW_CONTINUE_TX:
        pModuleBtDevice->FwContTxReport(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        pReport->TotalTXBits=pModuleBtReport->TotalTXBits;
        pReport->TotalTxCounts=pModuleBtReport->TotalTxCounts;
        break;

    case REPORT_FW_PACKET_RX:
        pModuleBtDevice->FwPacketRxReport(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        pReport->TotalRXBits=pModuleBtReport->TotalRXBits;
        pReport->TotalRxCounts=pModuleBtReport->TotalRxCounts;
        pReport->TotalRxErrorBits=pModuleBtReport->TotalRxErrorBits;
        pReport->ber=pModuleBtReport->ber;
        pReport->RxRssi=pModuleBtReport->RxRssi;
        pReport->RXRecvPktCnts=pModuleBtReport->RXRecvPktCnts;
        pReport->Cfo=pModuleBtReport->Cfo;
        break;

    case REPORT_FW_LE_CONTINUE_TX:
        pModuleBtDevice->FwContTxReport(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        pReport->TotalTXBits=pModuleBtReport->TotalTXBits;
        pReport->TotalTxCounts=pModuleBtReport->TotalTxCounts;
        break;

    case REPORT_TX_POWER_INFO:
        for ( i=0; i<LEN_5_BYTE; i++)
        {
            pReport->ReportData[i] = pModuleBtReport->ReportData[i];
        }
        break;

    case REPORT_GPIO3_0:
        pModuleBtDevice->GetGPIO3_0(pModuleBtDevice, &pReport->ReportData[0]);
        break;


    case REPORT_MP_DEBUG_MESSAGE:
        pModuleBtDevice->MpDebugMessageReport(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        for ( i=0; i<MP_DEBUG_MESSAGE_DATA_LEN; i++)
        {
            pReport->ReportData[i] = pModuleBtReport->ReportData[i];
        }
        break;

    case REPORT_MP_FT_VALUE:
        pModuleBtDevice->MpFTValueReport(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        for ( i=0; i<MP_FT_VALUE_DATA_LEN; i++)
        {
            pReport->ReportData[i] = pModuleBtReport->ReportData[i];
        }
        break;

    default:
        goto error;

    }


    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;

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
    pBtModuleParam->bHoppingFixChannel      = pParam->bHoppingFixChannel;
    pBtModuleParam->Rtl8761Xtal             = pParam->Rtl8761Xtal;

    pBtModuleParam->ExeMode                 =  pParam->ExeMode;
    pBtModuleParam->PHY                     =  pParam->PHY;
    pBtModuleParam->ModulationIndex         =  pParam->ModulationIndex;

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



int BTModule_ActionControlExcute(
    BT_MODULE *pBtModule
    )
{
    int rtn = BT_FUNCTION_SUCCESS;
    unsigned int ChipType;
    int Item = pBtModule->pBtParam->ParameterIndex;
    BT_DEVICE *pModuleBtDevice = pBtModule->pBtDevice;
    BT_PARAMETER *pModuleBtParam = pBtModule->pBtParam;
    BT_DEVICE_REPORT *pModuleBtReport = pBtModule->pModuleBtReport;

    SYSLOGI("BTModule_ActionControlExcute: pBtModule 0x%p, pBtDevice 0x%p, pBtParam 0x%p, "
           "pModuleBtReport 0x%p, ParameterIndex %d", pBtModule, pModuleBtDevice, pModuleBtParam,
            pModuleBtReport, Item);

    ChipType = pModuleBtDevice->pBTInfo->ChipType;

    switch (Item)
    {
    ////////////////////////// TABLE ///////////////////////////////////////////////////////
    case SET_TX_GAIN_TABLE:
        rtn = pModuleBtDevice->SetTxGainTable(pModuleBtDevice, pModuleBtParam->TXGainTable);
        break;

    case SET_TX_DAC_TABLE:
        rtn = pModuleBtDevice->SetTxDACTable(pModuleBtDevice, pModuleBtParam->TXDACTable);
        break;

    ////////////////////////// DEVICE MEMBER ////////////////////////////////////////////////
    case SET_POWER_GAIN_INDEX:
        rtn = pModuleBtDevice->SetPowerGainIndex(pModuleBtDevice, pModuleBtParam->mPacketType, pModuleBtParam->mTxGainIndex);
        break;

    case SET_POWER_GAIN:
        rtn = pModuleBtDevice->SetPowerGain(pModuleBtDevice, pModuleBtParam->mTxGainValue);
        break;

    case SET_POWER_DAC:
        rtn = pModuleBtDevice->SetPowerDac(pModuleBtDevice, pModuleBtParam->mTxDAC);
        break;

    ////////////////////////// HCI RESET /////////////////////////////////////////////////////////
    case HCI_RESET:
        rtn = pModuleBtDevice->SetHciReset(pModuleBtDevice, 700);
        if(rtn != BT_FUNCTION_SUCCESS)
            break;
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

    /////////////////////////// PACKET_TX /////////////////////////////////////////////////////////
    case PACKET_TX_START:
        if(ChipType <RTK_BT_CHIP_ID_RTL8822B)
        {
            rtn = pModuleBtDevice->SetPktTxBegin(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        else
        {
            rtn = pModuleBtDevice->FwPacketTxStart(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        break;

    case PACKET_TX_UPDATE:
        if(ChipType <RTK_BT_CHIP_ID_RTL8822B)
        {
            rtn = pModuleBtDevice->SetPktTxUpdate(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        break;

    case PACKET_TX_STOP:
        if(ChipType <RTK_BT_CHIP_ID_RTL8822B)
        {
            rtn = pModuleBtDevice->SetPktTxStop(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        else
        {
            rtn = pModuleBtDevice->FwPacketTxStop(pModuleBtDevice);
        }
        break;
    ////////////////////////// PACKET_RX /////////////////////////////////////////////////////////
    case PACKET_RX_START:
        if(ChipType <RTK_BT_CHIP_ID_RTL8822B)
        {
            rtn = pModuleBtDevice->SetPktRxBegin(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        else
        {
            rtn = pModuleBtDevice->FwPacketRxStart(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        break;

    case PACKET_RX_UPDATE:
        if(ChipType <RTK_BT_CHIP_ID_RTL8822B)
        {
            rtn = pModuleBtDevice->SetPktRxUpdate(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        break;

    case PACKET_RX_STOP:
        if(ChipType <RTK_BT_CHIP_ID_RTL8822B)
        {
            rtn = pModuleBtDevice->SetPktRxStop(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        else
        {
            rtn = pModuleBtDevice->FwPacketRxStop(pModuleBtDevice);
        }
        break;

    /////////////////////////// CONTINUE_TX /////////////////////////////////////////////////////////
    case CONTINUE_TX_START:
        if(ChipType <RTK_BT_CHIP_ID_RTL8822B)
        {
            rtn = pModuleBtDevice->SetContinueTxBegin(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        else
        {
            rtn = pModuleBtDevice->FwContTxStart(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        break;

    case CONTINUE_TX_STOP:
        if(ChipType <RTK_BT_CHIP_ID_RTL8822B)
        {
            rtn=pModuleBtDevice->SetContinueTxStop(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        else
        {
            rtn = pModuleBtDevice->FwContTxStop(pModuleBtDevice);
        }
        break;

    case CONTINUE_TX_UPDATE:
        if(ChipType <RTK_BT_CHIP_ID_RTL8822B)
        {
            rtn = pModuleBtDevice->SetContinueTxUpdate(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        break;

    /////////////////////////// HOPPING  /////////////////////////////////////////////////////////
    case HOPPING_DWELL_TIME:
        rtn = pModuleBtDevice->SetHoppingMode(pModuleBtDevice,
                pModuleBtParam->mChannelNumber,
                pModuleBtParam->mPacketType,
                pModuleBtParam->mPayloadType,
                pModuleBtParam->mTxGainValue,
                pModuleBtParam->mWhiteningCoeffValue,
                pModuleBtParam->mTxGainIndex,
                pModuleBtParam->mTxDAC,
                pModuleBtParam->bHoppingFixChannel
                );
        break;

    case TEST_MODE_ENABLE:
        rtn = pModuleBtDevice->TestModeEnable(pModuleBtDevice);
        break;

    /////////////////////////// REPORT /////////////////////////////////////////////////////////
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

    case SET_DEFAULT_TX_GAIN_TABLE:
        rtn = pModuleBtDevice->SetTxGainTable(pModuleBtDevice, NULL);
        break;

    case SET_DEFAULT_TX_DAC_TABLE:
        rtn = pModuleBtDevice->SetTxDACTable(pModuleBtDevice, NULL);
        break;

    case SET_XTAL:
        rtn = pModuleBtDevice->SetRtl8761Xtal(pModuleBtDevice, pModuleBtParam->Rtl8761Xtal);
        break;

    case WRITE_EFUSE_DATA:
        rtn = pModuleBtDevice->BT_WriteEfuseLogicalData(pModuleBtDevice, pModuleBtParam);
        break;

    case LE_TX_DUT_TEST_CMD:
        if(ChipType == RTK_BT_CHIP_ID_RTL8763B)//BBPro
        {
            rtn = BTDevice_LeTxEnhancedTest(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        else
        {
            rtn = pModuleBtDevice->LeTxTestCmd(pModuleBtDevice, pModuleBtParam, pModuleBtReport);
        }
        break;

    case LE_RX_DUT_TEST_CMD:
        if(ChipType == RTK_BT_CHIP_ID_RTL8763B)//BBPro
        {
            rtn = BTDevice_LeRxEnhancedTest(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        else
        {
            rtn = pModuleBtDevice->LeRxTestCmd(pModuleBtDevice, pModuleBtParam, pModuleBtReport);
        }
        break;

    case LE_DUT_TEST_END_CMD:
        rtn = pModuleBtDevice->LeTestEndCmd(pModuleBtDevice, pModuleBtParam, pModuleBtReport);
        break;

    case READ_EFUSE_DATA:
        rtn = pModuleBtDevice->BT_ReadEfuseLogicalData(pModuleBtDevice, pModuleBtParam, pModuleBtReport);
        break;

    case SET_CONFIG_FILE_DATA:
        rtn = pModuleBtDevice->BTSetConfigFileData(pModuleBtDevice, pModuleBtParam->mParamData);
        break;

    case CLEAR_CONFIG_FILE_DATA:
        rtn = pModuleBtDevice->BTSetConfigFileData(pModuleBtDevice, NULL);
        break;

    // LE Cont-Tx
    case LE_CONTINUE_TX_START:
        if(ChipType <RTK_BT_CHIP_ID_RTL8822B)
        {
            pModuleBtParam->mChannelNumber = pModuleBtParam->mChannelNumber * 2;
            pModuleBtParam->mWhiteningCoeffValue=0x7f;
            pModuleBtParam->mPayloadType= BT_PAYLOAD_TYPE_PRBS9;
            pModuleBtParam->mPacketType= BT_PKT_1DH1;
            rtn = pModuleBtDevice->SetContinueTxBegin(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        else
        {
            pModuleBtParam->mChannelNumber = pModuleBtParam->mChannelNumber*2;
            pModuleBtParam->mWhiteningCoeffValue=0x7f;
            pModuleBtParam->mPayloadType= BT_PAYLOAD_TYPE_PRBS9;
            if(ChipType != RTK_BT_CHIP_ID_RTL8763B)//not BBPro
            {
                pModuleBtParam->mPacketType= BT_PKT_1DH1;
            }
            rtn = pModuleBtDevice->FwContTxStart(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        break;

    case LE_CONTINUE_TX_STOP:
        if(ChipType <RTK_BT_CHIP_ID_RTL8822B)
        {
            rtn = pModuleBtDevice->SetContinueTxStop(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        }
        else
        {
            rtn = pModuleBtDevice->FwContTxStop(pModuleBtDevice);
        }
        break;

    case FW_PACKET_TX_START:
        rtn = pModuleBtDevice->FwPacketTxStart(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        break;

    case FW_PACKET_TX_STOP:
        rtn = pModuleBtDevice->FwPacketTxStop(pModuleBtDevice);
        break;

    case FW_CONTINUE_TX_START:
        rtn = pModuleBtDevice->FwContTxStart(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        break;

    case FW_CONTINUE_TX_STOP:
        rtn = pModuleBtDevice->FwContTxStop(pModuleBtDevice);
        break;

    case FW_PACKET_RX_START:
        rtn = pModuleBtDevice->FwPacketRxStart(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        break;

    case FW_PACKET_RX_STOP:
        rtn = pModuleBtDevice->FwPacketRxStop(pModuleBtDevice);
        break;

    case FW_LE_CONTINUE_TX_START:
        pModuleBtParam->mChannelNumber = pModuleBtParam->mChannelNumber*2;
        pModuleBtParam->mWhiteningCoeffValue=0x7f;
        pModuleBtParam->mPayloadType= BT_PAYLOAD_TYPE_PRBS9;
        pModuleBtParam->mPacketType= BT_PKT_1DH1;
        rtn = pModuleBtDevice->FwContTxStart(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        break;

    case FW_LE_CONTINUE_TX_STOP:
        rtn = pModuleBtDevice->FwContTxStop(pModuleBtDevice);
        break;

    case FW_READ_TX_POWER_INFO:
        rtn = pModuleBtDevice->FwReadTxPowerInfo(pModuleBtDevice,pModuleBtParam,pModuleBtReport);
        break;

    case SET_GPIO3_0:
        rtn = pModuleBtDevice->SetGPIO3_0(pModuleBtDevice, pModuleBtParam->mParamData[0]);
        break;

    case SET_ANT_INFO:
        rtn = pModuleBtDevice->SetAntInfo(pModuleBtDevice, pModuleBtParam->mParamData[0]);
        break;
    case SET_ANT_DIFF_S0S1:
        rtn = pModuleBtDevice->SetAntDiffS0S1(pModuleBtDevice, pModuleBtParam->mParamData[0], pModuleBtParam->mParamData[1]);
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
        rtn=pModuleBtDevice->BTDlMERGERFW(pModuleBtDevice, pPatchcode, patchLength);
    }
    else
    {
        rtn=pModuleBtDevice->BTDlFW(pModuleBtDevice, pPatchcode, patchLength);
    }

    pModuleBtDevice->SetHciReset(pModuleBtDevice, 500);

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
        uint16_t UserValue
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
        uint16_t UserValue
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
        uint16_t *pUserValue
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
        uint16_t UserValue
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
        uint16_t *pUserValue
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
        uint16_t UserValue
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
        uint16_t *pUserValue
        )
{
    BT_DEVICE *pBtDevice = pBtModule->pBtDevice;

    switch (Type)
    {
    case MD_REG:
        return pBtDevice->GetMdRegMaskBits(pBtDevice, (uint8_t)Addr, Msb, Lsb, pUserValue);

    case RF_REG:
        return pBtDevice->GetRfRegMaskBits(pBtDevice, (uint8_t)Addr, Msb, Lsb, pUserValue);

    case SYS_REG:
        return pBtDevice->GetSysRegMaskBits(pBtDevice, Addr, Msb, Lsb, pUserValue);

    case BB_REG:
        return pBtDevice->GetBBRegMaskBits(pBtDevice, Page, Addr, Msb, Lsb, pUserValue);

    default:
        goto error;
    }

error:

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
        uint16_t UserValue
        )
{
    BT_DEVICE *pBtDevice = pBtModule->pBtDevice;

    switch (Type)
    {
    case MD_REG:
        return pBtDevice->SetMdRegMaskBits(pBtDevice, (uint8_t)Addr, Msb, Lsb, UserValue);

    case RF_REG:
        return pBtDevice->SetRfRegMaskBits(pBtDevice, (uint8_t)Addr, Msb, Lsb, UserValue);

    case SYS_REG:
        return pBtDevice->SetSysRegMaskBits(pBtDevice, Addr, Msb, Lsb, UserValue);

    case BB_REG:
        return pBtDevice->SetBBRegMaskBits(pBtDevice, Page, Addr, Msb, Lsb, UserValue);

    default:
        goto error;
    }

error:

    return BT_FUNCTION_SUCCESS;


}





