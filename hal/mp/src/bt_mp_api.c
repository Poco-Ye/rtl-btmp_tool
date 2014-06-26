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
#define DEFAULT_WHITE_COEFF_VALUE           0

#define DEFAULT_TX_GAIN_INDEX               0xFF
#define DEFAULT_TEST_MODE                   BT_PSEUDO_MODE
#define DEFAULT_TX_DAC                      0x13
#define DEFAULT_PKTHEADER                   0x1234
#define DEFAULT_HOPPING_CH_NUM              0
#define DEFAULT_MULTI_RX_ENABLE             0

#define BT_PARAM0   0   //mChannelNumber
#define BT_PARAM1   1   //mPacketType
#define BT_PARAM2   2   //mPayloadType
#define BT_PARAM3   3   //mTxPacketCount
#define BT_PARAM4   4   //mTxGainValue
#define BT_PARAM5   5   //mWhiteningCoeffValue
#define BT_PARAM6   6   //mTxGainIndex
#define BT_PARAM7   7   //mTxDAC
#define BT_PARAM8   8   //mPacketHeader
#define BT_PARAM9   9   //mHoppingFixChannel
#define BT_PARAM10  10  //mHitTarget
#define BT_PARAM_NUM 11

typedef struct _EVENT_STRING {
    char EventData[3];//,XX
} EVENT_STRING;

static void bt_index2param(BT_MODULE *pBtModule, int index, int64_t value)
{
    switch (index) {
    case BT_PARAM0:
        pBtModule->pBtParam->mChannelNumber = (uint8_t)value;
        break;
    case BT_PARAM1:
        pBtModule->pBtParam->mPacketType = (BT_PKT_TYPE)value;
        break;
    case BT_PARAM2:
        pBtModule->pBtParam->mPayloadType = (BT_PAYLOAD_TYPE)value;
        break;
    case BT_PARAM3:
        pBtModule->pBtParam->mTxPacketCount = (uint16_t)value;
        break;
    case BT_PARAM4:
        pBtModule->pBtParam->mTxGainValue = (uint8_t)value;
        break;
    case BT_PARAM5:
        pBtModule->pBtParam->mWhiteningCoeffValue = (uint8_t)value;
        break;
    case BT_PARAM6:
        pBtModule->pBtParam->mTxGainIndex = (uint8_t)value;
        break;
    case BT_PARAM7:
        pBtModule->pBtParam->mTxDAC = (uint8_t)value;
        break;
    case BT_PARAM8:
        pBtModule->pBtParam->mPacketHeader = (uint16_t)value;
        break;
    case BT_PARAM9:
        pBtModule->pBtParam->mHoppingFixChannel = (uint8_t)value;
        break;
    case BT_PARAM10:
        pBtModule->pBtParam->mHitTarget = (uint64_t)value;
        break;
    default:
        break;
    }
}

int BT_SendHciCmd(BT_MODULE *pBtModule, char *p, char *pNotifyBuffer)
{
    int rtn = 0;
    char *token = NULL;
    uint16_t opcode = 0;
    unsigned char  paraLen = 0;
    unsigned char paraMeters[255];
    uint8_t maxParaCount = 255;

    const unsigned char nTotalParaCount = 2;//at least 2 parameters: opcode, parameterlen, parameters
    unsigned char params_count = 0;

    uint8_t EventType = 0x0E;
    uint8_t pEvent[255] = {0};
    EVENT_STRING pEventString[255];
    uint32_t EventLen = 0;
    uint8_t i = 0;

    ALOGI("++%s: %s", STR_BT_MP_HCI_CMD, p);

    token = strtok(p, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        opcode = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto EXIT;
    }

    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        paraLen = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto EXIT;
    }

    while (maxParaCount--) {
        //end of parameter
        token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
        if (token != NULL) {
            paraMeters[i++] = strtol(token, NULL, 0);
            params_count++;
        } else {
            goto EXIT;
        }
    }

EXIT:
    if (params_count == paraLen + 2) {
        ALOGI("Opcode:0x%04x, 0x%02x", opcode, paraLen);

        rtn = pBtModule->SendHciCommandWithEvent(pBtModule, opcode, paraLen, paraMeters, EventType, pEvent, &EventLen);

        ALOGI("%s%s%x", STR_BT_MP_HCI_CMD, STR_BT_MP_RESULT_DELIM, rtn);

        sprintf(pNotifyBuffer, "%s", STR_BT_MP_HCI_CMD);

        for (i = 0; i < EventLen; i++) {
            sprintf(pEventString[i].EventData, "%s%x", STR_BT_MP_RESULT_DELIM, pEvent[i]);
            strcat(pNotifyBuffer, pEventString[i].EventData);
        }
    } else {
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_HCI_CMD, STR_BT_MP_RESULT_DELIM, FUNCTION_PARAMETER_ERROR);
    }

    ALOGI("--%s", STR_BT_MP_HCI_CMD);

    return rtn;
}

int BT_GetParam(BT_MODULE *pBtModule, char *pNotifyBuffer)
{
    ALOGI("++%s", STR_BT_MP_GET_PARAM);

    ALOGI("%s%s%x%s%x%s%x%s%x%s%x%s%x%s%x%s%x%s%x%s%x%s%012llx",
                                STR_BT_MP_GET_PARAM,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mChannelNumber,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mPacketType,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mPayloadType,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mTxPacketCount,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mTxGainValue,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mWhiteningCoeffValue,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mTxGainIndex,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mTxDAC,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mPacketHeader,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mHoppingFixChannel,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mHitTarget
                                );

    sprintf(pNotifyBuffer, "%s%s%x%s%x%s%x%s%x%s%x%s%x%s%x%s%x%s%x%s%x%s%012llx",
                                STR_BT_MP_GET_PARAM,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mChannelNumber,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mPacketType,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mPayloadType,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mTxPacketCount,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mTxGainValue,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mWhiteningCoeffValue,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mTxGainIndex,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mTxDAC,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mPacketHeader,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mHoppingFixChannel,
                                STR_BT_MP_RESULT_DELIM,
                                pBtModule->pBtParam->mHitTarget
                                );

    ALOGI("--%s", STR_BT_MP_GET_PARAM);
    return 0;
}

int BT_SetParam(BT_MODULE *pBtModule, char *p, char *pNotifyBuffer)
{
    uint8_t pairs_count, params_count;
    char *pair_token, *param_token;
    char *pairs_buf, *params_buf;
    char *save_pairs, *save_params;
    int index;
    int64_t value = 0;

    ALOGI("++%s: %s", STR_BT_MP_SET_PARAM, p);

    for (pairs_count = 0, pairs_buf = p; ; pairs_count++, pairs_buf = NULL) {
        pair_token = strtok_r(pairs_buf, STR_BT_MP_PAIR_DELIM, &save_pairs);
        if (pair_token == NULL)
            break;

        for (params_count = 0, params_buf = pair_token; ; params_count++, params_buf = NULL) {
            param_token = strtok_r(params_buf, STR_BT_MP_PARAM_DELIM, &save_params);
            if (param_token && params_count == 0) {
                index = strtol(param_token, NULL, 0);
                if (index < 0 || index >= BT_PARAM_NUM) {
                    ALOGI("Invalid BT param index %d", index);
                    sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_PARAM, STR_BT_MP_RESULT_DELIM, FUNCTION_PARAMETER_ERROR);
                    return FUNCTION_PARAMETER_ERROR;
                }
            } else if (param_token && params_count == 1)
                value = strtoll(param_token, NULL, 0);
            else if (param_token == NULL) // null token OR token parsing completed
                break;
        }
        if (params_count == 2) { // valid pair format<index, value>
            bt_index2param(pBtModule, index, value);
            ALOGI("Pair index %d, pair value 0x%llx", index, value);
        } else if (params_count == 0) { // null pair
            continue;
        } else { // wrong pair format
            ALOGI("Invalid BT pair format, params count %d", params_count);
            sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_PARAM, STR_BT_MP_RESULT_DELIM, FUNCTION_PARAMETER_ERROR);
            return FUNCTION_PARAMETER_ERROR;
        }
    }

    ALOGI("--%s: pairs count %d", STR_BT_MP_SET_PARAM, pairs_count);

    sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_PARAM, STR_BT_MP_RESULT_DELIM, BT_FUNCTION_SUCCESS);

    return BT_FUNCTION_SUCCESS;
}

int BT_SetParam1(BT_MODULE *pBtModule, char *p, char *pNotifyBuffer)
{
    char *token = NULL;
    const uint8_t BT_PARA1_COUNT = 6;
    uint8_t params_count = 0;
    int ret = BT_FUNCTION_SUCCESS;

    ALOGI("++%s: %s", STR_BT_MP_SET_PARAM1, p);

    // uint8_t mChannelNumber;
    token = strtok(p, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        pBtModule->pBtParam->mChannelNumber = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto EXIT;
    }

    // BT_PKT_TYPE mPacketType;
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        pBtModule->pBtParam->mPacketType = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto EXIT;
    }

    // BT_PAYLOAD_TYPE mPayloadType;
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        pBtModule->pBtParam->mPayloadType = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto EXIT;
    }

    // uint16_t mTxPacketCount;
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        pBtModule->pBtParam->mTxPacketCount = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto EXIT;
    }

    // uint8_t mTxGainValue;
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        pBtModule->pBtParam->mTxGainValue = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto EXIT;
    }

    // uint8_t mWhiteningCoeffValue;
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        pBtModule->pBtParam->mWhiteningCoeffValue = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto EXIT;
    }

    //end of parameter
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        ALOGI("BT_SetParam1: redundant token[%s]", token);
        params_count++;
    }

EXIT:
    ALOGI("%s: params_count = %d", STR_BT_MP_SET_PARAM1, params_count);

    if (params_count != BT_PARA1_COUNT) {
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_PARAM1, STR_BT_MP_RESULT_DELIM, FUNCTION_PARAMETER_ERROR);
        ret = FUNCTION_PARAMETER_ERROR;
    } else {
        ALOGI("mChannelNumber:0x%02x, mPacketType:0x%02x, mPayloadType:0x%02x, "
              "mTxPacketCount:0x%04x, mTxGainValue:0x%x, mWhiteningCoeffValue:0x%02x",
              pBtModule->pBtParam->mChannelNumber,
              pBtModule->pBtParam->mPacketType,
              pBtModule->pBtParam->mPayloadType,
              pBtModule->pBtParam->mTxPacketCount,
              pBtModule->pBtParam->mTxGainValue,
              pBtModule->pBtParam->mWhiteningCoeffValue
             );

        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_PARAM1, STR_BT_MP_RESULT_DELIM, BT_FUNCTION_SUCCESS);
    }

    ALOGI("--%s", STR_BT_MP_SET_PARAM1);
    return ret;
}

int BT_SetParam2(BT_MODULE *pBtModule, char *p, char *pNotifyBuffer)
{
    char *token = NULL;
    const uint8_t BT_PARAM2_COUNT = 5;
    uint8_t params_count = 0;
    int ret = BT_FUNCTION_SUCCESS;

    ALOGI("++%s: %s", STR_BT_MP_SET_PARAM2, p);

    // uint8_t mTxGainIndex;
    token = strtok(p, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        pBtModule->pBtParam->mTxGainIndex = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto EXIT;
    }

    // uint8_t mTxDAC;
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        pBtModule->pBtParam->mTxDAC = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto EXIT;
    }

    // uint16_t mPacketHeader;
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        pBtModule->pBtParam->mPacketHeader = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto EXIT;
    }

    // uint8_t mHoppingFixChannel
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        pBtModule->pBtParam->mHoppingFixChannel = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto EXIT;
    }

    // uint64_t(6 Bytes) mHitTarget
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        pBtModule->pBtParam->mHitTarget = strtoull(token, NULL, 16);
        params_count++;
    } else {
        goto EXIT;
    }

    // end of parameter
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        ALOGI("BT_SetParam2: redundant token[%s]", token);
        params_count++;
    }

EXIT:
    ALOGI("%s: params_count = %d", STR_BT_MP_SET_PARAM2, params_count);

    if (params_count != BT_PARAM2_COUNT) {
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_PARAM2, STR_BT_MP_RESULT_DELIM, FUNCTION_PARAMETER_ERROR);
        ret = FUNCTION_PARAMETER_ERROR;
    } else {
        ALOGI("mTxGainIndex:0x%02x, mTxDAC:0x%02x, mPacketHeader:0x%04x, "
              "mHoppingFixChannel:0x%02x, mHitTarget 0x%012llx",
                pBtModule->pBtParam->mTxGainIndex,
                pBtModule->pBtParam->mTxDAC,
                pBtModule->pBtParam->mPacketHeader,
                pBtModule->pBtParam->mHoppingFixChannel,
                pBtModule->pBtParam->mHitTarget
             );

        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_PARAM2, STR_BT_MP_RESULT_DELIM, BT_FUNCTION_SUCCESS);
    }

    ALOGI("--%s", STR_BT_MP_SET_PARAM2);

    return ret;
}

int BT_SetGainTable(BT_MODULE *pBtModule, char *p, char *pNotifyBuffer)
{
    char *token = NULL;
    const unsigned char BT_PARA1_COUNT = 7;
    unsigned char params_count = 0;

    ALOGI("++%s: %s", STR_BT_MP_SET_GAIN_TABLE, p);

    token = strtok(p, STR_BT_MP_PARAM_DELIM);
    if (token != NULL)
    {
        pBtModule->pBtParam->TXGainTable[0] = strtol(token, NULL, 0);
        params_count++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL)
    {
        pBtModule->pBtParam->TXGainTable[1] = strtol(token, NULL, 0);
        params_count++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL)
    {
        pBtModule->pBtParam->TXGainTable[2] = strtol(token, NULL, 0);
        params_count++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL)
    {
        pBtModule->pBtParam->TXGainTable[3] = strtol(token, NULL, 0);
        params_count++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL)
    {
        pBtModule->pBtParam->TXGainTable[4] = strtol(token, NULL, 0);
        params_count++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL)
    {
        pBtModule->pBtParam->TXGainTable[5] = strtol(token, NULL, 0);
        params_count++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL)
    {
        pBtModule->pBtParam->TXGainTable[6] = strtol(token, NULL, 0);
        params_count++;
    }
    else
    {
        goto EXIT;
    }

    //end of parameter
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL)
    {
        ALOGI("token = %s", token);
        params_count++;
    }
    else
    {
        goto EXIT;
    }

EXIT:
    ALOGI("%s: params_count = %d", STR_BT_MP_SET_GAIN_TABLE, params_count);

    if (params_count != BT_PARA1_COUNT)
    {
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_GAIN_TABLE, STR_BT_MP_RESULT_DELIM, FUNCTION_PARAMETER_ERROR);
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

        ALOGI("%s%s%x", STR_BT_MP_SET_GAIN_TABLE, STR_BT_MP_RESULT_DELIM, BT_FUNCTION_SUCCESS);
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_GAIN_TABLE, STR_BT_MP_RESULT_DELIM, BT_FUNCTION_SUCCESS);
    }


    ALOGI("--%s", STR_BT_MP_SET_GAIN_TABLE);
    return 0;
}

int BT_SetDacTable(BT_MODULE  *pBtModule, char *p, char *pNotifyBuffer)
{
    char *token = NULL;
    const unsigned char BT_PARA1_COUNT = 5;
    unsigned char params_count = 0;

    ALOGI("++%s: %s", STR_BT_MP_SET_DAC_TABLE, p);

    token = strtok(p, STR_BT_MP_PARAM_DELIM);
    if (token != NULL)
    {
        pBtModule->pBtParam->TXDACTable[0] = strtol(token, NULL, 0);
        params_count++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL)
    {
        pBtModule->pBtParam->TXDACTable[1] = strtol(token, NULL, 0);
        params_count++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL)
    {
        pBtModule->pBtParam->TXDACTable[2] = strtol(token, NULL, 0);
        params_count++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL)
    {
        pBtModule->pBtParam->TXDACTable[3] = strtol(token, NULL, 0);
        params_count++;
    }
    else
    {
        goto EXIT;
    }

    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL)
    {
        pBtModule->pBtParam->TXDACTable[4] = strtol(token, NULL, 0);
        params_count++;
    }
    else
    {
        goto EXIT;
    }

    //end of parameter
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL)
    {
        ALOGI("token = %s", token);
        params_count++;
    }
    else
    {
        goto EXIT;
    }

EXIT:
    ALOGI("%s: params_count = %d", STR_BT_MP_SET_DAC_TABLE, params_count);

    if (params_count != BT_PARA1_COUNT)
    {
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_DAC_TABLE, STR_BT_MP_RESULT_DELIM, FUNCTION_PARAMETER_ERROR);
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
        ALOGI("%s%s%x", STR_BT_MP_SET_DAC_TABLE, STR_BT_MP_RESULT_DELIM, BT_FUNCTION_SUCCESS);
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_SET_DAC_TABLE, STR_BT_MP_RESULT_DELIM, BT_FUNCTION_SUCCESS);
    }

    ALOGI("--%s", STR_BT_MP_SET_DAC_TABLE);
    return 0;
}

int BT_Exec(BT_MODULE *pBtModule, char *p, char *pNotifyBuffer)
{
    char *token = NULL;
    int ParameterIndex = 0;
    BT_PARAMETER *pBtParam = NULL;
    int rtn = BT_FUNCTION_SUCCESS;

    // index
    token = strtok(p, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        ParameterIndex = strtol(token, NULL, 0);
    } else {
        goto EXIT;
    }

    // end of parameter
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        ALOGI("BT_Exec: redundant token[%s]", token);
        rtn = FUNCTION_PARAMETER_ERROR;
        goto EXIT;
    }

    ALOGI("BT_Exec: param index[%d]", ParameterIndex);

    if (ParameterIndex > NOTTHING &&
        ParameterIndex < NUMBEROFBT_ACTIONCONTROL_TAG) {

        pBtParam = pBtModule->pBtParam;
        pBtParam->ParameterIndex = ParameterIndex;
        rtn = pBtModule->ActionControlExcute(pBtModule);

        sprintf(pNotifyBuffer, "%s%s%x%s%x",
                STR_BT_MP_EXEC,
                STR_BT_MP_RESULT_DELIM,
                ParameterIndex,
                STR_BT_MP_RESULT_DELIM,
                rtn
                );
         ALOGI("%s%s%x%s%x",
                STR_BT_MP_EXEC,
                STR_BT_MP_RESULT_DELIM,
                ParameterIndex,
                STR_BT_MP_RESULT_DELIM,
                rtn
                );
    } else {
        sprintf(pNotifyBuffer, "%s%s%s%s%x",
                STR_BT_MP_EXEC,
                STR_BT_MP_RESULT_DELIM,
                p,
                STR_BT_MP_RESULT_DELIM,
                FUNCTION_PARAMETER_ERROR
                );
        ALOGI("%s%s%s%s%x",
                STR_BT_MP_EXEC,
                STR_BT_MP_RESULT_DELIM,
                p,
                STR_BT_MP_RESULT_DELIM,
                FUNCTION_PARAMETER_ERROR
                );
    }

EXIT:
    return rtn;
}

int BT_ReportTx(BT_MODULE  *pBtModule, char* pNotifyBuffer)
{
    BT_DEVICE_REPORT BtDeviceReport;
    int rtn = BT_FUNCTION_SUCCESS;

    ALOGI("++%s", STR_BT_MP_REPORTTX);

    rtn=pBtModule->ActionReport(pBtModule, REPORT_TX, &BtDeviceReport);

    if (rtn != BT_FUNCTION_SUCCESS) {
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_REPORTTX, STR_BT_MP_RESULT_DELIM, rtn);
        ALOGI("%s%s%x", STR_BT_MP_REPORTTX, STR_BT_MP_RESULT_DELIM, rtn);
        goto exit;
    } else {
        ALOGI("%s%s%x%s%x",
                STR_BT_MP_REPORTTX,
                STR_BT_MP_RESULT_DELIM,
                BtDeviceReport.TotalTXBits,
                STR_BT_MP_RESULT_DELIM,
                BtDeviceReport.TotalTxCounts);

        sprintf(pNotifyBuffer, "%s%s%x%s%x",
                                STR_BT_MP_REPORTTX,
                                STR_BT_MP_RESULT_DELIM,
                                BtDeviceReport.TotalTXBits,
                                STR_BT_MP_RESULT_DELIM,
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
        ALOGI("%s%s%x", STR_BT_MP_PKTRXSTART, STR_BT_MP_RESULT_DELIM, rtn);
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_PKTRXSTART, STR_BT_MP_RESULT_DELIM, rtn);
        goto exit;
    } else {
        ALOGI("%s%s%x%s%x%s%x%s%x",
                STR_BT_MP_REPORTRX,
                STR_BT_MP_RESULT_DELIM,
                BtDeviceReport.RxRssi,
                STR_BT_MP_RESULT_DELIM,
                BtDeviceReport.TotalRXBits,
                STR_BT_MP_RESULT_DELIM,
                BtDeviceReport.TotalRxCounts,
                STR_BT_MP_RESULT_DELIM,
                BtDeviceReport.TotalRxErrorBits
             );

        sprintf(pNotifyBuffer, "%s%s%x%s%x%s%x%s%x",
                STR_BT_MP_REPORTRX,
                STR_BT_MP_RESULT_DELIM,
                BtDeviceReport.RxRssi,
                STR_BT_MP_RESULT_DELIM,
                BtDeviceReport.TotalRXBits,
                STR_BT_MP_RESULT_DELIM,
                BtDeviceReport.TotalRxCounts,
                STR_BT_MP_RESULT_DELIM,
                BtDeviceReport.TotalRxErrorBits
               );
    }

exit:
    ALOGI("--%s", STR_BT_MP_REPORTRX);

    return rtn;
}

int BT_RegRW(BT_MODULE *pBtModule, char *p, char *pNotifyBuffer)
{
    char *token = NULL;
    uint8_t REGRW_PARAM_COUNT = 5;// 5:read, 6:write; +1 if BB reg
    uint8_t params_count = 0;
    uint8_t type = 0;
    uint8_t rw = 0;
    uint8_t page = 0;
    uint8_t address = 0;
    uint8_t msb = 0;
    uint8_t lsb = 0;
    uint32_t data = 0;
    int rtn = BT_FUNCTION_SUCCESS;

    ALOGI("++%s: %s", STR_BT_MP_REG_RW, p);

    // uint8_t type;
    token = strtok(p, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        type = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto EXIT;
    }

    // uint8_t rw;
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        rw = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto EXIT;
    }

    // BB Reg has an extra field<PAGE>
    if (type == BB_REG) {
        // uint8_t page;
        token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
        if (token != NULL) {
            page = strtol(token, NULL, 0);
            params_count++;
        } else {
            goto EXIT;
        }
    }

    // uint16_t address
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        address = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto EXIT;
    }

    // uint8_t msb;
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        msb = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto EXIT;
    }

    // uint8_t lsb;
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        lsb = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto EXIT;
    }

    if (rw == 1) { //write
        // uint32_t dataToWrite;
        token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
        if (token != NULL) {
            data = strtol(token, NULL, 0);
            params_count++;
        } else {
            goto EXIT;
        }

    }

    // end of parameters
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        ALOGI("BT_RegRW: redundant token[%s]", token);
        params_count++;
    } else {
        goto EXIT;
    }

EXIT:
    ALOGI("%s: params_count = %d", STR_BT_MP_REG_RW, params_count);

    if (rw == 1) REGRW_PARAM_COUNT++;
    if (type == BB_REG) REGRW_PARAM_COUNT++;

    if (params_count != REGRW_PARAM_COUNT) {
        sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_REG_RW, STR_BT_MP_RESULT_DELIM, FUNCTION_PARAMETER_ERROR);
    } else {
        ALOGI("BT_RegRW: type 0x%x, rw 0x%x, page 0x%x, address 0x%04x, msb 0x%x, lsb 0x%x, data 0x%08x",
                type, rw, page, address, msb, lsb, data);

        if (rw == 0) {
            rtn = pBtModule->GetRegMaskBits(pBtModule, type, page, address, msb, lsb, &data);
            sprintf(pNotifyBuffer, "%s%s%x%s%08x", STR_BT_MP_REG_RW, STR_BT_MP_RESULT_DELIM, rtn,
                    STR_BT_MP_RESULT_DELIM, data);
        } else {
            rtn = pBtModule->SetRegMaskBits(pBtModule, type, page, address, msb, lsb, data);
            sprintf(pNotifyBuffer, "%s%s%x", STR_BT_MP_REG_RW, STR_BT_MP_RESULT_DELIM, rtn);
        }

    }

    ALOGI("--%s", STR_BT_MP_REG_RW);
    return rtn;
}

void BT_GetBDAddr(BT_MODULE  *pBtModule)
{
    uint8_t pEvt[256];
    uint32_t EvtLen = 0;
    uint8_t para[256];

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

    pBtModule->pBtParam->mChannelNumber = DEFAULT_CH_NUM;
    pBtModule->pBtParam->mPacketType = DEFAULT_PKT_TYPE;
    pBtModule->pBtParam->mPayloadType = DEFAULT_PAYLOAD_TYPE;
    pBtModule->pBtParam->mTxPacketCount = DEFAULT_PKT_COUNT;
    pBtModule->pBtParam->mTxGainValue = DEFAULT_TX_GAIN_VALUE;
    pBtModule->pBtParam->mWhiteningCoeffValue = DEFAULT_WHITE_COEFF_VALUE;
    pBtModule->pBtParam->mTxGainIndex = DEFAULT_TX_GAIN_INDEX;
    pBtModule->pBtParam->mTxDAC = DEFAULT_TX_DAC;
    pBtModule->pBtParam->mPacketHeader = DEFAULT_PKTHEADER;
    pBtModule->pBtParam->mHoppingFixChannel = DEFAULT_HOPPING_CH_NUM;
    pBtModule->pBtParam->mHitTarget = 0x0000009e8b33;
}
