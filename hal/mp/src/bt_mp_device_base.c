#define LOG_TAG "bt_mp_device_base"

#include "bt_syslog.h"
#include "bluetoothmp.h"
#include "bt_mp_device_base.h"

#ifndef BOOL
typedef enum _BOOL{false, true}BOOL;
#endif

//  1 slot = 625u
//  con-tx : DH1   625*1   625u     1M/625 = 160;
//           DH3   625*3   1875u    1M/1875= 54;
//           DH5   625*5   3125u     1M/3125= 32
//  pkt-tx :  packet --> packet There will be a slot interval.
//          DH1   625*2   1250u      80
//          DH3   625*4   2500u      40
//          DH5   625*6   3750u      26.666
#define PKTTX_SEC_PACKET_CNT_DH1   160
#define PKTTX_SEC_PACKET_CNT_DH3   54
#define PKTTX_SEC_PACKET_CNT_DH5   32

#define CONTX_SEC_PACKET_CNT_DH1   80
#define CONTX_SEC_PACKET_CNT_DH3   40
#define CONTX_SEC_PACKET_CNT_DH5   27

//#define MultiPktInterval      1   //(n+1) * 100us
#define MULTIPKTINTERVAL        0x01
//#define TxDummyBits           16  //0~31
#define TXDUMMYBITS             0x10
//#define TxDummyPattern        0   //0:all zero, 1: 1010 toggle
#define TXDUMMYPATTEN           0

#define MAX_RX_READ_COUNT_ADR_0X72      60000
#define MAX_RX_READ_ERRORBITS_ADR_0X78  60000

                                                  //dh1   dh3     dh5    2dh1    2dh3     2dh5    3dh1    3dh3    3dh5   poll null
static unsigned char Arrary_Hopping_ptt[10]     ={0x00   ,0x00   ,0x00   ,0x01   ,0x01   ,0x01   ,0x01   ,0x01   ,0x01    ,0x00};
static unsigned char Arrary_Hopping_pkt_type[10]={0x04   ,0x0b   ,0x0f   ,0x04   ,0x0a   ,0x0e   ,0x08   ,0x0b   ,0x0f    ,0x01};
static unsigned int  Arrary_Hopping_pkt_len[10] ={27     ,183    ,339    ,54     ,367    ,679    ,83     ,552    ,1021   ,0   };
static unsigned char Arrary_Interval_slot_number[]={1,3,5,1,3,5,1,3,5,1};
static unsigned int  Arrary_PayloadLength[BT_PKT_TYPE_NUM]={
    216,1464,2712,  //1M
    432,2936,5432,  //2M
    664,4416,8168,  //3M
    296,296         //LE
};

#define OCF_HCI_WRITE_SCAN_ENABLE   0x1A
#define OGF_CONTROLER_AND_BB        0x03

#define OCF_HCI_ENABLE_DUT_MODE     0x03
#define OGF_TESTING                 0x06

#define OPCODE(ocf, ogf)            ((uint16_t) ( (ocf) | ((ogf)<<10) ))

#define BT_SCAN_INTERVAL            0x200
#define BT_SCAN_WINDOW              0x12

uint32_t PktRxCount = 0;
uint32_t PktRxErrBits = 0;

static int
BT_SetGlobalReg2Bytes(
        BT_DEVICE *pBtDevice,
        uint16_t Addr,
        uint16_t Val
        )
{
    uint8_t pPayload[MAX_HCI_COMANND_BUF_SIZ];
    uint8_t pEvent[MAX_HCI_EVENT_BUF_SIZ];
    uint32_t EvtLen;

    //Register Address
    *(pPayload+0) = Addr & 0xff;
    *(pPayload+1) = (Addr & 0xff00) >> 8;

    //Register Value
    *(pPayload+2) = Val & 0xff;
    *(pPayload+3) = (Val & 0xff00) >> 8;

    if (pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xfc64, LEN_4_BYTE, pPayload, 0x0E, pEvent, &EvtLen))
        goto error;

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

static int
BT_GetGlobalReg2Bytes(
        BT_DEVICE *pBtDevice,
        uint16_t Addr,
        uint16_t *pVal
        )
{
    uint8_t pPayload[MAX_HCI_COMANND_BUF_SIZ];
    uint8_t pEvent[MAX_HCI_EVENT_BUF_SIZ];
    uint32_t EvtLen;


    //Register Address
    *(pPayload+0) = Addr & 0xff;
    *(pPayload+1) = (Addr & 0xff00) >> 8;

    if (pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xfc65, LEN_2_BYTE, pPayload, 0x0E, pEvent, &EvtLen))
        goto error;

    *pVal = ( (*(pEvent+EVT_BYTE1)<<8) + *(pEvent+EVT_BYTE0 ) );

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int BT_GetStage(BT_DEVICE *pBtDevice, uint8_t *pStage)
{
    uint32_t data;

    data = 5;
    if (BT_SetSysRegMaskBits(pBtDevice, 0xa3, 7, 0, data))
        goto error;

    data = 0;
    if (BT_GetSysRegMaskBits(pBtDevice, 0xa0,7, 0, &data))
        goto error;

    *pStage = (uint8_t)(data & 0xFF);

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int BTDevice_SetRTL8761Xtal(BT_DEVICE *pBtDevice, uint32_t Value)
{
    uint32_t data_32=0;
    uint32_t data_lsb=0, data_msb=0;
    uint32_t x1=0, x2=0;
    uint32_t ChipID;

    if (pBtDevice->GetChipId(pBtDevice) != BT_FUNCTION_SUCCESS)
        goto error;

    ChipID = pBtDevice->pBTInfo->ChipType;

    if (ChipID != RTK_BT_CHIP_ID_RTL8761A)
        goto error;

    //set 0x2c [12:17]=[18:23]=Xtal Val
    if (BT_GetSysRegMaskBits(pBtDevice, 0x2c, 15, 0, &data_lsb))
        goto error;

    if (BT_GetSysRegMaskBits(pBtDevice, 0x2e, 15, 0, &data_msb))
        goto error;

    data_32 = (data_msb << 16) | (data_lsb & 0xFFFF);

    data_32 &= 0xFF000FFF;

    data_32 |= Value << 12;
    data_32 |= Value << 18;

    x1 = data_32 & 0xFFFF;
    x2 = (data_32 >> 16) & 0xFFFF;

    if (BT_SetSysRegMaskBits(pBtDevice, 0x2c, 15, 0, x1))
        goto error;

    if (BT_SetSysRegMaskBits(pBtDevice, 0x2e, 15, 0, x2))
        goto error;

    SYSLOGI("BTDevice_SetRTL8761Xtal : 0x%08x", Value);

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int BTDevice_GetRTL8761Xtal(BT_DEVICE *pBtDevice, uint32_t *pValue)
{
    uint32_t data=0;
    uint32_t ChipID;

    if (pBtDevice->GetChipId(pBtDevice) != BT_FUNCTION_SUCCESS)
        goto error;

    ChipID = pBtDevice->pBTInfo->ChipType;

    if (ChipID != RTK_BT_CHIP_ID_RTL8761A)
        goto error;

    if (BT_GetSysRegMaskBits(pBtDevice, 0x2e, 7, 2, &data))
        goto error;

    *pValue = data;

    SYSLOGI("BTDevice_GetRTL8761Xtal : 0x%08x", *pValue);

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int BTDevice_ReadThermal(BT_DEVICE *pBtDevice, BT_PARAMETER *pParam, uint8_t *pThermalValue)
{
    uint16_t Value;

    if( pParam->mPacketType==BT_PKT_LE)
    {

        if (BT_SetGlobalReg2Bytes(pBtDevice, 0x1ba, 0x800f))
            goto error;
        if (BT_GetGlobalReg2Bytes(pBtDevice, 0x1ba, &Value))
            goto error;

        *pThermalValue = (uint8_t)((Value >> 9) & 0x1f);
    }
    else
    {
        if(pBtDevice->SetRfRegMaskBits(pBtDevice, 0x00, 15, 0, 0x1000))
            goto error;

        //enable thermal meter
        if(pBtDevice->SetRfRegMaskBits(pBtDevice, 0x04, 15, 0, 0x0020))
            goto error;

        pBtDevice->pBaseInterface->WaitMs(pBtDevice->pBaseInterface, 10);

        if(pBtDevice->GetRfRegMaskBits(pBtDevice, 0x04, 15, 0, &Value))
            goto error;

        *pThermalValue= (uint8_t)(Value & 0x1f);
    }

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int BTDevice_TestModeEnable(BT_DEVICE *pBtDevice)
{
    uint16_t OpCode;
    uint8_t pPayload[MAX_HCI_COMANND_BUF_SIZ];
    uint8_t pEvent[MAX_HCI_EVENT_BUF_SIZ];
    uint32_t EvtLen;
    int status;

    //Set BT HCI Scan Enable
    OpCode = OPCODE( OCF_HCI_WRITE_SCAN_ENABLE, OGF_CONTROLER_AND_BB);
    pPayload[0] = 3;
    if (pBtDevice->SendHciCommandWithEvent(pBtDevice, OpCode, LEN_1_BYTE, pPayload, 0x0E, pEvent, &EvtLen))
        goto error;

    //Set BT HCI Test Mode
    pPayload[0] = 0x02;
    pPayload[1] = 0x00;
    pPayload[2] = 0x02;
    if (pBtDevice->SendHciCommandWithEvent(pBtDevice, 0x0c05, LEN_3_BYTE, pPayload, 0x0E, pEvent, &EvtLen))
        goto error;

    OpCode = OPCODE( OCF_HCI_ENABLE_DUT_MODE, OGF_TESTING);
    pPayload[0] = 0x00;
    if (pBtDevice->SendHciCommandWithEvent(pBtDevice, OpCode, LEN_0_BYTE, pPayload, 0x0E, pEvent, &EvtLen))
        goto error;

    //SetBTHCIScanActivity

    //page sacn Interval:
    *(pPayload+0) = BT_SCAN_INTERVAL & 0xff;
    *(pPayload+1) = (BT_SCAN_INTERVAL & 0xff00) >> 8;

    //page sacn window
    *(pPayload+2) = BT_SCAN_WINDOW & 0xff;
    *(pPayload+3) = (BT_SCAN_WINDOW & 0xff00) >> 8;

    if (pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xc1c, LEN_4_BYTE, pPayload, 0x0E, pEvent, &EvtLen))
        goto error;

    OpCode = (pEvent[4] << 8) + pEvent[3];
    status = pEvent[5];

    //Inquiry sacn Interval:
    *(pPayload+0) = BT_SCAN_INTERVAL & 0xff;
    *(pPayload+1) = (BT_SCAN_INTERVAL & 0xff00) >> 8;

    //Inquiry sacn window
    *(pPayload+2) = BT_SCAN_WINDOW & 0xff;
    *(pPayload+3) = (BT_SCAN_WINDOW & 0xff00) >> 8;

    if (pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xc1e, LEN_4_BYTE, pPayload, 0x0E, pEvent, &EvtLen))
        goto error;

    OpCode = (pEvent[4] << 8) + pEvent[3];
    status = pEvent[5];

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int BTDevice_SetPacketHeader(BT_DEVICE *pBtDevice, uint32_t pktHeader)
{
    int rtn = BT_FUNCTION_SUCCESS;

    rtn = pBtDevice->SetMdRegMaskBits(pBtDevice, 0x30, 15, 0, pktHeader & 0xFFFF);
    if (rtn != BT_FUNCTION_SUCCESS) {
        goto exit;
    }

    rtn = pBtDevice->SetMdRegMaskBits(pBtDevice, 0x32, 1, 0, (pktHeader>>16) & 0x3);
    if (rtn != BT_FUNCTION_SUCCESS) {
        goto exit;
    }

exit:
    return rtn;
}

int BTDevice_SetResetMDCount(BT_DEVICE *pBtDevice)
{
    int rtn = BT_FUNCTION_SUCCESS;

    /* reset report counter */
    if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x2e,11,9,0x00) != BT_FUNCTION_SUCCESS)
    {
        rtn = FUNCTION_HCISEND_ERROR;
        goto exit;
    }

    if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x2e,11,9,0x07) != BT_FUNCTION_SUCCESS)
    {
        rtn = FUNCTION_HCISEND_ERROR;
        goto exit;
    }

exit:
    return rtn;
}

#define CON_TX  0
#define PKT_TX  1

int BTDevice_GetPayloadLenTable(BT_DEVICE *pBtDevice, uint8_t *pTable, int length)
{
    int rtn=BT_FUNCTION_SUCCESS;
    int n=0;
    if (length >BT_PKT_TYPE_NUM)
        length=BT_PKT_TYPE_NUM;
    for (n=0;n<length;n++)
    {
        pTable[n]=Arrary_PayloadLength[n];
    }
    return rtn;
}

int BTDevice_SetPktRxStop(BT_DEVICE *pBtDevice, BT_PARAMETER *pParam, BT_DEVICE_REPORT *pBtReport)
{
    //uint8_t pPayload[MAX_HCI_COMANND_BUF_SIZ];
    //uint8_t pEvent[MAX_HCI_EVENT_BUF_SIZ];
    uint32_t EventLen = 0;
    uint16_t OpCode=0x0000;
    int pktType=pParam->mPacketType;

    SYSLOGI("+BTDevice_SetPktRxStop");
#if 0
    if (pParam->mTestMode == BT_DUT_MODE)
    {
        if (pktType == BT_PKT_LE)
        {
            OpCode=0x201F;
        }
        else
        {
            goto exit;
        }
        if (OpCode >0x0000)
        {
            if (pBtDevice->SendHciCommandWithEvent(pBtDevice,OpCode,0,pPayload,0x0E,pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
            {
                goto exit;
            }
            if (pEvent[5] != 0x00)
            {
                goto exit;
            }

        }
    }
    else
#endif
    {
        if (pBtDevice->SetMutiRxEnable(pBtDevice, 0x00) != BT_FUNCTION_SUCCESS)
        {
            goto exit;
        }

        if (pBtDevice->SetMdRegMaskBits(pBtDevice, 0x2e, 15, 0, 0x0070) != BT_FUNCTION_SUCCESS)
        {
            goto exit;
        }

        //Back to Shut Down mode
        if (pBtDevice->SetRfRegMaskBits(pBtDevice, 0x00, 15, 0, 0x0000) != BT_FUNCTION_SUCCESS)
        {
            goto exit;
        }
    }

    SYSLOGI("-BTDevice_SetPktRxStop");
    return BT_FUNCTION_SUCCESS;

exit:
    SYSLOGI("-BTDevice_SetPktRxStop: ERROR");
    return FUNCTION_ERROR;
}

int BTDevice_SetPktRxBegin(BT_DEVICE *pBtDevice, BT_PARAMETER *pParam, BT_DEVICE_REPORT *pBtReport)
{
    int rtn=BT_FUNCTION_SUCCESS;
    unsigned long btClockTime=0;
    //uint8_t pPayload[MAX_HCI_COMANND_BUF_SIZ];
    //uint8_t pEvent[MAX_HCI_EVENT_BUF_SIZ];
    uint32_t EventLen = 0;
    uint16_t OpCode=0x0000;
    int pktType = pParam->mPacketType;
    uint8_t pPayload_Len=0;
    uint16_t tmp = 0;

    SYSLOGI("+BTDevice_SetPktRxBegin: mChannelNumber 0x%x, mPacketType 0x%x, mTxGainIndex 0x%x, "
          "mTxGainValue 0x%x, mTxPacketCount 0x%x, mPayloadType 0x%x, mPacketHeader 0x%x, "
          "mWhiteningCoeffValue 0x%x, mTxDAC 0x%x, mHitTarget 0x%012llx",
          pParam->mChannelNumber, pParam->mPacketType, pParam->mTxGainIndex, pParam->mTxGainValue,
          pParam->mTxPacketCount, pParam->mPayloadType, pParam->mPacketHeader,
          pParam->mWhiteningCoeffValue, pParam->mTxDAC, pParam->mHitTarget);

    if (pParam->mPacketType == BT_PKT_LE)
    {
        if (pParam->mChannelNumber > 39)
        {
            rtn = FUNCTION_PARAMETER_INVALID_CHANNEL;
            return rtn;
        }
    }

    if (pBtReport != NULL)
    {
        // RX report Clear
        pBtReport->TotalRXBits=0;
        pBtReport->TotalRxCounts=0;
        pBtReport->TotalRxErrorBits=0;
        pBtReport->RxRssi=-90;
        pBtReport->RXRecvPktCnts=0;

        PktRxCount = 0;
        PktRxErrBits = 0;
    }
    //disable modem fix tx
    if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x3c,12,12,0x0) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    // set payload type
    if (pBtDevice->SetPayloadType(pBtDevice,pParam->mPayloadType) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    // set rate and payload length
    if (pBtDevice->SetPacketType(pBtDevice,pParam->mPacketType) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    // set packet header
    if (pBtDevice->SetPacketHeader(pBtDevice,pParam->mPacketHeader)!= BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    //set target bd address
    if (pBtDevice->SetHitTarget(pBtDevice,pParam->mHitTarget) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    // set WhiteningCoeffValue
    if (pBtDevice->SetWhiteningCoeff(pBtDevice,pParam->mWhiteningCoeffValue) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    //set channel
    if (pBtDevice->SetRxChannel(pBtDevice,pParam->mChannelNumber) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    //multi-packet Rx
    if (pBtDevice->SetMutiRxEnable(pBtDevice, 1) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    //set test mode
    if (pBtDevice->SetTestMode(pBtDevice, BT_PSEUDO_MODE) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
#if 0
    if (pParam->mTestMode == BT_DUT_MODE)
    {
        if (pktType == BT_PKT_LE)
        {
            pPayload[0]=pParam->mChannelNumber;
            OpCode=0x201d;

        }
        if (OpCode > 0)
        {
            if (pBtDevice->SendHciCommandWithEvent(pBtDevice,OpCode,pPayload_Len,pPayload,0x0E,pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
            {
                rtn=FUNCTION_HCISEND_ERROR;
            }
            if (pEvent[5] != 0x00)
            {
                rtn=FUNCTION_HCISEND_STAUTS_ERROR;
            }
        }
        else
        {
            rtn = FUNCTION_NO_SUPPORT;

        }
        return rtn;
    }
#endif
    if (pBtDevice->SetRestMDCount(pBtDevice) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    if (pBtDevice->SetMdRegMaskBits(pBtDevice, 0x2e, 1, 1, 0x00) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    if (pBtDevice->SetMdRegMaskBits(pBtDevice, 0x2e, 1, 1, 0x01) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    SYSLOGI("-BTDevice_SetPktRxBegin");
    return BT_FUNCTION_SUCCESS;

exit:
    SYSLOGI("-BTDevice_SetPktRxBegin: ERROR");
    return FUNCTION_ERROR;
}

int BTDevice_SetPktRxUpdate(BT_DEVICE *pBtDevice,BT_PARAMETER *pParam,BT_DEVICE_REPORT *pBtReport)
{
    uint16_t rxCount=0;
    uint32_t rxBits=0;
    uint16_t rxErrbits=0;
    uint16_t rxPin=0;
    uint16_t data;
    int32_t value;

    int pktType = pParam->mPacketType;

    pBtReport->Cfo = 999;

    if (pBtReport == NULL)
    {
        goto exit;
    }

    if (pktType > BT_PKT_LE)
    {
        goto exit;
    }

    //Cfo
    if (bt_default_GetMDRegMaskBits(pBtDevice, 0x6c, 8, 0, &data))
        goto exit;

    value = BinToSignedInt(data, LEN_9_BYTE);
    pBtReport->Cfo= ((float)value /(float)4096)*10000;

    //rx Pin
    if (pBtDevice->GetMdRegMaskBits(pBtDevice,0x70,15,10,&rxPin) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    pBtReport->RxRssi = (int)(rxPin);
    pBtReport->RxRssi = ((pBtReport->RxRssi >> 2) *2) -90;

    //if (pBtReport->RxRssi > -89)
    {
        //rx Count
        if (pBtDevice->GetMdRegMaskBits(pBtDevice, 0x72, 15, 0, &rxCount) != BT_FUNCTION_SUCCESS)
        {
            goto exit;
        }
        //rx error Bit
        if (pBtDevice->GetMdRegMaskBits(pBtDevice, 0x78, 15, 0, &rxErrbits) != BT_FUNCTION_SUCCESS)
        {
            goto exit;
        }

        pBtReport->RXRecvPktCnts = PktRxCount + rxCount - pBtReport->TotalRxCounts;
        pBtReport->TotalRxCounts = PktRxCount + rxCount;

        rxBits = pBtReport->TotalRxCounts * Arrary_PayloadLength[pktType];
        pBtReport->TotalRXBits = rxBits;
        pBtReport->TotalRxErrorBits = PktRxErrBits+rxErrbits;
        if (pBtReport->TotalRXBits > 0)
        {
            pBtReport->ber = (float)pBtReport->TotalRxErrorBits / (float)pBtReport->TotalRXBits;
        }
        else
        {
            pBtReport->ber = 0;
        }

        if ((rxCount > MAX_RX_READ_COUNT_ADR_0X72) || (rxErrbits > MAX_RX_READ_ERRORBITS_ADR_0X78))
        {

            SYSLOGI("BTDevice_SetPktRxUpdate: Reset rxCount & rxErrbits !!");

            if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x2e,10,9,0x00) != BT_FUNCTION_SUCCESS)
            {
                goto exit;
            }
            if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x2e,10,9,0x03) != BT_FUNCTION_SUCCESS)
            {
                goto exit;
            }
            PktRxCount = pBtReport->TotalRxCounts;
            PktRxErrBits = pBtReport->TotalRxErrorBits;
        }
    }

    SYSLOGI("-BTDevice_Set Pkt Rx Update : RxRssi= %d, rxCount = %d, ber=%f", pBtReport->RxRssi, rxCount, pBtReport->ber);
    return BT_FUNCTION_SUCCESS;

exit:
    SYSLOGI("-BTDevice_Set Pkt Rx Update: ERROR");
    return FUNCTION_ERROR;
}

int BTDevice_SetPktTxStop(BT_DEVICE *pBtDevice,BT_PARAMETER *pParam,BT_DEVICE_REPORT *pBtReport)
{
    //uint8_t pPayload[MAX_HCI_COMANND_BUF_SIZ];
    //uint8_t pEvent[MAX_HCI_EVENT_BUF_SIZ];
    uint16_t OpCode=0x0000;
    uint32_t EventLen = 0;
    int pktType = pParam->mPacketType;

    SYSLOGI("+BTDevice_SetPktTxStop");

    //disable multi-packet Tx
    if (pBtDevice->SetMutiRxEnable(pBtDevice, 0x00) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    if (pBtDevice->SetMdRegMaskBits(pBtDevice, 0x2e, 15, 0, 0x0070) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    //Back to Shut Down mode
    if (pBtDevice->SetRfRegMaskBits(pBtDevice, 0x00, 15, 0, 0x0000) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    SYSLOGI("-BTDevice_SetPktTxStop");
    return BT_FUNCTION_SUCCESS;

exit:
    SYSLOGI("-BTDevice_SetPktTxStop: ERROR");
    return FUNCTION_ERROR;
}

int BTDevice_SetPktTxBegin_PSEUDOMODE(BT_DEVICE *pBtDevice,BT_PARAMETER *pParam)
{
    unsigned long NewModemReg4Value =0;

    /* disable continous tx mode */
    if ( pBtDevice->SetMdRegMaskBits(pBtDevice,0x2e,12,12,0x00) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    /* set tx pkt count and interval */
    if ((pParam->mTxPacketCount >= 0xFFF) || (pParam->mTxPacketCount == 0))
    {
        NewModemReg4Value = 0xFFF;
    }
    else
    {
        NewModemReg4Value=pParam->mTxPacketCount;
    }
    NewModemReg4Value =( NewModemReg4Value <<4 ) | (MULTIPKTINTERVAL&0x000F);
    if ( pBtDevice->SetMdRegMaskBits(pBtDevice,0x04,15,0,NewModemReg4Value) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }


    //Dummy Tx bits 
    //[a] switch page-2
    if ( pBtDevice->SetMdRegMaskBits(pBtDevice,0x00,15,0,0x02) != BT_FUNCTION_SUCCESS) 
    {
        goto exit;
    }

    if ( pBtDevice->SetMdRegMaskBits(pBtDevice,0x3e,6,0, ((TXDUMMYPATTEN & 0x03)<<5)|(TXDUMMYBITS & 0x1F)) != BT_FUNCTION_SUCCESS) 
    {
        goto exit;
    }

    //[b] switch page-0
    if ( pBtDevice->SetMdRegMaskBits(pBtDevice,0x00,15,0,0x00) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    /* generate neg-edge pulse to trigger */
    if ( pBtDevice->SetMdRegMaskBits(pBtDevice,0x2e,0,0,0x01) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    if ( pBtDevice->SetMdRegMaskBits(pBtDevice,0x2e,0,0,0x00) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    return BT_FUNCTION_SUCCESS;

exit:
    return FUNCTION_ERROR;
}

int BTDevice_SetPktTxBegin(BT_DEVICE *pBtDevice, BT_PARAMETER *pParam, BT_DEVICE_REPORT *pBtReport)
{
    int rtn = BT_FUNCTION_SUCCESS;
    unsigned long btClockTime = 0;
    uint16_t tmp = 0;
    BT_TRX_TIME *pTxTime = &pBtDevice->TRxTime[TX_TIME_RUNING];

    SYSLOGI("BTDevice_SetPktTxBegin: mChannelNumber 0x%02x, mPacketType 0x%02x, "
          "mPayloadType 0x%02x, mTxPacketCount 0x%04x, mTxGainValue 0x%02x, "
          "mWhiteningCoeffValue 0x%02x, mTxGainIndex 0x%02x, "
          "mTxDAC 0x%02x, mPacketHeader 0x%04x",
          pParam->mChannelNumber, pParam->mPacketType,
          pParam->mPayloadType, pParam->mTxPacketCount, pParam->mTxGainValue,
          pParam->mWhiteningCoeffValue, pParam->mTxGainIndex,
          pParam->mTxDAC, pParam->mPacketHeader);

    if (pParam->mPacketType == BT_PKT_LE) {
        if (pParam->mChannelNumber > 39) {
            rtn = FUNCTION_PARAMETER_INVALID_CHANNEL;
            return rtn;
        }
    }

    SYSLOGI("+BTDevice_SetPktTxBegin");

    pTxTime->beginTimeClockCnt=0;
    pTxTime->UseTimeClockCnt=0;
    pTxTime->endTimeClockCnt=0;

    pBtDevice->TxTriggerPktCnt=0;

    if (pBtReport != NULL) {
        pBtReport->TotalTXBits=0;
        pBtReport->TotalTxCounts=0;
    }

    //disable modem fix tx
    // RTK_UPDATE_MODEM_REG(TRANS_MODEM_REG(0x3C), BIT12, 0);
    if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x3c,12,12,0x0) != BT_FUNCTION_SUCCESS) {
        goto exit;
    }

    // set payload type
    if (pBtDevice->SetPayloadType(pBtDevice,pParam->mPayloadType)  != BT_FUNCTION_SUCCESS) {
        goto exit;
    }

    // set rate and payload length
    if (pBtDevice->SetPacketType(pBtDevice,pParam->mPacketType) != BT_FUNCTION_SUCCESS) {
        goto exit;
    }

    // set packet header
    if (pBtDevice->SetPacketHeader(pBtDevice,pParam->mPacketHeader)!= BT_FUNCTION_SUCCESS) {
        goto exit;
    }

    //set target bd address
    if (pBtDevice->SetHitTarget(pBtDevice,pParam->mHitTarget) != BT_FUNCTION_SUCCESS) {
        goto exit;
    }

    // set WhiteningCoeffValue
    if (pBtDevice->SetWhiteningCoeff(pBtDevice,pParam->mWhiteningCoeffValue) != BT_FUNCTION_SUCCESS) {
        goto exit;
    }

    //set channel
    if (pBtDevice->SetTxChannel(pBtDevice,pParam->mChannelNumber) != BT_FUNCTION_SUCCESS) {
        goto exit;
    }

    //set outpower
    if ((pParam->mTxGainIndex > 0) && (pParam->mTxGainIndex <= 7))
        rtn = pBtDevice->SetPowerGainIndex(pBtDevice,pParam->mTxGainIndex);
    else
        rtn = pBtDevice->SetPowerGain(pBtDevice,pParam->mTxGainValue);

    if (rtn != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    //set test mode
    if (pBtDevice->SetTestMode(pBtDevice, BT_PSEUDO_MODE) != BT_FUNCTION_SUCCESS) {
        goto exit;
    }

    if (pBtDevice->SetMutiRxEnable(pBtDevice, 1) != BT_FUNCTION_SUCCESS) {
        goto exit;
    }
#if 0
    //begin con-tx setting
    if (pParam->mTestMode == BT_DUT_MODE)
    {
        if (pParam->mPacketType <= BT_PKT_LE)//1DH1~33DH5
        {
            if (BTDevice_SetPktTxBegin_DUTMODE(pBtDevice,pParam) != BT_FUNCTION_SUCCESS) {
                goto exit;
            }
        }
        else
        {
            //con-Tx don't support DUT MODE
            goto exit;
        }
    }
    else
#endif
    {
        if (pParam->mPacketType <= BT_PKT_LE)
        {
            if (BTDevice_SetPktTxBegin_PSEUDOMODE(pBtDevice,pParam) != BT_FUNCTION_SUCCESS) {
                goto exit;
            }
        }
        else
        {
            goto exit;
        }
    }
    //get begin clock
    if (BTDevice_GetBTClockTime(pBtDevice,&btClockTime) == BT_FUNCTION_SUCCESS)
    {
        pTxTime->beginTimeClockCnt = btClockTime;
    }

    SYSLOGI("-BTDevice_SetPktTxBegin");
    return BT_FUNCTION_SUCCESS;

exit:
    SYSLOGI("-BTDevice_SetPktTxBegin: ERROR");
    return FUNCTION_ERROR;
}

int BTDevice_SetPktTxUpdate(BT_DEVICE *pBtDevice,BT_PARAMETER *pParam,BT_DEVICE_REPORT *pBtReport)
{
    unsigned long NewModemReg4Value=0;
    uint16_t tmp=0;
    uint32_t TXUpdateBits, TXPktUpdateCnts;

    SYSLOGI("BTDevice_SetPktTxUpdate");

    //report
    BTDevice_CalculatedTxBits(pBtDevice, pParam, pBtReport, PKT_TX, &TXUpdateBits, &TXPktUpdateCnts);
    pBtReport->TotalTXBits += TXUpdateBits;
    pBtDevice->TxTriggerPktCnt += TXPktUpdateCnts;
    pBtReport->TotalTxCounts += TXPktUpdateCnts;

    if ((pBtReport->TotalTxCounts >= pParam->mTxPacketCount) && (pParam->mTxPacketCount != 0))
    {
        SYSLOGI("BTDevice_Set Pkt Tx Update: Pkt Tx Finish......");
        return FUNCTION_TX_FINISH;
    }

    if (pBtDevice->TxTriggerPktCnt >= 0xFFF)
    {
        SYSLOGI(" BTDevice_Set Pkt Tx Update: Pkt Tx Re-Trigger.....");
        pBtDevice->TxTriggerPktCnt=0;

        if (pParam->mTxPacketCount == 0)
        {
            NewModemReg4Value = 0xFFF;
        }
        else
        {

            tmp=pParam->mTxPacketCount - pBtReport->TotalTxCounts;

            if ( tmp >= 0xFFF)
            {
                NewModemReg4Value = 0xFFF;
            }
            else
            {
                NewModemReg4Value=tmp;
            }
        }
        NewModemReg4Value =( NewModemReg4Value <<4 ) | (MULTIPKTINTERVAL&0x000F);
        if ( pBtDevice->SetMdRegMaskBits(pBtDevice,0x04,15,0,NewModemReg4Value) != BT_FUNCTION_SUCCESS)
        {
            goto exit;
        }
        if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x2e,0,0,0x01) != BT_FUNCTION_SUCCESS)
        {
            goto exit;
        }
        if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x2e,0,0,0x00) != BT_FUNCTION_SUCCESS)
        {
            goto exit;
        }


    }

    if ((pBtReport == NULL) || (pParam == NULL))
        goto exit;

    return BT_FUNCTION_SUCCESS;

exit:
    SYSLOGI("-BTDevice_Set Pkt Tx Update: ERROR");
    return FUNCTION_ERROR;
}

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
        )
{
    int rtn = BT_FUNCTION_SUCCESS;
    uint8_t pEvent[MAX_HCI_EVENT_BUF_SIZ];
    uint32_t EventLen = 0;

    uint8_t pPayLoad[LEN_11_BYTE];
    uint8_t *ptt = Arrary_Hopping_ptt;
    uint8_t *pkt_type = Arrary_Hopping_pkt_type;
    unsigned int *pkt_len = Arrary_Hopping_pkt_len;
    int Index = 0;

    SYSLOGI("BTDevice_SetHoppingMode: ChannelNumber %d, PktType %d, PayloadType %d, TxGainValue %d, "
          "WhiteningCoeffValue %d, TxGainIndex %d, TxDAC %d, HoppingFixChannel %d",
          ChannelNumber, PktType, PayloadType, TxGainValue,
          WhiteningCoeffValue, TxGainIndex, TxDAC, HoppingFixChannel);

    switch (PktType) {
    case BT_PKT_1DH1:
    case BT_PKT_1DH3:
    case BT_PKT_1DH5:
    case BT_PKT_2DH1:
    case BT_PKT_2DH3:
    case BT_PKT_2DH5:
    case BT_PKT_3DH1:
    case BT_PKT_3DH3:
    case BT_PKT_3DH5:
        Index = PktType;
        break;

    case BT_PKT_TYPE_NULL:
        Index = 9;
        break;

    case BT_PKT_LE:
        Index = -1;
        break;

    default:
        rtn = FUNCTION_PARAMETER_ERROR;
        goto exit;
    }

    if (Index >= 0)
    {
        pPayLoad[0] = ptt[Index];
        pPayLoad[1] = pkt_type[Index];
        pPayLoad[2] = pkt_len[Index] & 0xFF;
        pPayLoad[3] = (pkt_len[Index]>>8) & 0xFF;
        pPayLoad[4] = HoppingFixChannel;

        if (HoppingFixChannel == 0)
        {
            pPayLoad[5] = 0;
        }
        else
        {
            pPayLoad[5] = ChannelNumber;
        }

        pPayLoad[6] = WhiteningCoeffValue;
        pPayLoad[7] = PayloadType;
        pPayLoad[8] = TxGainValue;
        pPayLoad[9] = TxGainIndex;
        pPayLoad[10] = TxDAC;

        if (pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xfc75, LEN_11_BYTE, pPayLoad, 0x0E, pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
        {
            rtn = FUNCTION_HCISEND_ERROR;
            goto exit;
        }

        if (pEvent[5] != 0x00)
        {
            rtn=FUNCTION_HCISEND_STAUTS_ERROR;
            goto exit;
        }

        pPayLoad[0]=0;
    }
    else
    {
        pPayLoad[0]=1;
    }

    if (pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xFD45, 1, pPayLoad, 0x0E, pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
    {
        rtn = FUNCTION_HCISEND_ERROR;
        goto exit;
    }

    if (pEvent[5] != 0x00)
        rtn = FUNCTION_HCISEND_STAUTS_ERROR;

exit:
    return rtn;
}

static int BTDevice_SetContinueTxBegin_PSEUDOMODE(BT_DEVICE *pBtDevice,BT_PARAMETER *pParam)
{
    int PacketType = pParam->mPacketType;
    //LE
    if (PacketType == BT_PKT_LE)
    {
        if (pParam->mChannelNumber > 39)
        {
            goto error;
        }
        if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x3c,5,5,0x01) != BT_FUNCTION_SUCCESS)
        {
            goto error;
        }
    }
    //Continue Tx mode
    if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x2e,12,12,0x01) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    //Generate Negedge Pulse
    if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x2e,0,0,0x01) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x2e,0,0,0x00) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int BTDevice_SetContinueTxBegin(BT_DEVICE *pBtDevice,BT_PARAMETER *pParam,BT_DEVICE_REPORT *pBtReport)
{
    int rtn=BT_FUNCTION_SUCCESS;
    unsigned long btClockTime=0;
    BT_TRX_TIME *pTxTime = &pBtDevice->TRxTime[TX_TIME_RUNING];

    pTxTime->beginTimeClockCnt=0;
    pTxTime->UseTimeClockCnt=0;
    pTxTime->endTimeClockCnt=0;

    if (pBtReport != NULL)
    {
        pBtReport->TotalTXBits=0;
        pBtReport->TotalTxCounts=0;
    }
    SYSLOGI("+BTDevice_SetContinueTxBegin");

    //disable modem fix tx
    // RTK_UPDATE_MODEM_REG(TRANS_MODEM_REG(0x3C), BIT12, 0);
    if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x3c,12,12,0x0) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    // set payload type
    if (pBtDevice->SetPayloadType(pBtDevice,pParam->mPayloadType) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    // set rate and payload length
    if (pBtDevice->SetPacketType(pBtDevice,pParam->mPacketType) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    // set packet header
    if (pBtDevice->SetPacketHeader(pBtDevice,pParam->mPacketHeader) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    //set target bd address
    if (pBtDevice->SetHitTarget(pBtDevice,pParam->mHitTarget) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    // set WhiteningCoeffValue
    if (pBtDevice->SetWhiteningCoeff(pBtDevice, pParam->mWhiteningCoeffValue) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    //set test mode
    if (pBtDevice->SetTestMode(pBtDevice, BT_PSEUDO_MODE) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    //set channel
    if (pBtDevice->SetTxChannel(pBtDevice,pParam->mChannelNumber) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    //set outpower
    if ((pParam->mTxGainIndex > 0) && (pParam->mTxGainIndex <= 7))
        rtn = pBtDevice->SetPowerGainIndex(pBtDevice,pParam->mTxGainIndex);
    else
        rtn = pBtDevice->SetPowerGain(pBtDevice,pParam->mTxGainValue);
    if (rtn != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }

    if (pBtDevice->SetMutiRxEnable(pBtDevice,0) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
#if 0
    //begin con-tx setting
    if (pParam->mTestMode == BT_DUT_MODE)
    {
        if (pParam->mPacketType <= BT_PKT_LE)//1DH1~33DH5
        {
            if (BTDevice_SetContinueTxBegin_DUTMODE(pBtDevice,pParam) != BT_FUNCTION_SUCCESS)
            {
                goto error;
            }
        }
        else
        {
            //con-Tx don't support DUT MODE
            goto error;
        }
    }
    else
#endif
    {
        if (pParam->mPacketType <= BT_PKT_LE)
        {
            if (BTDevice_SetContinueTxBegin_PSEUDOMODE(pBtDevice,pParam)!=BT_FUNCTION_SUCCESS)
            {
                goto error;
            }
        }
        else
        {
            goto error;
        }
    }
    //get begin clock
    if (BTDevice_GetBTClockTime(pBtDevice,&btClockTime) == BT_FUNCTION_SUCCESS)
    {
        pTxTime->beginTimeClockCnt=btClockTime;
    }

    SYSLOGI("-BTDevice_SetContinueTxBegin");
    return BT_FUNCTION_SUCCESS;

error:
    SYSLOGI("-BTDevice_SetContinueTxBegin: ERROR");
    return FUNCTION_ERROR;
}

static int BTDevice_SetContinueTxStop_PSEUDOMODE(BT_DEVICE *pBtDevice,BT_PARAMETER *pParam)
{
    if (pBtDevice->SetMdRegMaskBits(pBtDevice, 0x2e, 15, 0, 0x0070) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    //Back to Standby Mode from Shut Down mode
    if (pBtDevice->SetRfRegMaskBits(pBtDevice, 0x00, 15, 0, 0x0000) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    return BT_FUNCTION_SUCCESS;

exit:
    return FUNCTION_ERROR;
}

int BTDevice_CalculatedTxBits(BT_DEVICE *pBtDevice, BT_PARAMETER *pParam, BT_DEVICE_REPORT *pBtReport, int pktTx_conTx, uint32_t *txbits, uint32_t *txpkt_cnt)
{
    int rtn=BT_FUNCTION_SUCCESS;
    uint32_t pkt_cnt=0;
    uint32_t pkt_Interval_slot_number=0;
    uint32_t pkt_Len=0;
    BT_PKT_TYPE PacketType=0;
    uint32_t use_uSec=0;
    unsigned long btClockTime=0;

    BT_TRX_TIME *pTxTime = &pBtDevice->TRxTime[TX_TIME_RUNING];

    if (BTDevice_GetBTClockTime(pBtDevice, &pTxTime->endTimeClockCnt) == BT_FUNCTION_SUCCESS)
    {
        if (pTxTime->endTimeClockCnt >= pTxTime->beginTimeClockCnt)
        {
            pTxTime->UseTimeClockCnt = pTxTime->endTimeClockCnt - pTxTime->beginTimeClockCnt;
        }
        else
        {
            pTxTime->UseTimeClockCnt=0x1FFFFFFF - pTxTime->beginTimeClockCnt + pTxTime->endTimeClockCnt;
        }
    }

    pTxTime->beginTimeClockCnt = pTxTime->endTimeClockCnt;

    if ((pBtReport == NULL) || (pParam == NULL))
        goto exit;

    PacketType = pParam->mPacketType;

    use_uSec = pTxTime->UseTimeClockCnt * 3125;  // 1 clock =312.5u

    if (pktTx_conTx == PKT_TX)
        pkt_Interval_slot_number=1 + Arrary_Interval_slot_number[PacketType];   //pkt-tx 
    else  if(pktTx_conTx == CON_TX)
        pkt_Interval_slot_number= Arrary_Interval_slot_number[PacketType];	//con-tx
    else
    {
        rtn = FUNCTION_PARAMETER_ERROR;
        goto exit;
    }
    pkt_cnt = use_uSec/ (pkt_Interval_slot_number * 6250);
    *txpkt_cnt= pkt_cnt;
    pkt_Len= Arrary_Hopping_pkt_len[PacketType];
    *txbits=pkt_cnt * pkt_Len *8;

    SYSLOGI("BTDevice_CalculatedTxBits: time= %d, txpkt_cnt = %d, txbits=%d", use_uSec, *txpkt_cnt, *txbits);

exit:
    return rtn;
}

int BTDevice_SetContinueTxStop(BT_DEVICE *pBtDevice,BT_PARAMETER *pParam,BT_DEVICE_REPORT *pBtReport)
{
    SYSLOGI("+BTDevice_SetContinueTxStop");
#if 0
    if (pParam->mTestMode == BT_DUT_MODE)
    {
        if (pParam->mPacketType <= BT_PKT_LE)//1DH1~3DH5,LE
        {
            if (BTDevice_SetContinueTxStop_DUTMODE(pBtDevice,pParam) != BT_FUNCTION_SUCCESS)
            {
                goto exit;
            }
        }
        else
        {
            goto exit;
        }
    }
    else
#endif
    {
        if (pParam->mPacketType <= BT_PKT_LE)//1DH1~3DH5,LE
        {
            if (BTDevice_SetContinueTxStop_PSEUDOMODE(pBtDevice,pParam) != BT_FUNCTION_SUCCESS)
            {
                goto exit;
            }
        }
        else
        {
            goto exit;
        }
    }

    SYSLOGI("-BTDevice_SetContinueTxStop");
    return BT_FUNCTION_SUCCESS;

exit:
    SYSLOGI("-BTDevice_SetContinueTxStop: ERROR");
    return FUNCTION_ERROR;
}

int BTDevice_SetContinueTxUpdate(BT_DEVICE *pBtDevice, BT_PARAMETER *pParam, BT_DEVICE_REPORT *pBtReport)
{
    uint32_t TXUpdateBits, TXPktUpdateCnts;

    if ((pBtReport == NULL) || (pParam == NULL))
        goto exit;

    //report
    BTDevice_CalculatedTxBits(pBtDevice, pParam, pBtReport, PKT_TX, &TXUpdateBits, &TXPktUpdateCnts);

    pBtReport->TotalTXBits += TXUpdateBits;

    pBtReport->TotalTxCounts += TXPktUpdateCnts;

    SYSLOGI("BTDevice_SetContinueTxUpdate");

    return BT_FUNCTION_SUCCESS;

exit:
    SYSLOGI("BTDevice_SetContinueTxUpdate: ERROR");
    return FUNCTION_ERROR;
}

int BTDevice_LeTxTestCmd(BT_DEVICE *pBtDevice, BT_PARAMETER *pParam, BT_DEVICE_REPORT *pBtReport)
{
    uint8_t pPayload[MAX_HCI_COMANND_BUF_SIZ];
    uint8_t pEvent[MAX_HCI_EVENT_BUF_SIZ];
    uint32_t EventLen = 0;

    pPayload[0] = pParam->mParamData[0]; // channel: 0~39
    pPayload[1] = pParam->mParamData[1]; // Length_Of_Test_Data: 0x00~0x25
    pPayload[2] = pParam->mParamData[2]; // Packet_Payload

    if (pPayload[0] > 39 || pPayload[1] > 0x25 || pPayload[2] > 7)
    {
        goto exit;
    }

    SYSLOGI("BTDevice_LeTxTestCmd: Channel %d, Data length %d, Pkt Payload %d",
            pPayload[0], pPayload[1], pPayload[2]);

    if (pBtDevice->SendHciCommandWithEvent(pBtDevice, 0x201E, LEN_3_BYTE, pPayload, 0x0E, pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    if (pEvent[5] != 0x00)
    {
        goto exit;
    }

    return BT_FUNCTION_SUCCESS;

exit:
    SYSLOGI("BTDevice_LeTxTestCmd: ERROR");
    return FUNCTION_ERROR;
}

int BTDevice_LeRxTestCmd(BT_DEVICE *pBtDevice, BT_PARAMETER *pParam, BT_DEVICE_REPORT *pBtReport)
{
    uint8_t pPayload[MAX_HCI_COMANND_BUF_SIZ];
    uint8_t pEvent[MAX_HCI_EVENT_BUF_SIZ];
    uint32_t EventLen = 0;

    pPayload[0] = pParam->mParamData[0]; // channel: 0~39

    if (pPayload[0] > 39)
    {
        SYSLOGI("BTDevice_LeRxTestCmd: Channel error %d", pPayload[0]);
        goto exit;
    }

    if (pBtDevice->SendHciCommandWithEvent(pBtDevice, 0x201D, LEN_1_BYTE, pPayload, 0x0E, pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    if (pEvent[5] != 0x00)
    {
        goto exit;
    }

    SYSLOGI("BTDevice_LeRxTestCmd");

    return BT_FUNCTION_SUCCESS;

exit:
    SYSLOGI("BTDevice_LeRxTestCmd: ERROR");
    return FUNCTION_ERROR;
}

int BTDevice_LeTestEndCmd(BT_DEVICE *pBtDevice, BT_PARAMETER *pParam, BT_DEVICE_REPORT *pBtReport)
{
    uint8_t pPayload[MAX_HCI_COMANND_BUF_SIZ];
    uint8_t pEvent[MAX_HCI_EVENT_BUF_SIZ];
    uint32_t EventLen = 0;

    if (pBtDevice->SendHciCommandWithEvent(pBtDevice, 0x201F, LEN_0_BYTE, pPayload, 0x0E, pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    if (pEvent[5] != 0x00)
    {
        goto exit;
    }

    // Rx: Number_Of_Packets
    pBtReport->TotalRxCounts = (*(pEvent + EVT_BYTE1) << 8) + *(pEvent + EVT_BYTE0);

    SYSLOGI("BTDevice_LeTestEndCmd");

    return BT_FUNCTION_SUCCESS;

exit:
    SYSLOGI("BTDevice_LeTestEndCmd: ERROR");
    return FUNCTION_ERROR;
}

int BTDevice_GetBTClockTime(
        BT_DEVICE *pBtDevice,
        unsigned long *btClockTime
        )
{
    int rtn=BT_FUNCTION_SUCCESS;
    int i=0;
    unsigned long Time=0;
    unsigned char pPayload[MAX_HCI_COMANND_BUF_SIZ];
    unsigned char pEvent[MAX_HCI_EVENT_BUF_SIZ];
    uint32_t EventLen = 0;
    pPayload[0]=0x00;
    pPayload[1]=0x00;
    pPayload[2]=0x00;

    if (pBtDevice->SendHciCommandWithEvent(pBtDevice,0x1407,3,pPayload,0x0E,pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
    {
        rtn=FUNCTION_HCISEND_ERROR;
        goto exit;
    }
    if (pEvent[5] != 0x00)
    {
        rtn=FUNCTION_HCISEND_STAUTS_ERROR;
    }
    *btClockTime=0;
    for (i=0;i<4;i++)
    {
        Time= pEvent[8+i];

        *btClockTime += (Time << (i*8));
    }
    *btClockTime = *btClockTime & 0x1fffffff;

exit:
    return rtn;
}

int BTDevice_SetMutiRxEnable(BT_DEVICE *pBtDevice,int IsMultiPktRx)
{
    int rtn=BT_FUNCTION_SUCCESS;
    if (IsMultiPktRx)
    {
        rtn = pBtDevice->SetMdRegMaskBits(pBtDevice,0x32,9,9,1);
    }
    else
    {
        rtn = pBtDevice->SetMdRegMaskBits(pBtDevice,0x32,9,9,0);
    }

    return rtn;
}

unsigned char  BT_DEFAULT_TX_GAIN_TABLE[][MAX_TXGAIN_TABLE_SIZE] =
{
    {0x2d,0x30,0x6d,0x70,0xb0,0xcd,0xd0},   //RTL8723A
    {0x2b,0x2e,0x6b,0x6e,0x8b,0x8e,0xce},   //RTL8723B
    {0x28,0x2b,0x48,0x4b,0xc8,0xcc,0xe9},   //RTL8821A
    {0x0d,0x49,0x4d,0x69,0x89,0x8d,0xa9}    //RTL8761A
};

int BTDevice_SetTxGainTable(BT_DEVICE *pBtDevice, unsigned char *pTable)
{
    int n=0;
    unsigned int ChipID;

    if (pTable == NULL)
    {
        if(pBtDevice->GetChipId(pBtDevice) !=BT_FUNCTION_SUCCESS)
            goto error;

        ChipID = pBtDevice->pBTInfo->ChipType;

        for (n=0;n<MAX_TXGAIN_TABLE_SIZE;n++)
        {
            pBtDevice->TXGainTable[n]=BT_DEFAULT_TX_GAIN_TABLE[ChipID][n];
        }
    }
    else
    {
        for (n=0;n<MAX_TXGAIN_TABLE_SIZE;n++)
        {
            pBtDevice->TXGainTable[n]=pTable[n];
        }
    }
    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

unsigned char BT_DEFAULT_TX_DAC_TABLE[][MAX_TXDAC_TABLE_SIZE]=
{
    {0x0b,0x0c,0x0d,0x0e,0x0f}, //RTL8723A
    {0x11,0x12,0x13,0x14,0x15}, //RTL8723B
    {0x11,0x12,0x13,0x14,0x16}, //RTL8821A
    {0x0b,0x0c,0x0d,0x0e,0x0f}  //RTL8761A
};

int BTDevice_SetTxDACTable(BT_DEVICE *pBtDevice, unsigned char *pTable)
{
    int n=0;
    unsigned int ChipID;

    if (pTable == NULL)
    {
        if (pBtDevice->GetChipId(pBtDevice) !=BT_FUNCTION_SUCCESS)
            goto error;

        ChipID = pBtDevice->pBTInfo->ChipType;

        for (n=0;n<MAX_TXDAC_TABLE_SIZE;n++)
        {
            pBtDevice->TXDACTable[n]=BT_DEFAULT_TX_DAC_TABLE[ChipID][n];
        }
    }
    else
    {
        for (n=0;n<MAX_TXDAC_TABLE_SIZE;n++)
        {
            pBtDevice->TXDACTable[n]=pTable[n];
        }
    }
    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int BTDevice_SetWhiteningCoeff(BT_DEVICE *pBtDevice, uint8_t WhiteningCoeffValue)
{
    int rtn = BT_FUNCTION_SUCCESS;

    if (WhiteningCoeffValue > 0x7f)
    {
        rtn = pBtDevice->SetMdRegMaskBits(pBtDevice,0x2e,7,7,0);
    }
    else
    {
        rtn = pBtDevice->SetMdRegMaskBits(pBtDevice,0x2e,7,7,1);
        if (rtn != BT_FUNCTION_SUCCESS)
            return rtn;

        rtn = pBtDevice->SetMdRegMaskBits(pBtDevice,0x32,8,2,WhiteningCoeffValue);
    }

    return rtn;
}

int BTDevice_SetPayloadType(BT_DEVICE *pBtDevice,BT_PAYLOAD_TYPE PayloadType)
{
    int rtn=BT_FUNCTION_SUCCESS;

    rtn=pBtDevice->SetMdRegMaskBits(pBtDevice,0x2e,6,4,PayloadType);

    return rtn;
}

int BTDevice_SetPacketType(BT_DEVICE *pBtDevice,BT_PKT_TYPE PktType)
{
    int rtn = BT_FUNCTION_SUCCESS;
    uint8_t PktBandWidth = 0;
    uint16_t Payload_length = 0;

    switch (PktType) {
        case BT_PKT_1DH1: PktBandWidth=1;  break;
        case BT_PKT_1DH3: PktBandWidth=1;  break;
        case BT_PKT_1DH5: PktBandWidth=1;  break;
        case BT_PKT_2DH1: PktBandWidth=2;  break;
        case BT_PKT_2DH3: PktBandWidth=2;  break;
        case BT_PKT_2DH5: PktBandWidth=2;  break;
        case BT_PKT_3DH1: PktBandWidth=3;  break;
        case BT_PKT_3DH3: PktBandWidth=3;  break;
        case BT_PKT_3DH5: PktBandWidth=3;  break;
        case BT_PKT_LE:   PktBandWidth=0;  break;
        default:
                          rtn=FUNCTION_ERROR;
    }

    if (rtn != FUNCTION_ERROR) {
        Payload_length = Arrary_PayloadLength[PktType];
    }

    if (PktType == BT_PKT_1DH1)
        rtn = pBtDevice->SetMdRegMaskBits(pBtDevice, 0x2e, 3, 2, 0x1);
    else
        rtn = pBtDevice->SetMdRegMaskBits(pBtDevice, 0x2e, 3, 2, 0x2);

    if (PktBandWidth != 0) {

        rtn = pBtDevice->SetMdRegMaskBits(pBtDevice,0x2c,15,14,PktBandWidth);
        if (rtn != BT_FUNCTION_SUCCESS) {
            goto exit;
        }

        rtn = pBtDevice->SetMdRegMaskBits(pBtDevice,0x2c,12,0,Payload_length);
    } else {
        rtn = pBtDevice->SetMdRegMaskBits(pBtDevice,0x2c,12,0,Payload_length);
    }

exit:
    return rtn;
}

int BTDevice_SetFWPowerTrackEnable(BT_DEVICE *pBtDevice,unsigned char FWPowerTrackEnable)
{
    uint8_t pPayload[MAX_HCI_COMANND_BUF_SIZ];
    uint8_t pEvent[MAX_HCI_EVENT_BUF_SIZ];
    uint32_t EventLen = 0;
    switch (FWPowerTrackEnable)
    {
        case 0:     *(pPayload+0) = 0;  //disable
        case 0xFF:  *(pPayload+0) = 2;  //default vale
        default:    *(pPayload+0) = FWPowerTrackEnable;  //any value
    }

    if (pBtDevice->SendHciCommandWithEvent(pBtDevice,0xFC69,1,pPayload,0x0E,pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    if (pEvent[5] != 0x00)
    {
        goto error;
    }

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int BTDevice_SetHciReset(BT_DEVICE *pBtDevice,int Delay_mSec)
{
    uint8_t pEvent[MAX_HCI_EVENT_BUF_SIZ];
    uint32_t EventLen = 0;
    uint8_t pPayload[1]={0};

    //Send reset command
    if (pBtDevice->SendHciCommandWithEvent(pBtDevice,0x0C03,0,pPayload,0x0E,pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }

    if (pEvent[5] != 0x00)
    {
        goto error;
    }

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int BTDevice_SetHitTarget(BT_DEVICE *pBtDevice, uint64_t HitTarget)
{
    unsigned long pAccessCode[4];
    int i=0;
    for (i=0;i<4;i++)
        pAccessCode[i]=0;

    if (pBtDevice->HitTargetAccessCodeGen(pBtDevice,HitTarget,pAccessCode) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x1c,15,0,pAccessCode[0]) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x1e,15,0,pAccessCode[1]) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x20,15,0,pAccessCode[2]) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x22,15,0,pAccessCode[3]) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    return BT_FUNCTION_SUCCESS;

exit:
    return FUNCTION_ERROR;
}

int BTBASE_GetPayLoadTypeValidFlag(BT_DEVICE *pBtDevice,BT_TEST_MODE TestMode,BT_PKT_TYPE PKT_TYPE,unsigned int *ValidFlag)
{
    int i=0,index=0;
    int *pFlag=NULL;
    int Arrary_PayloadType_PseudoOuterMODE_ValidFlag[3][BT_PAYLOAD_TYPE_NUM]=
    {
        {1,1,1,1,1,1,1,1},      //General
        {1,1,0,1,0,0,1,1},      //LE
    };
    int Arrary_PayloadType_DUTMODE_ValidFlag[3][BT_PAYLOAD_TYPE_NUM]=
    {
        {1,1,1,0,0,0,1,0},      //General
        {1,1,0,1,0,0,1,1},      //LE
    };

    if (PKT_TYPE == BT_PKT_LE)
        index=1;
    else
        index=0;

    switch (TestMode) {
    case BT_DUT_MODE:
        pFlag=Arrary_PayloadType_DUTMODE_ValidFlag[index];
        break;
    case BT_PSEUDO_MODE:
        pFlag=Arrary_PayloadType_PseudoOuterMODE_ValidFlag[index];
        break;
    default:
        goto error;
    }

    if (ValidFlag != NULL) {
        for (i=0;i<BT_PAYLOAD_TYPE_NUM;i++)
            ValidFlag[i]=pFlag[i];
    }

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int BTBASE_HitTargetAccessCodeGen(BT_DEVICE *pBtDevice, uint64_t HitTarget,unsigned long *pAccessCode)
{
#if 1
    BOOL AccessCode[72];
    BOOL LC_PN_SEQ_MSB[32]   = {1,0,0,0,0,0,1,1,1,0,0,0,0,1,0,0,1,0,0,0,1,1,0,1,1,0,0,1,0,1,1,0};      //LC_PN_SEQ_MSB=0x83848d96
    BOOL LC_GEN_POLY_MSB[32] = {0,1,1,0,0,0,0,1,0,1,0,1,1,1,0,0,0,1,0,0,1,1,1,1,0,1,1,0,1,0,1,0};      //LC_GEN_POLY_MSB=0x615c4f6a
    BOOL LC_GEN_POLY_LSB[32] = {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};      //LC_GEN_POLY_LSB=0x40000000
    BOOL p33_p0[34]          = {1,0,1,0,1,1,1,0,1,1,1,1,0,0,1,1,0,0,0,1,0,1,0,1,0,0,1,1,1,1,1,1,0,0};  //PN_seq(P33~P0) = 0x2bbcc54fc
    BOOL BDAddress[48]; //6 byte  ,48bit
    BOOL LAP_INV[25], LAP_BK[31], barker_seq[33], temp[33], mod2[33], mod1[33];
    BOOL Parity[35], hold_mod2[33], hold_mod1[33], tmp;      //Bit0 is not valid

    int i=0, j=0;

    for (i=0;i<72;i++)
    {
        AccessCode[i]=0;
    }

    for (i=47;i>=0;i--)
    {
        BDAddress[i]=(int)((HitTarget>>i) &0x01);
    }

    //Extract LAP from BD Address
    for (i=1; i<=24; i++)
    {
        LAP_INV[i]     =  *(BDAddress + i-1);
        LAP_BK[i]      =  LAP_INV[i];
    }
    if (LAP_INV[24] == 0)
    {
        LAP_BK[25] =  0;
        LAP_BK[26] =  0;
        LAP_BK[27] =  1;
        LAP_BK[28] =  1;
        LAP_BK[29] =  0;
        LAP_BK[30] =  1;
    }
    else
    {
        LAP_BK[25] =  1;
        LAP_BK[26] =  1;
        LAP_BK[27] =  0;
        LAP_BK[28] =  0;
        LAP_BK[29] =  1;
        LAP_BK[30] =  0;
    }
    for (i=1; i<=30; i++)
    {
        barker_seq[i] = LAP_BK[31-i];
    }
    barker_seq[31] = 0;
    barker_seq[32] = 0;

    for (i=1; i<=30; i++)
    {
        mod2[i] = barker_seq[i] ^ LC_PN_SEQ_MSB[i-1];
    }
    mod2[31] = 0;
    mod2[32] = 0;

    for (i=1; i<=30; i++)
    {
        mod1[i] = 0;
    }

    for (i=1; i<=30; i++)
    {
        if (mod2[1] == 1)
        {
            if (mod1[1] == 1)
            {
                for (j=1; j<=31; j++)  //shift
                    mod2[j] = mod2[j+1];
                mod2[32] = 1;
            }
            else
            {
                for (j=1; j<=31; j++)  //shift
                    mod2[j] = mod2[j+1];

                mod2[32] = 0;
            }//end if (mod1[1] == 1)

            for (j=1; j<=31; j++)
                mod1[j] = mod1[j+1];

            mod1[32] = 0;

            for (j=1; j<=32; j++)
            {
                mod2[j] = mod2[j] ^ LC_GEN_POLY_MSB[j-1];
                mod1[j] = mod1[j] ^ LC_GEN_POLY_LSB[j-1];
            }//end for (j=1; j<=32; j++)
        }
        else //if (mod2[1] == 1) else 
        {
            if (mod1[1] == 1)
            {
                for (j=1; j<=31; j++)  //shift
                    mod2[j] = mod2[j+1];

                mod2[32] = 1;
            }
            else
            {
                for (j=1; j<=31; j++)  //shift
                    mod2[j] = mod2[j+1];

                mod2[32] = 0;
            }//end (mod1[1] == 1)

            for (j=1; j<=31; j++)
                mod1[j] = mod1[j+1];

            mod1[32] = 0;
        }//end if (mod2[1] == 1)
    } //end for (i=1; i<=30; i++)

    for (i=1; i<=30; i++)
        hold_mod2[i] = mod2[i+2];

    hold_mod2[31] = 0;
    hold_mod2[32] = 0;

    for (i=1; i<=30; i++)
        hold_mod1[i] = 0;

    hold_mod1[31] = mod1[1];
    hold_mod1[32] = mod1[2];

    for (i=1; i<=32; i++)
    {
        temp[i] = hold_mod2[i] | hold_mod1[i];
        Parity[i+2] = temp[i];
    }

    Parity[1] = mod2[1];
    Parity[2] = mod2[2];

    //assign AccessCode
    for (i=1; i<=34; i++)
    {
        tmp = Parity[35-i] ^ p33_p0[34-i];
        AccessCode[i+3] = tmp;
    }

    for (i=38; i<=67; i++)
        AccessCode[i] = LAP_BK[i-37];


    if (AccessCode[4] == 1)
    {
        AccessCode[0] = 0;
        AccessCode[1] = 1;
        AccessCode[2] = 0;
        AccessCode[3] = 1;
        AccessCode[4] = 0;
    }
    else
    {
        AccessCode[0] = 1;
        AccessCode[1] = 0;
        AccessCode[2] = 1;
        AccessCode[3] = 0;
        AccessCode[4] = 1;
    } //end if (AccessCode[4] == 1)

    if (AccessCode[67] == 1)
    {
        AccessCode[68] = 0;
        AccessCode[69] = 1;
        AccessCode[70] = 0;
        AccessCode[71] = 1;
    }
    else
    {
        AccessCode[68] = 1;
        AccessCode[69] = 0;
        AccessCode[70] = 1;
        AccessCode[71] = 0;
    }   //end  if (AccessCode[67] == 1)

    for (i=0;i<4;i++)
    {
        for (j=0;j<16;j++)
        {
            if (AccessCode[67-((i*16)+j)] == 0)
                pAccessCode[i]=pAccessCode[i]|(0x0<<j);
            else
                pAccessCode[i]=pAccessCode[i]|(0x1<<j);
        }

    }

#else

#endif
    return BT_FUNCTION_SUCCESS;
}

int
BTDevice_SendHciCommandWithEvent(
        BT_DEVICE *pBtDevice,
        uint16_t OpCode,
        uint8_t PayLoadLength,
        uint8_t *pPayLoad,
        uint8_t EventType,
        uint8_t *pEvent,
        uint32_t *pEventLen
        )
{
    uint32_t len;
    uint8_t pWritingBuf[MAX_HCI_COMANND_BUF_SIZ];
    uint8_t hci_rtn = 0;
    uint8_t n = 0;
    BASE_INTERFACE_MODULE *pBaseInterface = pBtDevice->pBaseInterface;

    len = PayLoadLength + 3;
    pWritingBuf[0] = OpCode & 0xFF;
    pWritingBuf[1] = (OpCode >> 0x08) & 0xFF;
    pWritingBuf[2] = PayLoadLength;
    for (n = 0; n < PayLoadLength; n++)
    {
        pWritingBuf[3 + n] = pPayLoad[n];
    }

    if (pBtDevice->SendHciCmd(pBtDevice, HCIIO_BTCMD, pWritingBuf, len) != BT_FUNCTION_SUCCESS)
    {
        SYSLOGI("BTDevice_SendHciCommandWithEvent, ERROR: SendHciCmd");
        goto exit;
    }

    if (((OpCode == 0xFC20) && ((pPayLoad[0] & 0x80) != 0)) || (OpCode == 0x0C03))
    {
        pBaseInterface->WaitMs(pBaseInterface,965);
    }

    if (pBtDevice->RecvHciEvent(pBtDevice, HCIIO_BTEVT, pEvent, pEventLen) != BT_FUNCTION_SUCCESS)
    {
        SYSLOGI("BTDevice_SendHciCommandWithEvent, ERROR: RecvHciEvent");
        goto exit;
    }

    switch (EventType) {
        case 0x0e:
            hci_rtn = pEvent[5];
            break;
        case 0x0F:
            break;
        default:
            break;
    }

    if (hci_rtn != 0x00)
    {
        SYSLOGI("BTDevice_SendHciCommandWithEvent, ERROR: hci_rtn %d", hci_rtn);
        goto exit;
    }

    return BT_FUNCTION_SUCCESS;

exit:
    return FUNCTION_ERROR;
}

int
BTDevice_RecvAnyHciEvent(
        BT_DEVICE *pBtDevice,
        unsigned char *pEvent
        )
{
    uint32_t Retlen =0;
    if (pBtDevice->RecvHciEvent(pBtDevice, HCIIO_BTEVT, pEvent, &Retlen) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    return BT_FUNCTION_SUCCESS;

exit:
    return FUNCTION_ERROR;
}
