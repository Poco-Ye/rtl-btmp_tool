#define LOG_TAG "btif_mp_api"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <termios.h>
#include <time.h>
#include <utils/Log.h>

#include "bluetoothmp.h"
#include "bt_mp_transport.h"
#include "bt_mp_api.h"
#include "bt_mp_base.h"
#include "bt_mp_build.h"
#include "bt_user_func.h"
#include "bt_mp_device_general.h"
#include "bt_mp_device_base.h"
#include "bt_mp_device_skip.h"
#include "bt_mp_device_rtl8723a.h"
#include "bt_mp_module_base.h"
#include "foundation.h"

#include "btif_api.h"

#include "gki.h"
#include "btu.h"

#include "hcidefs.h"
#include "hcimsgs.h"
#include "btif_common.h"


#define DEFAULT_HIT_ADDRESS 0x0000009e8b33

#define DEFAULT_CH_NUM                      10
#define DEFAULT_PKT_TYPE                    BT_PKT_3DH5
#define DEFAULT_PAYLOAD_TYPE                BT_PAYLOAD_TYPE_PRBS9
#define DEFAULT_PKT_COUNT                   0
#define DEFAULT_TX_GAIN_VALUE               0xA9
#define DEFAULT_WHITE_COEFF_ENABLE          0

#define DEFAULT_TX_GAIN_INDEX               0xFF
#define DEFAULT_TSET_MODE                   BT_PSEUDO_MODE
#define DEFAULT_TX_DAC                      0x13
#define DEFAULT_PKTHEADER                   0x1234
#define DEFAULT_MULTI_RX_ENABLE             0

typedef  struct _EVENT_STRING{

     char  EventData[3];//,XX

}EVENT_STRING;

int BT_SendHciCmd(BT_MODULE *pBtModule, char *p, char *pNotifyBuffer)
{
    int rtn = 0;
    const char *delim = STR_BT_MP_TX_PARA_DELIM;
    char *token = NULL;
    uint16_t opcode = 0;
    unsigned char  paraLen = 0;
    unsigned char paraMeters[255];
    uint8_t maxParaCount = 255;

    const unsigned char nTotalParaCount = 2;//at least 2 parameters: opcode, parameterlen, parameters
    unsigned char rxParaCount = 0;


    uint8_t EventType = 0x0E;
    uint8_t pEvent[255] = {0};
    EVENT_STRING pEventString[255];
    unsigned long EventLen = 0;
    uint8_t i = 0;

    ALOGI("++%s: %s", STR_BT_MP_HCI_CMD, p);

    token = strtok(p, delim);
    if (token != NULL) {
        opcode = strtol(token, NULL, 0);
        rxParaCount++;
    } else {
        goto EXIT;
    }

    token = strtok(NULL, delim);
    if (token != NULL) {
        paraLen = strtol(token, NULL, 0);
        rxParaCount++;
    } else {
        goto EXIT;
    }

    while (maxParaCount--) {
        //end of parameter
        token = strtok(NULL, delim);
        if (token != NULL) {
            paraMeters[i++] = strtol(token, NULL, 0);
            rxParaCount++;
        } else {
            goto EXIT;
        }
    }

EXIT:
    if (rxParaCount == paraLen + 2) {
        ALOGI("Opcode:0x%04x, 0x%02x", opcode, paraLen);

        rtn = pBtModule->SendHciCommandWithEvent(pBtModule, opcode, paraLen, paraMeters, EventType, pEvent, &EventLen);

        ALOGI("%s%s%x", STR_BT_MP_HCI_CMD, STR_BT_MP_RX_RESULT_DELIM, rtn);

        sprintf(pNotifyBuffer, "%s", STR_BT_MP_HCI_CMD);

        for (i = 0; i < EventLen; i++) {
            sprintf(pEventString[i].EventData, "%s%x", STR_BT_MP_RX_RESULT_DELIM, pEvent[i]);
            strcat(pNotifyBuffer, pEventString[i].EventData);
        }
    } else {
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_HCI_CMD, STR_BT_MP_RX_RESULT_DELIM, FUNCTION_PARAMETER_ERROR);
    }

    ALOGI("--%s", STR_BT_MP_HCI_CMD);

    return rtn;
}

int BT_GetPara(BT_MODULE  *pBtModule, char* pNotifyBuffer)
{
    ALOGI("++%s", STR_BT_MP_GET_PARA);

    ALOGI("%s%s%x%s%x%s%x%s%x%s%x%s%x%s%x%s%x%s%x%s%x%s%llx",
                                STR_BT_MP_GET_PARA,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mChannelNumber,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mPacketType,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mPayloadType,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mTxPacketCount,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mTxGainValue,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mWhiteningCoeffEnable,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mTxGainIndex,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mTestMode,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mTxDAC,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mPacketHeader,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mHitTarget
                                );

    sprintf(pNotifyBuffer, "%s%s%x%s%x%s%x%s%x%s%x%s%x%s%x%s%x%s%x%s%x%s%llx",
                                STR_BT_MP_GET_PARA,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mChannelNumber,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mPacketType,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mPayloadType,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mTxPacketCount,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mTxGainValue,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mWhiteningCoeffEnable,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mTxGainIndex,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mTestMode,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mTxDAC,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mPacketHeader,
                                STR_BT_MP_RX_RESULT_DELIM,
                                pBtModule->pBtParam->mHitTarget
                                );

    ALOGI("--%s", STR_BT_MP_GET_PARA);
    return 0;
}


int BT_SetGainTable(BT_MODULE  *pBtModule, char *p, char *pNotifyBuffer)
{
    const char  *delim = STR_BT_MP_TX_PARA_DELIM;
    char    * token = NULL;
    const unsigned char BT_PARA1_COUNT = 7;
    unsigned char rxParaCount = 0;

    ALOGI("++%s: %s", STR_BT_MP_SET_GAIN_TABLE, p);

    token = strtok(p, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->TXGainTable[0] = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->TXGainTable[1] = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->TXGainTable[2] = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->TXGainTable[3] = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->TXGainTable[4] = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->TXGainTable[5] = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->TXGainTable[6] = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    //end of parameter
    token = strtok(NULL, delim);
    if(token != NULL)
    {
        ALOGI("token = %s", token);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

EXIT:
    ALOGI("%s: rxParaCount = %d", STR_BT_MP_SET_GAIN_TABLE, rxParaCount);

    if(rxParaCount != BT_PARA1_COUNT)
    {
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_GAIN_TABLE, STR_BT_MP_RX_RESULT_DELIM, FUNCTION_PARAMETER_ERROR);
    }
    else
    {
        ALOGI("TXGainTable:0x%02x, 0x%02x,0x%02x, 0x%02x,0x%02x, 0x%02x,0x%02x",
                                   pBtModule->pBtParam->TXGainTable[0],
                                   pBtModule->pBtParam->TXGainTable[1],
                                   pBtModule->pBtParam->TXGainTable[2],
                                   pBtModule->pBtParam->TXGainTable[3],
                                   pBtModule->pBtParam->TXGainTable[4],
                                   pBtModule->pBtParam->TXGainTable[5],
                                   pBtModule->pBtParam->TXGainTable[6]
                                   );

        ALOGI("%s%s%x", STR_BT_MP_SET_GAIN_TABLE, STR_BT_MP_RX_RESULT_DELIM, BT_FUNCTION_SUCCESS);
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_GAIN_TABLE, STR_BT_MP_RX_RESULT_DELIM, BT_FUNCTION_SUCCESS);
    }


    ALOGI("--%s", STR_BT_MP_SET_GAIN_TABLE);
    return 0;
}



int BT_SetDacTable(BT_MODULE  *pBtModule, char *p, char *pNotifyBuffer)
{
    const char  *delim = STR_BT_MP_TX_PARA_DELIM;
    char    * token = NULL;
    const unsigned char BT_PARA1_COUNT = 5;
    unsigned char rxParaCount = 0;

    ALOGI("++%s: %s", STR_BT_MP_SET_DAC_TABLE, p);

    token = strtok(p, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->TXDACTable[0] = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->TXDACTable[1] = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->TXDACTable[2] = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->TXDACTable[3] = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->TXDACTable[4] = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    //end of parameter
    token = strtok(NULL, delim);
    if(token != NULL)
    {
        ALOGI("token = %s", token);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

EXIT:
    ALOGI("%s: rxParaCount = %d", STR_BT_MP_SET_DAC_TABLE, rxParaCount);

    if(rxParaCount != BT_PARA1_COUNT)
    {
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_DAC_TABLE, STR_BT_MP_RX_RESULT_DELIM, FUNCTION_PARAMETER_ERROR);
    }
    else
    {
        ALOGI("TXDACTable:0x%02x, 0x%02x,0x%02x, 0x%02x,0x%02x",
                                   pBtModule->pBtParam->TXDACTable[0],
                                   pBtModule->pBtParam->TXDACTable[1],
                                   pBtModule->pBtParam->TXDACTable[2],
                                   pBtModule->pBtParam->TXDACTable[3],
                                   pBtModule->pBtParam->TXDACTable[4]
                                   );
        ALOGI("%s%s%x", STR_BT_MP_SET_DAC_TABLE, STR_BT_MP_RX_RESULT_DELIM, BT_FUNCTION_SUCCESS);
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_DAC_TABLE, STR_BT_MP_RX_RESULT_DELIM, BT_FUNCTION_SUCCESS);
    }

    ALOGI("--%s", STR_BT_MP_SET_DAC_TABLE);
    return 0;
}


int BT_SetPara1(BT_MODULE  *pBtModule, char *p, char *pNotifyBuffer)
{
    const char  *delim = STR_BT_MP_TX_PARA_DELIM;
    char    * token = NULL;
    const unsigned char BT_PARA1_COUNT = 5;
    unsigned char rxParaCount = 0;

    ALOGI("++%s: %s", STR_BT_MP_SET_PARA1, p);

    //    unsigned char mChannelNumber;
    token = strtok(p, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->mChannelNumber = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    //BT_PKT_TYPE   mPacketType;
    token = strtok(NULL, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->mPacketType = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    //BT_PAYLOAD_TYPE mPayloadType;
    token = strtok(NULL, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->mPayloadType = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    //unsigned long mTxPacketCount;
    token = strtok(NULL, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->mTxPacketCount = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    //unsigned char mTxGainValue;
    token = strtok(NULL, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->mTxGainValue = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    //end of parameter
    token = strtok(NULL, delim);
    if(token != NULL)
    {
        ALOGI("token = %s", token);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

EXIT:
    ALOGI("%s: rxParaCount = %d", STR_BT_MP_SET_PARA1, rxParaCount);

    if(rxParaCount != BT_PARA1_COUNT)
    {
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_PARA1, STR_BT_MP_RX_RESULT_DELIM, FUNCTION_PARAMETER_ERROR);
    }
    else
    {
        ALOGI("mChannelNumber:0x%x, mPacketType:0x%x, mPayloadType:0x%x, mTxPacketCount:0x%x, mTxGainValue:0x%x, mWhiteningCoeffEnable:0x%x",
                                   pBtModule->pBtParam->mChannelNumber,
                                   pBtModule->pBtParam->mPacketType,
                                   pBtModule->pBtParam->mPayloadType,
                                   pBtModule->pBtParam->mTxPacketCount,
                                   pBtModule->pBtParam->mTxGainValue,
                                   pBtModule->pBtParam->mWhiteningCoeffEnable
                                   );

        ALOGI("%s%s%x", STR_BT_MP_SET_PARA1, STR_BT_MP_RX_RESULT_DELIM, BT_FUNCTION_SUCCESS);
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_PARA1, STR_BT_MP_RX_RESULT_DELIM, BT_FUNCTION_SUCCESS);
    }

    ALOGI("--%s", STR_BT_MP_SET_PARA1);
    return 0;
}

int BT_SetPara2(BT_MODULE  *pBtModule, char *p, char *pNotifyBuffer)
{
    const char  *delim = ",";
    char    * token = NULL;
    const unsigned char BT_PARA1_COUNT = 5;
    unsigned char rxParaCount = 0;

    ALOGI("++%s: %s", STR_BT_MP_SET_PARA2, p);

    //unsigned char mTxGainIndex;
    token = strtok(p, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->mTxGainIndex = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    //BT_TEST_MODE mTestMode;
    token = strtok(NULL, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->mTestMode = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    //unsigned char mTxDAC;
    token = strtok(NULL, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->mTxDAC = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

     //unsigned char mWhiteningCoeffEnable;
    token = strtok(NULL, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->mWhiteningCoeffEnable = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    //unsigned int  mPacketHeader;
    token = strtok(NULL, delim);
    if(token != NULL)
    {
        pBtModule->pBtParam->mPacketHeader = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

     //end of parameter
    token = strtok(NULL, delim);
    if(token != NULL)
    {
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

EXIT:
    ALOGI("%s: rxParaCount = %d", STR_BT_MP_SET_PARA2, rxParaCount);

    if(rxParaCount != BT_PARA1_COUNT)
    {
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_PARA2, STR_BT_MP_RX_RESULT_DELIM, FUNCTION_PARAMETER_ERROR);
    }
    else
    {
        ALOGI("mTxGainIndex:0x%x, mTestMode:0x%x, mTxDAC:0x%x, mWhiteningCoeffEnable:0x%x, mPacketHeader:0x%x",
                                   pBtModule->pBtParam->mTxGainIndex,
                                   pBtModule->pBtParam->mTestMode,
                                   pBtModule->pBtParam->mTxDAC,
                                   pBtModule->pBtParam->mWhiteningCoeffEnable,
                                   pBtModule->pBtParam->mPacketHeader
                                   );

        ALOGI("%s%s%x", STR_BT_MP_SET_PARA2, STR_BT_MP_RX_RESULT_DELIM, BT_FUNCTION_SUCCESS);
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_PARA2, STR_BT_MP_RX_RESULT_DELIM, BT_FUNCTION_SUCCESS);
    }

    ALOGI("--%s", STR_BT_MP_SET_PARA2);
    return 0;

}

int BT_SetHit(BT_MODULE  *pBtModule, char *p, char* pNotifyBuffer)
{
    ALOGI("++%s: %s", STR_BT_MP_SET_HIT, p);
    pBtModule->pBtParam->mHitTarget = strtoull(p, NULL, 16);

    ALOGI("%s:0x%llx", STR_BT_MP_SET_HIT, pBtModule->pBtParam->mHitTarget);

    ALOGI("%s%s%x", STR_BT_MP_SET_HIT, STR_BT_MP_RX_RESULT_DELIM, BT_FUNCTION_SUCCESS);
    sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_HIT, STR_BT_MP_RX_RESULT_DELIM, BT_FUNCTION_SUCCESS);

    ALOGI("--%s", STR_BT_MP_SET_HIT);
    return 0;
}

int BT_Exec(BT_MODULE  *pBtModule, char *p, char* pNotifyBuffer)
{
    int ParameterIndex = 0;
    BT_PARAMETER *pBtParam = NULL;
    int rtn = BT_FUNCTION_SUCCESS;

    ParameterIndex = strtol(p, NULL, 0);

    ALOGI("BT_Exec: buf[%s], param index[%d]", p, ParameterIndex);

    if (ParameterIndex > NOTTHING &&
        ParameterIndex < NUMBEROFBT_ACTIONCONTROL_TAG) {

        pBtParam = pBtModule->pBtParam;
        pBtParam->ParameterIndex = ParameterIndex;
        rtn = pBtModule->ActionControlExcute(pBtModule);

        sprintf(pNotifyBuffer, "%s%s%x%s%x",
                STR_BT_MP_EXEC,
                STR_BT_MP_RX_RESULT_DELIM,
                ParameterIndex,
                STR_BT_MP_RX_RESULT_DELIM,
                rtn
                );
         ALOGI("%s%s%x%s%x",
                STR_BT_MP_EXEC,
                STR_BT_MP_RX_RESULT_DELIM,
                ParameterIndex,
                STR_BT_MP_RX_RESULT_DELIM,
                rtn
                );
    } else {
        sprintf(pNotifyBuffer, "%s%s%s%s%x",
                STR_BT_MP_EXEC,
                STR_BT_MP_RX_RESULT_DELIM,
                p,
                STR_BT_MP_RX_RESULT_DELIM,
                FUNCTION_PARAMETER_ERROR
                );
        ALOGI("%s%s%s%s%x",
                STR_BT_MP_EXEC,
                STR_BT_MP_RX_RESULT_DELIM,
                p,
                STR_BT_MP_RX_RESULT_DELIM,
                FUNCTION_PARAMETER_ERROR
                );
    }

    return rtn;
}


int BT_ReportTx(BT_MODULE  *pBtModule, char* pNotifyBuffer)
{
    BT_DEVICE_REPORT BtDeviceReport;
    int rtn = BT_FUNCTION_SUCCESS;

    ALOGI("++%s", STR_BT_MP_REPORTTX);

    rtn=pBtModule->ActionReport(pBtModule, REPORT_TX, &BtDeviceReport);

    if (rtn != BT_FUNCTION_SUCCESS) {
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_REPORTTX, STR_BT_MP_RX_RESULT_DELIM, rtn);
        ALOGI("%s%s%x", STR_BT_MP_REPORTTX, STR_BT_MP_RX_RESULT_DELIM, rtn);
        goto exit;
    } else {
        ALOGI("%s%s%lx%s%lx",
                STR_BT_MP_REPORTTX,
                STR_BT_MP_RX_RESULT_DELIM,
                BtDeviceReport.TotalTXBits,
                STR_BT_MP_RX_RESULT_DELIM,
                BtDeviceReport.TotalTxCounts);

        sprintf(pNotifyBuffer, "%s%s%lx%s%lx",
                                STR_BT_MP_REPORTTX,
                                STR_BT_MP_RX_RESULT_DELIM,
                                BtDeviceReport.TotalTXBits,
                                STR_BT_MP_RX_RESULT_DELIM,
                                BtDeviceReport.TotalTxCounts);
    }

exit:
    ALOGI("--%s", STR_BT_MP_REPORTTX);

    return rtn;
}

int BT_ReportRx(BT_MODULE  *pBtModule, char* pNotifyBuffer)
{
    BT_DEVICE_REPORT BtDeviceReport;
    int rtn = BT_FUNCTION_SUCCESS;

    ALOGI("++%s", STR_BT_MP_REPORTRX);

    rtn = pBtModule->ActionReport(pBtModule, REPORT_RX, &BtDeviceReport);
    if (rtn != BT_FUNCTION_SUCCESS) {
        ALOGI("%s%s%x", STR_BT_MP_PKTRXSTART, STR_BT_MP_RX_RESULT_DELIM, rtn);
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_PKTRXSTART, STR_BT_MP_RX_RESULT_DELIM, rtn);
        goto exit;
    } else {
        ALOGI("%s%s%x%s%lx%s%lx%s%lx",
                STR_BT_MP_REPORTRX,
                STR_BT_MP_RX_RESULT_DELIM,
                BtDeviceReport.IsRxRssi,
                STR_BT_MP_RX_RESULT_DELIM,
                BtDeviceReport.TotalRXBits,
                STR_BT_MP_RX_RESULT_DELIM,
                BtDeviceReport.TotalRxCounts,
                STR_BT_MP_RX_RESULT_DELIM,
                BtDeviceReport.TotalRxErrorBits
             );

        sprintf(pNotifyBuffer, "%s%s%x%s%lx%s%lx%s%lx",
                STR_BT_MP_REPORTRX,
                STR_BT_MP_RX_RESULT_DELIM,
                BtDeviceReport.IsRxRssi,
                STR_BT_MP_RX_RESULT_DELIM,
                BtDeviceReport.TotalRXBits,
                STR_BT_MP_RX_RESULT_DELIM,
                BtDeviceReport.TotalRxCounts,
                STR_BT_MP_RX_RESULT_DELIM,
                BtDeviceReport.TotalRxErrorBits
               );
    }

exit:
    ALOGI("--%s", STR_BT_MP_REPORTRX);

    return rtn;
}


/**

register	    read/Write	    address	msb	        lsb	data	return(Status)	value
bt_regmd	(write)1	            1	                1	        1	2	1	　
	(read)0	1	1	1	0	1	2
bt_regrf	(write)1	1	1	1	2	1	　
	(read)0	1	1	1	0	1	2


*/

int BT_RegRf(BT_MODULE  *pBtModule, char *p, char* pNotifyBuffer)
{
    int rtn = BT_FUNCTION_SUCCESS;
    const char *delim = STR_BT_MP_TX_PARA_DELIM;
    char *token = NULL;
    uint8_t BT_REGRF_PARA_COUNT = 4;// 4:read, 5:write
    uint8_t rxParaCount = 0;
    uint8_t opReadWrite = 0;
    uint8_t address = 0;
    uint8_t msb = 0;
    uint8_t lsb = 0;
    uint16_t dataReadWrite = 0; //for opReadWrite = 1(write)

    ALOGI("++%s: %s", STR_BT_MP_REG_RF, p);

    //unsigned char opReadWrite;
    token = strtok(p, delim);
    if (token != NULL) {
        opReadWrite = strtol(token, NULL, 0);
        rxParaCount++;
    } else {
        goto EXIT;
    }

    //unsigned char address
    token = strtok(NULL, delim);
    if (token != NULL) {
        address = strtol(token, NULL, 0);
        rxParaCount++;
    } else {
        goto EXIT;
    }

    //unsigned char msb;
    token = strtok(NULL, delim);
    if (token != NULL) {
        msb = strtol(token, NULL, 0);
        rxParaCount++;
    } else {
        goto EXIT;
    }

    //unsigned char lsb;
    token = strtok(NULL, delim);
    if (token != NULL) {
        lsb = strtol(token, NULL, 0);
        rxParaCount++;
    } else {
        goto EXIT;
    }

    if (opReadWrite == 1) { //write
        BT_REGRF_PARA_COUNT = 5;
        //unsigned char dataToWrite;
        token = strtok(NULL, delim);
        if (token != NULL) {
            dataReadWrite = strtol(token, NULL, 0);
            rxParaCount++;
        } else {
            goto EXIT;
        }

    } else {
        BT_REGRF_PARA_COUNT = 4;
    }

    //end of parameter
    token = strtok(NULL, delim);
    if(token != NULL)
    {
        ALOGI("BT_RegRf: redundant token[%s]", token);
        rxParaCount++;
    } else {
        goto EXIT;
    }

EXIT:
    ALOGI("%s: rxParaCount = %d", STR_BT_MP_REG_RF, rxParaCount);

    if (rxParaCount != BT_REGRF_PARA_COUNT) {
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_REG_RF, STR_BT_MP_RX_RESULT_DELIM, FUNCTION_PARAMETER_ERROR);
    } else {
        ALOGI("BT_RegRf: opReadWrite 0x%02x, address 0x%02x, msb 0x%02x, lsb 0x%02x, dataToWrite 0x%04x",
                opReadWrite, address, msb, lsb, dataReadWrite);

        //sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_REG_RF, STR_BT_MP_RX_RESULT_DELIM, BT_FUNCTION_SUCCESS);

        if (opReadWrite == 0) {
            rtn = pBtModule->GetRfRegMaskBits(pBtModule, address, msb, lsb, &dataReadWrite);
            sprintf(pNotifyBuffer, "%s%s%x%s%x", STR_BT_MP_REG_RF, STR_BT_MP_RX_RESULT_DELIM, rtn, STR_BT_MP_RX_RESULT_DELIM, dataReadWrite);
        } else {
            rtn = pBtModule->SetRfRegMaskBits(pBtModule, address, msb, lsb, dataReadWrite);
            sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_REG_RF, STR_BT_MP_RX_RESULT_DELIM, rtn);
        }

    }

    ALOGI("--%s", STR_BT_MP_REG_RF);
    return rtn;
}

int BT_RegMd(BT_MODULE  *pBtModule, char *p, char *pNotifyBuffer)
{
    int rtn = BT_FUNCTION_SUCCESS;
    const char *delim = STR_BT_MP_TX_PARA_DELIM;
    char *token = NULL;
    uint8_t BT_REGRF_PARA_COUNT = 4;// 4:read, 5:write
    uint8_t rxParaCount = 0;
    uint8_t opReadWrite = 0;
    uint8_t address = 0;
    uint8_t msb = 0;
    uint8_t lsb = 0;
    uint16_t dataReadWrite = 0; //for opReadWrite = 1(write)

    ALOGI("++%s: %s", STR_BT_MP_REG_MD, p);

    //unsigned char opReadWrite;
    token = strtok(p, delim);
    if (token != NULL) {
        opReadWrite = strtol(token, NULL, 0);
        rxParaCount++;
    } else {
        goto EXIT;
    }

    //unsigned char address
    token = strtok(NULL, delim);
    if (token != NULL) {
        address = strtol(token, NULL, 0);
        rxParaCount++;
    } else {
        goto EXIT;
    }

    //unsigned char msb;
    token = strtok(NULL, delim);
    if (token != NULL) {
        msb = strtol(token, NULL, 0);
        rxParaCount++;
    } else {
        goto EXIT;
    }

    //unsigned char lsb;
    token = strtok(NULL, delim);
    if (token != NULL) {
        lsb = strtol(token, NULL, 0);
        rxParaCount++;
    } else {
        goto EXIT;
    }

    if (opReadWrite == 1) { //write
        BT_REGRF_PARA_COUNT = 5;
        //unsigned char dataToWrite;
        token = strtok(NULL, delim);
        if (token != NULL) {
            dataReadWrite = strtol(token, NULL, 0);
            rxParaCount++;
        } else {
            goto EXIT;
        }

    } else {
        BT_REGRF_PARA_COUNT = 4;
    }

    //end of parameter
    token = strtok(NULL, delim);
    if(token != NULL)
    {
        ALOGI("BT_RegMd: redundant token[%s]", token);
        rxParaCount++;
    } else {
        goto EXIT;
    }

EXIT:
    ALOGI("%s: rxParaCount = %d", STR_BT_MP_REG_MD, rxParaCount);

    if (rxParaCount != BT_REGRF_PARA_COUNT) {
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_REG_MD, STR_BT_MP_RX_RESULT_DELIM, FUNCTION_PARAMETER_ERROR);
    } else {
        ALOGI("BT_RegMd: opReadWrite 0x%x, address 0x%x, msb 0x%x, lsb 0x%x, dataToWrite 0x%x",
                opReadWrite, address, msb, lsb, dataReadWrite);

        //sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_REG_MD, STR_BT_MP_RX_RESULT_DELIM, BT_FUNCTION_SUCCESS);

        if (opReadWrite == 0) {
            rtn = pBtModule->GetMdRegMaskBits(pBtModule, address, msb, lsb, &dataReadWrite);
            sprintf(pNotifyBuffer, "%s%s%x%s%x", STR_BT_MP_REG_MD, STR_BT_MP_RX_RESULT_DELIM, rtn, STR_BT_MP_RX_RESULT_DELIM, dataReadWrite);
        } else {
            rtn = pBtModule->SetMdRegMaskBits(pBtModule, address, msb, lsb, dataReadWrite);
            sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_REG_MD, STR_BT_MP_RX_RESULT_DELIM, rtn);
        }

    }

    ALOGI("--%s", STR_BT_MP_REG_MD);
    return rtn;
}

int BT_SetHoppingMode(BT_MODULE *pBtModule, char *p, char* pNotifyBuffer)
{
    const char *delim = STR_BT_MP_TX_PARA_DELIM;
    char *token = NULL;
    const unsigned char BT_PARA1_COUNT = 4;
    unsigned char rxParaCount = 0;

    ALOGI("++%s: %s", STR_BT_MP_SET_HOPPING_MODE, p);

    // unsigned char mChannelNumber
    token = strtok(p, delim);
    if (token != NULL)
    {
        pBtModule->pBtParam->mChannelNumber = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    // BT_PKT_TYPE mPacketType
    token = strtok(NULL, delim);
    if (token != NULL)
    {
        pBtModule->pBtParam->mPacketType = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    // unsigned char mHoppingFixChannel
    token = strtok(NULL, delim);
    if (token != NULL)
    {
        pBtModule->pBtParam->mHoppingFixChannel = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    // unsigned char mWhiteningCoeffEnable
    token = strtok(NULL, delim);
    if (token != NULL)
    {
        pBtModule->pBtParam->mWhiteningCoeffEnable = strtol(token, NULL, 0);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

    // end of parameter
    token = strtok(NULL, delim);
    if (token != NULL)
    {
        ALOGI("token = %s", token);
        rxParaCount++;
    }
    else
    {
        goto EXIT;
    }

EXIT:
    ALOGI("%s: rxParaCount = %d", STR_BT_MP_SET_HOPPING_MODE, rxParaCount);

    if (rxParaCount != BT_PARA1_COUNT)
    {
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_HOPPING_MODE, STR_BT_MP_RX_RESULT_DELIM, FUNCTION_PARAMETER_ERROR);
    }
    else
    {
        ALOGI("mChannelNumber:0x%x, mPacketType:0x%x, mHoppingFixChannel:0x%x, mWhiteningCoeffEnable:0x%x",
                                   pBtModule->pBtParam->mChannelNumber,
                                   pBtModule->pBtParam->mPacketType,
                                   pBtModule->pBtParam->mHoppingFixChannel,
                                   pBtModule->pBtParam->mWhiteningCoeffEnable
                                   );

        ALOGI("%s%s%x", STR_BT_MP_SET_HOPPING_MODE, STR_BT_MP_RX_RESULT_DELIM, BT_FUNCTION_SUCCESS);
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_HOPPING_MODE, STR_BT_MP_RX_RESULT_DELIM, BT_FUNCTION_SUCCESS);
    }

    ALOGI("--%s", STR_BT_MP_SET_HOPPING_MODE);
    return 0;
}

void BT_GetBDAddr(BT_MODULE  *pBtModule)
{
    unsigned char  pEvt[256];
    unsigned long  EvtLen = 0;
    unsigned char  para[256];

    if (pBtModule->SendHciCommandWithEvent(pBtModule,0x1009,0,para,0x0e,pEvt, &EvtLen) != BT_FUNCTION_SUCCESS)
    {
        printf("Get BD Addree Fail!!..");
    }
    printf("BD_ADDR=[0x%.2x%.2x%.2x%.2x%.2x%.2x]",pEvt[11],pEvt[10],pEvt[9],pEvt[8],pEvt[7],pEvt[6]);
}

void bt_mp_module_init(BASE_INTERFACE_MODULE *pBaseInterfaceModule, BT_MODULE *pBtModule)
{
    unsigned char pTxGainTable[7] = {0x49,0x4d,0x69,0x89,0x8d,0xa9,0xa9};  //RTL8761 Table
    unsigned char pTxDACTable[5] = {0x10,0x11,0x12,0x13,0x14};

    ALOGI("bt_mp_module_init, pBaseInterfaceModule %p, pBtModule %p", pBaseInterfaceModule, pBtModule);

    BuildTransportInterface(
            pBaseInterfaceModule,
            1,
            115200,
            NULL,//open
            bt_transport_SendHciCmd,
            bt_transport_RecvHciEvt,
            NULL,//close
            UserDefinedWaitMs
            );

    BuildBluetoothModule(
            pBaseInterfaceModule,
            pBtModule,
            NULL,
            pTxGainTable,
            pTxDACTable
            );

    pBtModule->pBtParam->mHitTarget = 0x0000009e8b33;

    pBtModule->pBtParam->mChannelNumber = DEFAULT_CH_NUM;
    pBtModule->pBtParam->mPacketType = DEFAULT_PKT_TYPE;
    pBtModule->pBtParam->mPayloadType= DEFAULT_PAYLOAD_TYPE;
    pBtModule->pBtParam->mTxPacketCount = DEFAULT_PKT_COUNT;
    pBtModule->pBtParam->mTxGainValue= DEFAULT_TX_GAIN_VALUE;
    pBtModule->pBtParam->mWhiteningCoeffEnable = DEFAULT_WHITE_COEFF_ENABLE;
    pBtModule->pBtParam->mTxGainIndex= DEFAULT_TX_GAIN_INDEX;
    pBtModule->pBtParam->mTestMode= DEFAULT_TSET_MODE;
    pBtModule->pBtParam->mTxDAC= DEFAULT_TX_DAC;
    pBtModule->pBtParam->mPacketHeader= DEFAULT_PKTHEADER;
    pBtModule->pBtParam->mMutiRxEnable= DEFAULT_MULTI_RX_ENABLE;
}
