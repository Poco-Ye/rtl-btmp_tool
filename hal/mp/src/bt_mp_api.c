#define LOG_TAG "btif_mp_api"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <termios.h>
#include <time.h>
#include <inttypes.h>

#include "bt_syslog.h"
#include "bluetoothmp.h"
#include "bt_mp_transport.h"
#include "bt_mp_api.h"
#include "bt_mp_base.h"
#include "bt_mp_build.h"
#include "bt_mp_device_base.h"
#include "bt_mp_module_base.h"
#include "foundation.h"

#include "btif_api.h"

#include "gki.h"
#include "btu.h"

#include "hcidefs.h"
#include "hcimsgs.h"
#include "btif_common.h"


#define DEFAULT_HIT_ADDRESS                 0x0000009E8B33
#define DEFAULT_CH_NUM                      0
#define DEFAULT_PKT_TYPE                    BT_PKT_1DH1
#define DEFAULT_PAYLOAD_TYPE                BT_PAYLOAD_TYPE_ALL0
#define DEFAULT_PKT_COUNT                   0
#define DEFAULT_TX_GAIN_VALUE               0xCE
#define DEFAULT_WHITE_COEFF_VALUE           0x7F

#define DEFAULT_TX_GAIN_INDEX               0x06
#define DEFAULT_TEST_MODE                   BT_PSEUDO_MODE
#define DEFAULT_TX_DAC                      0x13
#define DEFAULT_PKT_HEADER                  0x1234
#define DEFAULT_HOPPING_CH_NUM              0
#define DEFAULT_MULTI_RX_ENABLE             0

#define BT_PARAM_IDX0    0   //mPGRawData
#define BT_PARAM_IDX1    1   //mChannelNumber
#define BT_PARAM_IDX2    2   //mPacketType
#define BT_PARAM_IDX3    3   //mPayloadType
#define BT_PARAM_IDX4    4   //mTxPacketCount
#define BT_PARAM_IDX5    5   //mTxGainValue
#define BT_PARAM_IDX6    6   //mWhiteningCoeffValue
#define BT_PARAM_IDX7    7   //mTxGainIndex
#define BT_PARAM_IDX8    8   //mTxDAC
#define BT_PARAM_IDX9    9   //mPacketHeader
#define BT_PARAM_IDX10   10  //mHoppingFixChannel
#define BT_PARAM_IDX11   11  //mHitTarget
#define BT_PARAM_IDX12   12  //TXGainTable
#define BT_PARAM_IDX13   13  //TXDACTable
#define BT_PARAM_IDX14   14  //Xtal
#define BT_PARAM_IDX15   15  //mLEDataLen
#define BT_PARAM_IDX_NUM 16

static void bt_index2param(BT_MODULE *pBtModule, int index, int64_t value)
{
    switch (index) {
    case BT_PARAM_IDX0:
        pBtModule->pBtParam->mPGRawData[0] = (uint8_t)value;
        break;
    case BT_PARAM_IDX1:
        pBtModule->pBtParam->mChannelNumber = (uint8_t)value;
        break;
    case BT_PARAM_IDX2:
        pBtModule->pBtParam->mPacketType = (BT_PKT_TYPE)value;
        break;
    case BT_PARAM_IDX3:
        pBtModule->pBtParam->mPayloadType = (BT_PAYLOAD_TYPE)value;
        break;
    case BT_PARAM_IDX4:
        pBtModule->pBtParam->mTxPacketCount = (uint16_t)value;
        break;
    case BT_PARAM_IDX5:
        pBtModule->pBtParam->mTxGainValue = (uint8_t)value;
        break;
    case BT_PARAM_IDX6:
        pBtModule->pBtParam->mWhiteningCoeffValue = (uint8_t)value;
        break;
    case BT_PARAM_IDX7:
        pBtModule->pBtParam->mTxGainIndex = (uint8_t)value;
        break;
    case BT_PARAM_IDX8:
        pBtModule->pBtParam->mTxDAC = (uint8_t)value;
        break;
    case BT_PARAM_IDX9:
        pBtModule->pBtParam->mPacketHeader = (uint32_t)value;
        break;
    case BT_PARAM_IDX10:
        pBtModule->pBtParam->mHoppingFixChannel = (uint8_t)value;
        break;
    case BT_PARAM_IDX11:
        pBtModule->pBtParam->mHitTarget = (uint64_t)value;
        break;
    case BT_PARAM_IDX12:
        pBtModule->pBtParam->TXGainTable[0] = (uint8_t)value;
        break;
    case BT_PARAM_IDX13:
        pBtModule->pBtParam->TXDACTable[0] = (uint8_t)value;
        break;
    case BT_PARAM_IDX14:
        pBtModule->pBtParam->Rtl8761Xtal = (uint16_t)value;
        break;
    case BT_PARAM_IDX15:
        pBtModule->pBtParam->mParamData[0] = (uint8_t)value; //mLEDataLen
        break;
    default:
        break;
    }
}

static void bt_index2print(BT_MODULE *pBtModule, int index, char *buf_cb)
{
    char pair_str[6];
    uint8_t i, len;

    switch (index) {
    case BT_PARAM_IDX0:
        len = pBtModule->pBtParam->mPGRawData[1];

        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%02x%s0x%02x",
                STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                index, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mPGRawData[0], STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mPGRawData[1]);

        for (i = 0; i < len; i++) {
            sprintf(pair_str, "%s0x%02x", STR_BT_MP_RESULT_DELIM, pBtModule->pBtParam->mPGRawData[2+i]);
            strcat(buf_cb, pair_str);
        }
        break;
    case BT_PARAM_IDX1:
        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%02x",
                STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                index, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mChannelNumber);
        break;
    case BT_PARAM_IDX2:
        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%02x",
                STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                index, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mPacketType);
        break;
    case BT_PARAM_IDX3:
        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%02x",
                STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                index, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mPayloadType);
        break;
    case BT_PARAM_IDX4:
        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%04x",
                STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                index, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mTxPacketCount);
        break;
    case BT_PARAM_IDX5:
        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%02x",
                STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                index, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mTxGainValue);
        break;
    case BT_PARAM_IDX6:
        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%02x",
                STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                index, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mWhiteningCoeffValue);
        break;
    case BT_PARAM_IDX7:
        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%02x",
                STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                index, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mTxGainIndex);
        break;
    case BT_PARAM_IDX8:
        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%02x",
                STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                index, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mTxDAC);
        break;
    case BT_PARAM_IDX9:
        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%08x",
                STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                index, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mPacketHeader);
        break;
    case BT_PARAM_IDX10:
        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%02x",
                STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                index, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mHoppingFixChannel);
        break;
    case BT_PARAM_IDX11:
        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%012"PRIx64"",
                STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                index, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mHitTarget);
        break;
    case BT_PARAM_IDX12:
        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%02x",
                STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                index, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->TXGainTable[0], STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->TXGainTable[1], STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->TXGainTable[2], STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->TXGainTable[3], STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->TXGainTable[4], STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->TXGainTable[5], STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->TXGainTable[6]);
        break;
    case BT_PARAM_IDX13:
        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%02x",
                STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                index, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->TXDACTable[0], STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->TXDACTable[1], STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->TXDACTable[2], STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->TXDACTable[3], STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->TXDACTable[4]);
        break;
    case BT_PARAM_IDX14:
        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%04x",
                STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                index, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->Rtl8761Xtal);
        break;
    case BT_PARAM_IDX15:
        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%02x",
                STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                index, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mParamData[0]);
        break;
    default:
        break;
    }
}

static void bt_item2print(BT_DEVICE_REPORT *pBtDeviceReport, int item, char *buf_cb)
{
    uint8_t efuse_len = pBtDeviceReport->ReportData[3] + 4;
    char efuse_str[6] = {0};
    uint8_t i;

    switch (item) {
    case REPORT_ALL:
        break;
    case REPORT_PKT_TX:
    case REPORT_CONT_TX:
        SYSLOGI("%s%s%d%s0x%02x%s0x%08x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTXBits, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTxCounts);

        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%08x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTXBits, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTxCounts);
        break;

    case REPORT_PKT_RX:
        SYSLOGI("%s%s%d%s0x%02x%s%d%s0x%08x%s0x%08x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->RxRssi, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalRXBits, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalRxCounts, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalRxErrorBits);

        sprintf(buf_cb, "%s%s%d%s0x%02x%s%d%s0x%08x%s0x%08x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->RxRssi, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalRXBits, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalRxCounts, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalRxErrorBits);
        break;

    case REPORT_TX_GAIN_TABLE:
        SYSLOGI("%s%s%d%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%02x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXGainTable[0], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXGainTable[1], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXGainTable[2], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXGainTable[3], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXGainTable[4], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXGainTable[5], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXGainTable[6]);

        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%02x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXGainTable[0], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXGainTable[1], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXGainTable[2], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXGainTable[3], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXGainTable[4], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXGainTable[5], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXGainTable[6]);
        break;

    case REPORT_TX_DAC_TABLE:
        SYSLOGI("%s%s%d%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%02x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXDACTable[0], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXDACTable[1], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXDACTable[2], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXDACTable[3], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXDACTable[4]);

        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%02x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXDACTable[0], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXDACTable[1], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXDACTable[2], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXDACTable[3], STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrTXDACTable[4]);
        break;

    case REPORT_XTAL:
        SYSLOGI("%s%s%d%s0x%02x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrRtl8761Xtal);

        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrRtl8761Xtal);
        break;

    case REPORT_THERMAL:
        SYSLOGI("%s%s%d%s0x%02x%s0x%02x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrThermalValue);

        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%02x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrThermalValue);
        break;

    case REPORT_BT_STAGE:
        SYSLOGI("%s%s%d%s0x%02x%s0x%02x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrStage);

        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%02x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->CurrStage);
        break;

    case REPORT_CHIP:
        SYSLOGI("%s%s%d%s0x%02x%s0x%08x%s0x%08x%s0x%08x%s0x%08x%s0x%08x%s0x%08x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->pBTInfo->ChipType, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->pBTInfo->HCI_SubVersion, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->pBTInfo->HCI_Version, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->pBTInfo->Is_After_PatchCode, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->pBTInfo->LMP_SubVersion, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->pBTInfo->LMP_Version, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->pBTInfo->Version);

        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%08x%s0x%08x%s0x%08x%s0x%08x%s0x%08x%s0x%08x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->pBTInfo->ChipType, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->pBTInfo->HCI_SubVersion, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->pBTInfo->HCI_Version, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->pBTInfo->Is_After_PatchCode, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->pBTInfo->LMP_SubVersion, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->pBTInfo->LMP_Version, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->pBTInfo->Version);
        break;

    case REPORT_LOGICAL_EFUSE:
        sprintf(buf_cb, "%s%s%d%s0x%02x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS);

        for (i = 0; i < efuse_len; i++) {
            sprintf(efuse_str, "%s0x%02x", STR_BT_MP_RESULT_DELIM, pBtDeviceReport->ReportData[i]);
            strcat(buf_cb, efuse_str);
        }

        SYSLOGI("%s", buf_cb);
        break;

    case REPORT_LE_RX:
        SYSLOGI("%s%s%d%s0x%02x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalRxCounts);

        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalRxCounts);
        break;

    case REPORT_LE_CONTINUE_TX:
        SYSLOGI("%s%s%d%s0x%02x%s0x%08x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTXBits, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTxCounts);

        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%08x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTXBits, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTxCounts);

        break;

    case REPORT_FW_PACKET_TX:
        SYSLOGI("%s%s%d%s0x%02x%s0x%08x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTXBits, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTxCounts);

        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%08x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTXBits, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTxCounts);

        break;

    case REPORT_FW_CONTINUE_TX:
        SYSLOGI("%s%s%d%s0x%02x%s0x%08x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTXBits, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTxCounts);

        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%08x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTXBits, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTxCounts);

        break;

    case REPORT_FW_PACKET_RX:
        SYSLOGI("%s%s%d%s0x%02x%s%d%s0x%08x%s0x%08x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->RxRssi, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalRXBits, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalRxCounts, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalRxErrorBits);

        sprintf(buf_cb, "%s%s%d%s0x%02x%s%d%s0x%08x%s0x%08x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->RxRssi, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalRXBits, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalRxCounts, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalRxErrorBits);

        break;

    case REPORT_FW_LE_CONTINUE_TX:
        SYSLOGI("%s%s%d%s0x%02x%s0x%08x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTXBits, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTxCounts);

        sprintf(buf_cb, "%s%s%d%s0x%02x%s0x%08x%s0x%08x",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTXBits, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->TotalTxCounts);

        break;

    case REPORT_TX_POWER_INFO:
        SYSLOGI("%s%s%d%s0x%02x%s%d%s%d%s%d%s%d%s%d",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->ReportData[0],STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->ReportData[1],STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->ReportData[2],STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->ReportData[3],STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->ReportData[4]);

        sprintf(buf_cb, "%s%s%d%s0x%02x%s%d%s%d%s%d%s%d%s%d",
                STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
                item, STR_BT_MP_RESULT_DELIM,
                BT_FUNCTION_SUCCESS, STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->ReportData[0],STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->ReportData[1],STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->ReportData[2],STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->ReportData[3],STR_BT_MP_RESULT_DELIM,
                pBtDeviceReport->ReportData[4]);
        break;

    default:
        break;
    }
}

int BT_SendHciCmd(BT_MODULE *pBtModule, char *p, char *buf_cb)
{
    int ret = BT_FUNCTION_SUCCESS;
    char *token = NULL;
    uint16_t OpCode = 0;
    uint8_t ParamLen = 0;
    uint8_t ParamLen_1 = 0;
    uint8_t ParamArray[255];
    uint8_t pEvent[255] = {0};
    char evt_str[6] = {0};
    uint32_t EventLen = 0;
    uint8_t params_count = 0;
    uint8_t i = 0;

    SYSLOGI("++%s: %s", STR_BT_MP_HCI_CMD, p);

    token = strtok(p, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        OpCode = strtol(token, NULL, 0);
        params_count++;
    } else {
        ret = FUNCTION_PARAMETER_ERROR;
        goto exit;
    }

    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        ParamLen = strtol(token, NULL, 0);
        params_count++;
    } else {
        ret = FUNCTION_PARAMETER_ERROR;
        goto exit;
    }

    ParamLen_1 = ParamLen;
    while (ParamLen_1--) {
        token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
        if (token != NULL) {
            ParamArray[i++] = strtol(token, NULL, 0);
            params_count++;
        } else {
            ret = FUNCTION_PARAMETER_ERROR;
            goto exit;
        }
    }

    if (params_count != ParamLen + 2) {
        ret = FUNCTION_PARAMETER_ERROR;
        goto exit;
    }

    ret = pBtModule->SendHciCommandWithEvent(pBtModule, OpCode, ParamLen, ParamArray, 0x0E, pEvent, &EventLen);
    if (ret == BT_FUNCTION_SUCCESS) {
        sprintf(buf_cb, "%s", STR_BT_MP_HCI_CMD);
        for (i = 0; i < EventLen; i++) {
            sprintf(evt_str, "%s0x%02x", STR_BT_MP_RESULT_DELIM, pEvent[i]);
            strcat(buf_cb, evt_str);
        }
    } else {
        goto exit;
    }

    SYSLOGI("--%s%s0x%02x", STR_BT_MP_HCI_CMD, STR_BT_MP_RESULT_DELIM, ret);
    return ret;

exit:
    sprintf(buf_cb, "%s%s0x%02x", STR_BT_MP_HCI_CMD, STR_BT_MP_RESULT_DELIM, FUNCTION_PARAMETER_ERROR);
    return ret;
}

int BT_GetParam(BT_MODULE *pBtModule, char *p, char *buf_cb)
{
    char *token = NULL;
    char *endptr = NULL;
    int index = -1;
    int ret = BT_FUNCTION_SUCCESS;

    SYSLOGI("++%s: index %s", STR_BT_MP_GET_PARAM, p);

    token = strtok(p, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        index = strtol(token, &endptr, 0);
        if (*endptr || index < 0 || index >= BT_PARAM_IDX_NUM) {
            if (*endptr) {
                index = -1;
            }

            SYSLOGI("%s%s%d%s0x%02x",
                    STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                    index, STR_BT_MP_RESULT_DELIM,
                    FUNCTION_PARAMETER_ERROR);

            sprintf(buf_cb, "%s%s%d%s0x%02x",
                    STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                    index, STR_BT_MP_RESULT_DELIM,
                    FUNCTION_PARAMETER_ERROR);

            return FUNCTION_PARAMETER_ERROR;
        }

        bt_index2print(pBtModule, index, buf_cb);
    } else {
        SYSLOGW("%s: Param index not specified", STR_BT_MP_GET_PARAM);

        SYSLOGI("%s%s0x%02x%s0x%02x%s0x%02x%s0x%04x%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%08x%s0x%02x%s0x%012"PRIx64"%s0x%02x",
                STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mChannelNumber, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mPacketType, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mPayloadType, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mTxPacketCount, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mTxGainValue, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mWhiteningCoeffValue, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mTxGainIndex, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mTxDAC, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mPacketHeader, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mHoppingFixChannel, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mHitTarget, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mParamData[0]);

        sprintf(buf_cb,
                "%s%s0x%02x%s0x%02x%s0x%02x%s0x%04x%s0x%02x%s0x%02x%s0x%02x%s0x%02x%s0x%08x%s0x%02x%s0x%012"PRIx64"%s0x%02x",
                STR_BT_MP_GET_PARAM, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mChannelNumber, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mPacketType, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mPayloadType, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mTxPacketCount, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mTxGainValue, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mWhiteningCoeffValue, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mTxGainIndex, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mTxDAC, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mPacketHeader, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mHoppingFixChannel, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mHitTarget, STR_BT_MP_RESULT_DELIM,
                pBtModule->pBtParam->mParamData[0]);
    }

    SYSLOGI("--%s", STR_BT_MP_GET_PARAM);
    return ret;
}

int BT_SetParam(BT_MODULE *pBtModule, char *p, char *buf_cb)
{
    uint16_t pairs_count, params_count;
    char *pair_token, *param_token;
    char *pairs_buf, *params_buf;
    char *save_pairs, *save_params;
    int index = -1;
    int var_pair;
    int64_t value = 0;

    SYSLOGI("++%s: %s", STR_BT_MP_SET_PARAM, p);

    for (pairs_count = 0, pairs_buf = p; ; pairs_count++, pairs_buf = NULL) {
        pair_token = strtok_r(pairs_buf, STR_BT_MP_PAIR_DELIM, &save_pairs);
        if (pair_token == NULL)
            break;

        var_pair = 0; // set 1 if variable pair

        for (params_count = 0, params_buf = pair_token; ; params_count++, params_buf = NULL) {
            param_token = strtok_r(params_buf, STR_BT_MP_PARAM_DELIM, &save_params);
            if (param_token && params_count == 0) {
                index = strtol(param_token, NULL, 0);
                if (index < BT_PARAM_IDX0 || index >= BT_PARAM_IDX_NUM) {
                    SYSLOGI("Invalid BT param index %d", index);
                    sprintf(buf_cb, "%s%s%d%s0x%02x",
                            STR_BT_MP_SET_PARAM, STR_BT_MP_RESULT_DELIM,
                            index, STR_BT_MP_RESULT_DELIM,
                            FUNCTION_PARAMETER_ERROR);
                    return FUNCTION_PARAMETER_ERROR;
                } else if (index == BT_PARAM_IDX0 || index == BT_PARAM_IDX12 || index == BT_PARAM_IDX13) {
                    var_pair = 1;
                }
            } else if (param_token && params_count == 1) {
                value = strtoll(param_token, NULL, 0);
            } else if (param_token && params_count > 1 && var_pair == 1) {
                if (index == BT_PARAM_IDX0)
                    pBtModule->pBtParam->mPGRawData[params_count - 1] = (uint8_t)strtoll(param_token, NULL, 0);
                else if (index == BT_PARAM_IDX12 && params_count <= MAX_TXGAIN_TABLE_SIZE)
                    pBtModule->pBtParam->TXGainTable[params_count - 1] = (uint8_t)strtoll(param_token, NULL, 0);
                else if (index == BT_PARAM_IDX13 && params_count <= MAX_TXDAC_TABLE_SIZE)
                    pBtModule->pBtParam->TXDACTable[params_count - 1] = (uint8_t)strtoll(param_token, NULL, 0);
            } else if (param_token == NULL) // null token OR token parsing completed
                break;
        }

        if (params_count > 2 && var_pair == 1) {
            bt_index2param(pBtModule, index, value);
            uint16_t i = 0;
            for (i = 0; i < params_count - 1; i++) {
                if (index == BT_PARAM_IDX0) // variable pair format<index, cmd, len, data...>
                    SYSLOGI("PG raw data[%d]: 0x%02x", i, pBtModule->pBtParam->mPGRawData[i]);
                else if (index == BT_PARAM_IDX12)
                    SYSLOGI("TX gain table[%d]: 0x%02x", i, pBtModule->pBtParam->TXGainTable[i]);
                else if (index == BT_PARAM_IDX13)
                    SYSLOGI("TX dac table[%d]: 0x%02x", i, pBtModule->pBtParam->TXDACTable[i]);
            }
        } else if (params_count == 2 && var_pair == 0) { // 2-param pair format<index, value>
            SYSLOGI("Pair index %d, pair value 0x%llx", index, value);
            bt_index2param(pBtModule, index, value);
        } else if (params_count == 0) { // null pair
            continue;
        } else { // wrong pair format
            SYSLOGI("Invalid BT pair format, params count %d", params_count);
            sprintf(buf_cb, "%s%s%d%s0x%02x",
                    STR_BT_MP_SET_PARAM, STR_BT_MP_RESULT_DELIM,
                    index, STR_BT_MP_RESULT_DELIM,
                    FUNCTION_PARAMETER_ERROR);
            return FUNCTION_PARAMETER_ERROR;
        }
    }

    SYSLOGI("--%s: pairs count %d", STR_BT_MP_SET_PARAM, pairs_count);

    sprintf(buf_cb, "%s%s%d%s0x%02x",
            STR_BT_MP_SET_PARAM, STR_BT_MP_RESULT_DELIM,
            index, STR_BT_MP_RESULT_DELIM,
            BT_FUNCTION_SUCCESS);

    return BT_FUNCTION_SUCCESS;
}

/**
 * Set Config I/F has two pairs.
 * First pair: file_path,flag,type,option.
 *  file_path: a string to file system directory.
 *  flag: 0 for APPEND, 1 for TRUNC, 2 for DELETE.
 *  type: 0 for stream/raw data, 1 for formatted data.
 *  option: 0 for reserved(default), 1 for random mac address,
 *          and others to be defined.
 * Second pair: char string for stream/raw data.
 *              byte sequence for formatted data.
 */
int BT_SetConfig(BT_MODULE *pBtModule, char *p, char *buf_cb)
{
    uint16_t pairs_count, params_count;
    char *pair_token, *param_token;
    char *pairs_buf, *params_buf;
    char *save_pairs, *save_params;
    char file_path[128];
    char *buffer = file_path;
    uint8_t flag;
    uint8_t type;
    uint8_t option;
    int fd = -1;
    ssize_t count;
    uint16_t append_bytes = 0;

    SYSLOGI("++%s: %s", STR_BT_MP_SET_CONFIG, p);

    for (pairs_count = 0, pairs_buf = p; ; pairs_count++, pairs_buf = NULL) {
        pair_token = strtok_r(pairs_buf, STR_BT_MP_PAIR_DELIM, &save_pairs);
        if (!pair_token) {
            if (pairs_count == 0) {
                SYSLOGE("Invalid config pair format<%s>", pair_token);
                goto param_err;
            } else
                break;
        }

        // First pair must be <file_path,flag,type,option>
        if (pairs_count == 0) {
            for (params_count = 0, params_buf = pair_token; ; params_count++, params_buf = NULL) {
                param_token = strtok_r(params_buf, STR_BT_MP_PARAM_DELIM, &save_params);
                if (!param_token) // null token OR token parsing completed
                    break;

                switch (params_count) {
                case 0: // file_path
                    strncpy(file_path, param_token, 127);
                    file_path[127] = '\0';
                    break;
                case 1: // flag
                    flag = (uint8_t)strtoul(param_token, NULL, 0);
                    if (flag > 2) {
                        SYSLOGE("Invalid file flag %d", flag);
                        goto param_err;
                    }
                    break;
                case 2: // type
                    type = (uint8_t)strtoul(param_token, NULL, 0);
                    if (type > 1) {
                        SYSLOGE("Invalid file type %d", type);
                        goto param_err;
                    }
                    break;
                case 3: // option
                    option = (uint8_t)strtoul(param_token, NULL, 0);
                    if (option > 1) {
                        SYSLOGE("Invalid file option %d", option);
                        goto param_err;
                    }
                    break;
                default:
                    SYSLOGE("Invalid config pair format<%s>", pair_token);
                    goto param_err;
                }
            }

            if (params_count == 4) {
                int file_flags = O_RDWR | O_CREAT;
                int file_exist;

                // as we may use lseek & write, O_APPEND shouldn't be set
                // when flag equals 0 or 2.
                if (flag == 1)
                    file_flags |= O_TRUNC;

                file_exist = access(file_path, F_OK);
                if (file_exist < 0 && flag == 2) {
                    SYSLOGE("Failed to delete contents: %s", strerror(errno));
                    goto op_err;
                }

                fd = open(file_path, file_flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if (fd >= 0) {
                    if (flag == 0) {
                        count = lseek(fd, 0, SEEK_END);
                        if (count < 0) {
                            SYSLOGE("Failed to seek config file<%s>", strerror(errno));
                            close(fd);
                            goto op_err;
                        }
                    }

                    if ((file_exist < 0 || flag == 1) && type == 1) {
                        // set config file header
                        uint8_t header[] = {0x55,0xab,0x23,0x87,0x00,0x00};

                        count = write(fd, header, sizeof(header));
                        if (count < 0) {
                            SYSLOGE("Failed to write config file<%s>", strerror(errno));
                            close(fd);
                            goto op_err;
                        }
                    }
                } else {
                    SYSLOGE("Failed to open config file: %s", strerror(errno));
                    goto op_err;
                }
            } else {
                SYSLOGE("Invalid config pair format<%s>", pair_token);
                goto param_err;
            }
        } else {
            if (type == 0) {
                // we treat all data following first pair delim as stream
                count = write(fd, pair_token, strlen(pair_token));
                if (count < 0) {
                    SYSLOGE("Failed to write config file<%s>", strerror(errno));
                    close(fd);
                    goto op_err;
                }
            } else if (type == 1) {
                for (params_count = 0, params_buf = pair_token; ; params_count++, params_buf = NULL) {
                    param_token = strtok_r(params_buf, STR_BT_MP_PARAM_DELIM, &save_params);
                    if (param_token)
                        buffer[params_count] = strtol(param_token, NULL, 0);
                    else
                        break;
                }

                count = write(fd, buffer, params_count);
                if (count < 0) {
                    SYSLOGE("Failed to write config file<%s>", strerror(errno));
                    close(fd);
                    goto op_err;
                } else
                    append_bytes = (uint16_t)count;
            }
        }
    }

    // generate random mac address
    if (option == 1) {
        int i;
        uint8_t btaddr[9]; // 3 btyes for offset & len
mac_gen:
        for (i = 0; i < 6; i++) {
            srand((unsigned int)time(NULL) * getpid() + random());
            btaddr[i+3] = (uint8_t)rand();
        }
        // reserve LAP addr from 0x9e8b00 to 0x9e8b3f
        if (btaddr[5] == 0x9e && btaddr[4] == 0x8b && (btaddr[3] <= 0x3f))
            goto mac_gen;

        if (type == 0) {
            char btaddr_str[18];
            sprintf(btaddr_str, "%02X:%02X:%02X:%02X:%02X:%02X",
                    btaddr[3], btaddr[4], btaddr[5], btaddr[6], btaddr[7], btaddr[8]);
            count = write(fd, btaddr_str, 17);
            if (count < 0) {
                SYSLOGE("Failed to write config file<%s>", strerror(errno));
                close(fd);
                goto op_err;
            }
        } else if (type == 1) {
            btaddr[0] = 0x3c;
            btaddr[1] = 0x00;
            btaddr[2] = 0x06;
            count = write(fd, btaddr, 9);
            if (count < 0) {
                SYSLOGE("Failed to write config file<%s>", strerror(errno));
                close(fd);
                goto op_err;
            } else
                append_bytes += (uint16_t)count;
        }
    }

    // update config file len
    if (type == 1) {
        uint16_t cfg_len;
        count = lseek(fd, 4, SEEK_SET);
        if (count >= 0) {
            count = read(fd, &cfg_len, 2);
            if (count > 0)
                cfg_len += append_bytes;
            else {
                SYSLOGE("Failed to read config file<%s>", strerror(errno));
                close(fd);
                goto op_err;
            }
        } else {
            SYSLOGE("Failed to seek config file<%s>", strerror(errno));
            close(fd);
            goto op_err;
        }

        count = lseek(fd, 4, SEEK_SET);
        if (count >= 0) {
            count = write(fd, &cfg_len, 2);
            if (count < 0) {
                SYSLOGE("Failed to write config file<%s>", strerror(errno));
                close(fd);
                goto op_err;
            }
        } else {
            SYSLOGE("Failed to seek config file<%s>", strerror(errno));
            close(fd);
            goto op_err;
        }
    }

    close(fd);

    SYSLOGI("--%s: pairs count %d", STR_BT_MP_SET_CONFIG, pairs_count);

    sprintf(buf_cb, "%s%s0x%02x",
            STR_BT_MP_SET_CONFIG, STR_BT_MP_RESULT_DELIM,
            BT_FUNCTION_SUCCESS);

    return BT_FUNCTION_SUCCESS;

param_err:
    sprintf(buf_cb, "%s%s0x%02x",
            STR_BT_MP_SET_CONFIG, STR_BT_MP_RESULT_DELIM,
            FUNCTION_PARAMETER_ERROR);

    return FUNCTION_PARAMETER_ERROR;

op_err:
    sprintf(buf_cb, "%s%s0x%02x",
            STR_BT_MP_SET_CONFIG, STR_BT_MP_RESULT_DELIM,
            FUNCTION_ERROR);

    return FUNCTION_ERROR;
}

int BT_Exec(BT_MODULE *pBtModule, char *p, char *buf_cb)
{
    char *token = NULL;
    char *endptr = NULL;
    int action_index = -1;
    BT_PARAMETER *pBtParam = NULL;
    int ret = BT_FUNCTION_SUCCESS;

    SYSLOGI("++%s: action index[%s]", STR_BT_MP_EXEC, p);

    // index
    token = strtok(p, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        action_index = strtol(token, &endptr, 0);
        if (*endptr || action_index < 0 || action_index >= BT_ACTION_NUM) {
            if (*endptr) {
                action_index = -1;
            }
            ret = FUNCTION_PARAMETER_ERROR;
            goto exit;
        }
    } else {
        action_index = -1;
        ret = FUNCTION_PARAMETER_ERROR;
        goto exit;
    }

    // end of parameter
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        SYSLOGI("BT_Exec: redundant token[%s]", token);
        ret = FUNCTION_PARAMETER_ERROR;
        goto exit;
    }

    pBtParam = pBtModule->pBtParam;
    pBtParam->ParameterIndex = action_index;

    ret = pBtModule->ActionControlExcute(pBtModule);
    if (ret == BT_FUNCTION_SUCCESS) {
        SYSLOGI("%s%s%d%s0x%02x",
                STR_BT_MP_EXEC, STR_BT_MP_RESULT_DELIM,
                action_index, STR_BT_MP_RESULT_DELIM,
                ret);

        sprintf(buf_cb, "%s%s%d%s0x%02x",
                STR_BT_MP_EXEC, STR_BT_MP_RESULT_DELIM,
                action_index, STR_BT_MP_RESULT_DELIM,
                ret);
    } else {
        goto exit;
    }

    SYSLOGI("--%s: action index[%d]", STR_BT_MP_EXEC, action_index);
    return ret;

exit:
    SYSLOGI("%s%s%d%s0x%02x",
            STR_BT_MP_EXEC, STR_BT_MP_RESULT_DELIM,
            action_index, STR_BT_MP_RESULT_DELIM,
            ret);

    sprintf(buf_cb, "%s%s%d%s0x%02x",
            STR_BT_MP_EXEC, STR_BT_MP_RESULT_DELIM,
            action_index, STR_BT_MP_RESULT_DELIM,
            ret);

    return ret;
}

int BT_Report(BT_MODULE *pBtModule, char *p, char *buf_cb)
{
    BT_DEVICE_REPORT BtDeviceReport;
    char *token = NULL;
    char *endptr = NULL;
    int report_item = -1;
    int ret = BT_FUNCTION_SUCCESS;

    SYSLOGI("++%s: %s", STR_BT_MP_REPORT, p);

    token = strtok(p, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        report_item = strtol(token, &endptr, 0);
        if (*endptr) {
            report_item = -1;
            ret = FUNCTION_PARAMETER_ERROR;
            goto exit;
        }
    } else {
        report_item = -1;
        ret = FUNCTION_PARAMETER_ERROR;
        goto exit;
    }

    ret = pBtModule->ActionReport(pBtModule, report_item, &BtDeviceReport);

    if (ret == BT_FUNCTION_SUCCESS) {
        bt_item2print(&BtDeviceReport, report_item, buf_cb);
    } else {
        goto exit;
    }

    SYSLOGI("--%s", STR_BT_MP_REPORT);
    return ret;

exit:
    SYSLOGI("%s%s%d%s0x%02x",
            STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
            report_item, STR_BT_MP_RESULT_DELIM,
            ret);

    sprintf(buf_cb, "%s%s%d%s0x%02x",
            STR_BT_MP_REPORT, STR_BT_MP_RESULT_DELIM,
            report_item, STR_BT_MP_RESULT_DELIM,
            ret);

    return ret;
}

int BT_RegRW(BT_MODULE *pBtModule, char *p, char *buf_cb)
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
    uint16_t data = 0;
    int ret = BT_FUNCTION_SUCCESS;

    SYSLOGI("++%s: %s", STR_BT_MP_REG_RW, p);

    // uint8_t type;
    token = strtok(p, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        type = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto exit;
    }

    // uint8_t rw;
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        rw = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto exit;
    }

    // BB Reg has an extra field<PAGE>
    if (type == BB_REG) {
        // uint8_t page;
        token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
        if (token != NULL) {
            page = strtol(token, NULL, 0);
            params_count++;
        } else {
            goto exit;
        }
    }

    // uint16_t address
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        address = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto exit;
    }

    // uint8_t msb;
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        msb = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto exit;
    }

    // uint8_t lsb;
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        lsb = strtol(token, NULL, 0);
        params_count++;
    } else {
        goto exit;
    }

    if (rw == 1) { //write
        // uint16_t dataToWrite;
        token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
        if (token != NULL) {
            data = strtol(token, NULL, 0);
            params_count++;
        } else {
            goto exit;
        }

    }

    // end of parameters
    token = strtok(NULL, STR_BT_MP_PARAM_DELIM);
    if (token != NULL) {
        SYSLOGI("BT_RegRW: redundant token[%s]", token);
        params_count++;
    } else {
        goto exit;
    }

exit:
    SYSLOGI("%s: params_count = %d", STR_BT_MP_REG_RW, params_count);

    if (rw == 1) REGRW_PARAM_COUNT++;
    if (type == BB_REG) REGRW_PARAM_COUNT++;

    if (params_count != REGRW_PARAM_COUNT) {
        sprintf(buf_cb, "%s%s0x%02x", STR_BT_MP_REG_RW, STR_BT_MP_RESULT_DELIM, FUNCTION_PARAMETER_ERROR);
    } else {
        SYSLOGI("BT_RegRW: type 0x%x, rw 0x%x, page 0x%x, address 0x%04x, msb 0x%x, lsb 0x%x, data 0x%04x",
                type, rw, page, address, msb, lsb, data);

        if (rw == 0) {
            ret = pBtModule->GetRegMaskBits(pBtModule, type, page, address, msb, lsb, &data);
            sprintf(buf_cb, "%s%s0x%02x%s0x%04x", STR_BT_MP_REG_RW, STR_BT_MP_RESULT_DELIM, ret,
                    STR_BT_MP_RESULT_DELIM, data);
        } else {
            ret = pBtModule->SetRegMaskBits(pBtModule, type, page, address, msb, lsb, data);
            sprintf(buf_cb, "%s%s0x%02x", STR_BT_MP_REG_RW, STR_BT_MP_RESULT_DELIM, ret);
        }

    }

    SYSLOGI("--%s", STR_BT_MP_REG_RW);
    return ret;
}

void bt_mp_module_init(BASE_INTERFACE_MODULE *pBaseInterfaceModule, BT_MODULE *pBtModule)
{
    SYSLOGI("bt_mp_module_init, pBaseInterfaceModule %p, pBtModule %p", pBaseInterfaceModule, pBtModule);

    BuildTransportInterface(
            pBaseInterfaceModule,
            1,
            115200,
            NULL,//open
            bt_transport_SendHciCmd,
            bt_transport_RecvHciEvt,
            NULL,//close
            bt_transport_WaitMs
            );

    BuildBluetoothModule(
            pBaseInterfaceModule,
            pBtModule
            );

    pBtModule->pBtParam->mPGRawData[0] = 0;
    pBtModule->pBtParam->mChannelNumber = DEFAULT_CH_NUM;
    pBtModule->pBtParam->mPacketType = DEFAULT_PKT_TYPE;
    pBtModule->pBtParam->mPayloadType = DEFAULT_PAYLOAD_TYPE;
    pBtModule->pBtParam->mTxPacketCount = DEFAULT_PKT_COUNT;
    pBtModule->pBtParam->mTxGainValue = DEFAULT_TX_GAIN_VALUE;
    pBtModule->pBtParam->mWhiteningCoeffValue = DEFAULT_WHITE_COEFF_VALUE;
    pBtModule->pBtParam->mTxGainIndex = DEFAULT_TX_GAIN_INDEX;
    pBtModule->pBtParam->mTxDAC = DEFAULT_TX_DAC;
    pBtModule->pBtParam->mPacketHeader = DEFAULT_PKT_HEADER;
    pBtModule->pBtParam->mHoppingFixChannel = DEFAULT_HOPPING_CH_NUM;
    pBtModule->pBtParam->mHitTarget = DEFAULT_HIT_ADDRESS;
}
