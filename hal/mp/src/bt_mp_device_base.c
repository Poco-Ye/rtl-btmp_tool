#define LOG_TAG "bt_mp_device_base"

#include <math.h>

#include "bt_syslog.h"
#include "bluetoothmp.h"
#include "bt_mp_device_efuse_base.h"
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
    uint8_t pPayload[HCI_CMD_LEN_MAX];
    uint8_t pEvent[HCI_EVT_LEN_MAX];
    uint32_t EvtLen;

    //Register Address
    *(pPayload+0) = Addr & 0xff;
    *(pPayload+1) = (Addr & 0xff00) >> 8;

    //Register Value
    *(pPayload+2) = Val & 0xff;
    *(pPayload+3) = (Val & 0xff00) >> 8;

    if (bt_default_SendHciCommandWithEvent(pBtDevice, 0xfc64, LEN_4_BYTE, pPayload, 0x0E, pEvent, &EvtLen))
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
    uint8_t pPayload[HCI_CMD_LEN_MAX];
    uint8_t pEvent[HCI_EVT_LEN_MAX];
    uint32_t EvtLen;


    //Register Address
    *(pPayload+0) = Addr & 0xff;
    *(pPayload+1) = (Addr & 0xff00) >> 8;

    if (bt_default_SendHciCommandWithEvent(pBtDevice, 0xfc65, LEN_2_BYTE, pPayload, 0x0E, pEvent, &EvtLen))
        goto error;

    *pVal = ( (*(pEvent+EVT_BYTE1)<<8) + *(pEvent+EVT_BYTE0 ) );

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
BTDevice_SetMDRegMaskBits(
        BT_DEVICE *pBtDevice,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        )
{
    return bt_default_SetMDRegMaskBits(pBtDevice, Addr, Msb, Lsb, UserValue);
}

int
BTDevice_GetMDRegMaskBits(
        BT_DEVICE *pBt,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        )
{
    return bt_default_GetMDRegMaskBits(pBt, Addr, Msb, Lsb, pUserValue);
}

int
BTDevice_SetRFRegMaskBits(
        BT_DEVICE *pBt,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        )
{
    return bt_default_SetRFRegMaskBits(pBt, Addr, Msb, Lsb, UserValue);
}

int
BTDevice_GetRFRegMaskBits(
        BT_DEVICE *pBt,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        )
{
    return bt_default_GetRFRegMaskBits(pBt, Addr, Msb, Lsb, pUserValue);
}

int
BTDevice_SetSysRegMaskBits(
        BT_DEVICE *pBtDevice,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        )
{
    return bt_default_SetSysRegMaskBits(pBtDevice, Addr, Msb, Lsb, UserValue);
}

int
BTDevice_GetSysRegMaskBits(
        BT_DEVICE *pBtDevice,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        )
{
    return bt_default_GetSysRegMaskBits(pBtDevice, Addr, Msb, Lsb, pUserValue);
}

int
BTDevice_SetBBRegMaskBits(
        BT_DEVICE *pBtDevice,
        int Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        )
{
    return bt_default_SetBBRegMaskBits(pBtDevice, Page, Addr, Msb, Lsb, UserValue);
}

int
BTDevice_GetBBRegMaskBits(
        BT_DEVICE *pBtDevice,
        int Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        )
{
    return bt_default_GetBBRegMaskBits(pBtDevice, Page, Addr, Msb, Lsb, pUserValue);
}

int
BTDevice_SendHciCommandWithEvent(
        BT_DEVICE *pBtDevice,
        uint16_t  OpCode,
        uint8_t PayLoadLength,
        uint8_t *pPayLoad,
        uint8_t  EventType,
        uint8_t  *pEvent,
        uint32_t *pEventLen
        )
{
    return bt_default_SendHciCommandWithEvent(pBtDevice, OpCode, PayLoadLength, pPayLoad, EventType, pEvent, pEventLen);
}

int
BTDevice_RecvAnyHciEvent(
        BT_DEVICE *pBtDevice,
        uint8_t *pEvent
        )
{
    return bt_default_RecvAnyHciEvent(pBtDevice, pEvent);
}

static int
BTDevice_SetTestMode(
        BT_DEVICE *pBtDevice,
        BT_TEST_MODE TestMode
        )
{
    int rtn=BT_FUNCTION_SUCCESS;

    switch (TestMode) {
    case BT_DUT_MODE:
        if (bt_default_SetMDRegMaskBits(pBtDevice,0x2e,11,8,0x0E) !=BT_FUNCTION_SUCCESS)
        {
            rtn=FUNCTION_ERROR;
            goto exit;
        }
        if (bt_default_SetMDRegMaskBits(pBtDevice,0x3c,12,12,0x1) !=BT_FUNCTION_SUCCESS)
        {
            rtn=FUNCTION_ERROR;
            goto exit;
        }
        break;

    case BT_PSEUDO_MODE:
        /* disable modem fix tx */
        if (bt_default_SetMDRegMaskBits(pBtDevice,0x3c,12,12,0x0) !=BT_FUNCTION_SUCCESS)
        {
            rtn=FUNCTION_ERROR;
            goto exit;
        }
        /* enable pesudo outter mode */
        rtn=bt_default_SetMDRegMaskBits(pBtDevice,0x2e,8,8,0x1);
        if (rtn !=BT_FUNCTION_SUCCESS)
        {
            rtn=FUNCTION_ERROR;
            goto exit;
        }
        break;

    default:
        rtn=FUNCTION_PARAMETER_ERROR;
        goto exit;
    }

exit:
    return rtn;
}

static int
BTDevice_SetTxChannel(
        BT_DEVICE *pBtDevice,
        uint8_t ChannelNumber
        )
{
    int rtn=BT_FUNCTION_SUCCESS;
    /* set rf standby mode */
    if (ChannelNumber >=79)
        return FUNCTION_PARAMETER_INVALID_CHANNEL;

    rtn=bt_default_SetRFRegMaskBits(pBtDevice,0x00,15,0,0x1000);
    if (rtn != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    rtn=bt_default_SetRFRegMaskBits(pBtDevice,0x3F,15,0,ChannelNumber&0x7F);

exit:
    return rtn;
}

static int
BTDevice_SetRxChannel(
        BT_DEVICE *pBtDevice,
        uint8_t ChannelNumber
        )
{
    int rtn=BT_FUNCTION_SUCCESS;
    if (ChannelNumber >=79)
        return FUNCTION_PARAMETER_INVALID_CHANNEL;
    /* set rf standby mode */
    rtn=bt_default_SetRFRegMaskBits(pBtDevice,0x00,15,0,0x1000);
    if (rtn != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    /* set rf channel */
    rtn=bt_default_SetRFRegMaskBits(pBtDevice,0x3F,15,0,0x80 | (ChannelNumber&0x7F));

exit:
    return rtn;
}

static int
BTDevice_SetPacketHeader(
        BT_DEVICE *pBtDevice,
        uint32_t pktHeader
        )
{
    int rtn = BT_FUNCTION_SUCCESS;

    rtn = bt_default_SetMDRegMaskBits(pBtDevice, 0x30, 15, 0, pktHeader & 0xFFFF);
    if (rtn != BT_FUNCTION_SUCCESS) {
        goto exit;
    }

    rtn = bt_default_SetMDRegMaskBits(pBtDevice, 0x32, 1, 0, (pktHeader>>16) & 0x3);
    if (rtn != BT_FUNCTION_SUCCESS) {
        goto exit;
    }

exit:
    return rtn;
}

static int
BTDevice_SetMutiRxEnable(
        BT_DEVICE *pBtDevice,
        int IsMultiPktRx
        )
{
    int rtn=BT_FUNCTION_SUCCESS;
    if (IsMultiPktRx)
    {
        rtn = bt_default_SetMDRegMaskBits(pBtDevice,0x32,9,9,1);
    }
    else
    {
        rtn = bt_default_SetMDRegMaskBits(pBtDevice,0x32,9,9,0);
    }

    return rtn;
}

static int
BTDevice_SetWhiteningCoeff(
        BT_DEVICE *pBtDevice,
        uint8_t WhiteningCoeffValue
        )
{
    int rtn = BT_FUNCTION_SUCCESS;

    if (WhiteningCoeffValue > 0x7f)
    {
        rtn = bt_default_SetMDRegMaskBits(pBtDevice,0x2e,7,7,0);
    }
    else
    {
        rtn = bt_default_SetMDRegMaskBits(pBtDevice,0x2e,7,7,1);
        if (rtn != BT_FUNCTION_SUCCESS)
            return rtn;

        rtn = bt_default_SetMDRegMaskBits(pBtDevice,0x32,8,2,WhiteningCoeffValue);
    }

    return rtn;
}

static int
BTDevice_SetPayloadType(
        BT_DEVICE *pBtDevice,
        BT_PAYLOAD_TYPE PayloadType
        )
{
    int rtn=BT_FUNCTION_SUCCESS;

    rtn=bt_default_SetMDRegMaskBits(pBtDevice,0x2e,6,4,PayloadType);

    return rtn;
}

static int
BTDevice_SetPacketType(
        BT_DEVICE *pBtDevice,
        BT_PKT_TYPE PktType
        )
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
        rtn = bt_default_SetMDRegMaskBits(pBtDevice, 0x2e, 3, 2, 0x1);
    else
        rtn = bt_default_SetMDRegMaskBits(pBtDevice, 0x2e, 3, 2, 0x2);

    if (PktBandWidth != 0) {

        rtn = bt_default_SetMDRegMaskBits(pBtDevice,0x2c,15,14,PktBandWidth);
        if (rtn != BT_FUNCTION_SUCCESS) {
            goto exit;
        }

        rtn = bt_default_SetMDRegMaskBits(pBtDevice,0x2c,12,0,Payload_length);
    } else {
        rtn = bt_default_SetMDRegMaskBits(pBtDevice,0x2c,12,0,Payload_length);
    }

exit:
    return rtn;
}

static int
BTBASE_HitTargetAccessCodeGen(
        BT_DEVICE *pBtDevice,
        uint64_t HitTarget,
        unsigned long *pAccessCode
        )
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

static int
BTDevice_SetHitTarget(
        BT_DEVICE *pBtDevice,
        uint64_t HitTarget
        )
{
    unsigned long pAccessCode[4];
    int i=0;
    for (i=0;i<4;i++)
        pAccessCode[i]=0;

    if (BTBASE_HitTargetAccessCodeGen(pBtDevice,HitTarget,pAccessCode) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    if (bt_default_SetMDRegMaskBits(pBtDevice,0x1c,15,0,pAccessCode[0]) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    if (bt_default_SetMDRegMaskBits(pBtDevice,0x1e,15,0,pAccessCode[1]) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    if (bt_default_SetMDRegMaskBits(pBtDevice,0x20,15,0,pAccessCode[2]) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    if (bt_default_SetMDRegMaskBits(pBtDevice,0x22,15,0,pAccessCode[3]) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    return BT_FUNCTION_SUCCESS;

exit:
    return FUNCTION_ERROR;
}

int
BTDevice_SetPowerGainIndex(
        BT_DEVICE *pBtDevice,
        uint8_t Index
        )
{
    uint32_t ChipType;
    uint8_t pEvent[HCI_EVT_LEN_MAX];
    uint8_t TxGainIndex;
    uint8_t PowerGainValue = 0;
    uint32_t EvtLen = 0;
    int rtn = BT_FUNCTION_SUCCESS;

    ChipType = pBtDevice->pBTInfo->ChipType;

    if(ChipType < RTK_BT_CHIP_ID_RTL8822B)
    {
        PowerGainValue = pBtDevice->TXGainTable[Index-1];
        /* set rf standby mode */
        rtn = bt_default_SetRFRegMaskBits(pBtDevice,0x00,15,0,0x1000);
        if (rtn != BT_FUNCTION_SUCCESS)
        {
            goto exit;
        }

        rtn = BTDevice_SetPowerGain(pBtDevice,PowerGainValue);
    } else {
        TxGainIndex = Index - 1;
        if(bt_default_SendHciCommandWithEvent(pBtDevice, 0xfce7, LEN_1_BYTE, &TxGainIndex, 0x0E, pEvent, &EvtLen) != BT_FUNCTION_SUCCESS)
            goto exit;
    }

exit:
    return rtn;
}

int
BTDevice_SetPowerGain(
        BT_DEVICE *pBtDevice,
        uint8_t PowerGainValue
        )
{
    int rtn = BT_FUNCTION_SUCCESS;

    /* set rf standby mode */
    rtn = bt_default_SetRFRegMaskBits(pBtDevice,0x00,15,0,0x1000);
    if (rtn != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    rtn = bt_default_SetRFRegMaskBits(pBtDevice,0x02,15,8,PowerGainValue);
    if (rtn != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    return rtn;

exit:
    return rtn;
}

int
BTDevice_SetPowerDac(
        BT_DEVICE *pBtDevice,
        uint8_t DacValue
        )
{
    int rtn=BT_FUNCTION_SUCCESS;

    rtn=bt_default_SetMDRegMaskBits(pBtDevice,0x38,7,3,DacValue);

    return rtn;
}

int
BTDevice_GetStage(
        BT_DEVICE *pBtDevice,
        uint8_t *pStage
        )
{
    uint16_t data;

    data = 5;
    if (bt_default_SetSysRegMaskBits(pBtDevice, 0xa3, 7, 0, data))
        goto error;

    data = 0;
    if (bt_default_GetSysRegMaskBits(pBtDevice, 0xa0,7, 0, &data))
        goto error;

    *pStage = (uint8_t)(data & 0xFF);

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
BTDevice_SetRTL8761Xtal(
        BT_DEVICE *pBtDevice,
        uint16_t Value
        )
{
    uint32_t data_32=0;
    uint16_t data_lsb=0, data_msb=0;
    uint16_t x1=0, x2=0;

    //set 0x2c [12:17]=[18:23]=Xtal Val
    if (bt_default_GetSysRegMaskBits(pBtDevice, 0x2c, 15, 0, &data_lsb))
        goto error;

    if (bt_default_GetSysRegMaskBits(pBtDevice, 0x2e, 15, 0, &data_msb))
        goto error;

    data_32 = (data_msb << 16) | data_lsb;

    data_32 &= 0xFF000FFF;

    data_32 |= Value << 12;
    data_32 |= Value << 18;

    x1 = data_32 & 0xFFFF;
    x2 = (data_32 >> 16) & 0xFFFF;

    if (bt_default_SetSysRegMaskBits(pBtDevice, 0x2c, 15, 0, x1))
        goto error;

    if (bt_default_SetSysRegMaskBits(pBtDevice, 0x2e, 15, 0, x2))
        goto error;

    SYSLOGI("BTDevice_SetRTL8761Xtal : 0x%04x", Value);

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
BTDevice_GetRTL8761Xtal(
        BT_DEVICE *pBtDevice,
        uint16_t *pValue
        )
{
    uint16_t data = 0;

    if (bt_default_GetSysRegMaskBits(pBtDevice, 0x2e, 7, 2, &data))
        goto error;

    *pValue = data;

    SYSLOGI("BTDevice_GetRTL8761Xtal : 0x%04x", *pValue);

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int BTDevice_ReadThermal(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        uint8_t *pThermalValue
        )
{
    uint16_t Value;
    uint32_t ChipType;

    // chips have different read-thermal procedures
    if (BTDevice_GetBTChipVersionInfo(pBtDevice) !=BT_FUNCTION_SUCCESS)
        goto error;

    ChipType = pBtDevice->pBTInfo->ChipType;

    if (bt_default_SetRFRegMaskBits(pBtDevice, 0x00, 15, 0, 0x1000))
        goto error;

    // enable thermal meter
    if (ChipType <= RTK_BT_CHIP_ID_RTL8763A) {
        if (bt_default_SetRFRegMaskBits(pBtDevice, 0x04, 5, 5, 1))
            goto error;
    } else {
        if (bt_default_SetRFRegMaskBits(pBtDevice, 0x04, 6, 6, 1))
            goto error;
    }

    if (ChipType <= RTK_BT_CHIP_ID_RTL8763A) {
        if (bt_default_GetRFRegMaskBits(pBtDevice, 0x04, 4, 0, &Value))
            goto error;
    } else if (ChipType <= RTK_BT_CHIP_ID_RTL8723C) {
        if (bt_default_SetRFRegMaskBits(pBtDevice, 0x0e, 15, 15, 1))
            goto error;
        if (bt_default_GetRFRegMaskBits(pBtDevice, 0x3b, 5, 0, &Value))
            goto error;
        if (bt_default_SetRFRegMaskBits(pBtDevice, 0x0e, 15, 15, 0))
            goto error;
    } else if (ChipType <= RTK_BT_CHIP_ID_RTL8723D) {
        if (bt_default_GetRFRegMaskBits(pBtDevice, 0x04, 5, 0, &Value))
            goto error;
    }

    *pThermalValue = (uint8_t)Value;

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
BTDevice_TxPowerTracking(
    BT_DEVICE *pBtDevice,
    BOOL openPwrTracking
    )
{
    uint8_t pPayload[HCI_CMD_LEN_MAX];
    uint8_t pEvent[HCI_EVT_LEN_MAX];
    uint32_t EvtLen = 0;

    if (openPwrTracking == true)
    {
        //Enable TX Power Tracking
        pPayload[0] = 1;
    } else {
        //Disable TX Power Tracking
        pPayload[0] = 0;
    }

    if (bt_default_SendHciCommandWithEvent(pBtDevice, 0xFCE8, LEN_1_BYTE, pPayload, 0x0E, pEvent, &EvtLen))
        goto error;

    SYSLOGI("BTDevice_TxPowerTracking = %d", openPwrTracking);

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
BTDevice_TestModeEnable(
        BT_DEVICE *pBtDevice
        )
{
    uint16_t OpCode;
    uint8_t pPayload[HCI_CMD_LEN_MAX];
    uint8_t pEvent[HCI_EVT_LEN_MAX];
    uint32_t EvtLen;
    int status;

    //Set BT HCI Scan Enable
    OpCode = OPCODE( OCF_HCI_WRITE_SCAN_ENABLE, OGF_CONTROLER_AND_BB);
    pPayload[0] = 3;
    if (bt_default_SendHciCommandWithEvent(pBtDevice, OpCode, LEN_1_BYTE, pPayload, 0x0E, pEvent, &EvtLen))
        goto error;

    //Set BT HCI Test Mode
    pPayload[0] = 0x02;
    pPayload[1] = 0x00;
    pPayload[2] = 0x02;
    if (bt_default_SendHciCommandWithEvent(pBtDevice, 0x0c05, LEN_3_BYTE, pPayload, 0x0E, pEvent, &EvtLen))
        goto error;

    OpCode = OPCODE( OCF_HCI_ENABLE_DUT_MODE, OGF_TESTING);
    pPayload[0] = 0x00;
    if (bt_default_SendHciCommandWithEvent(pBtDevice, OpCode, LEN_0_BYTE, pPayload, 0x0E, pEvent, &EvtLen))
        goto error;

    //SetBTHCIScanActivity

    //page sacn Interval:
    *(pPayload+0) = BT_SCAN_INTERVAL & 0xff;
    *(pPayload+1) = (BT_SCAN_INTERVAL & 0xff00) >> 8;

    //page sacn window
    *(pPayload+2) = BT_SCAN_WINDOW & 0xff;
    *(pPayload+3) = (BT_SCAN_WINDOW & 0xff00) >> 8;

    if (bt_default_SendHciCommandWithEvent(pBtDevice, 0xc1c, LEN_4_BYTE, pPayload, 0x0E, pEvent, &EvtLen))
        goto error;

    OpCode = (pEvent[4] << 8) + pEvent[3];
    status = pEvent[5];

    //Inquiry sacn Interval:
    *(pPayload+0) = BT_SCAN_INTERVAL & 0xff;
    *(pPayload+1) = (BT_SCAN_INTERVAL & 0xff00) >> 8;

    //Inquiry sacn window
    *(pPayload+2) = BT_SCAN_WINDOW & 0xff;
    *(pPayload+3) = (BT_SCAN_WINDOW & 0xff00) >> 8;

    if (bt_default_SendHciCommandWithEvent(pBtDevice, 0xc1e, LEN_4_BYTE, pPayload, 0x0E, pEvent, &EvtLen))
        goto error;

    OpCode = (pEvent[4] << 8) + pEvent[3];
    status = pEvent[5];

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
BTDevice_SetResetMDCount(
        BT_DEVICE *pBtDevice
        )
{
    int rtn = BT_FUNCTION_SUCCESS;

    /* reset report counter */
    if (bt_default_SetMDRegMaskBits(pBtDevice,0x2e,11,9,0x00) != BT_FUNCTION_SUCCESS)
    {
        rtn = FUNCTION_HCISEND_ERROR;
        goto exit;
    }

    if (bt_default_SetMDRegMaskBits(pBtDevice,0x2e,11,9,0x07) != BT_FUNCTION_SUCCESS)
    {
        rtn = FUNCTION_HCISEND_ERROR;
        goto exit;
    }

exit:
    return rtn;
}

#define CON_TX  0
#define PKT_TX  1

int
BTDevice_SetPktRxStop(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        )
{
    uint16_t OpCode = 0x0000;
    int pktType = pParam->mPacketType;

    SYSLOGI("+BTDevice_SetPktRxStop");

    if (BTDevice_SetMutiRxEnable(pBtDevice, 0x00) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    if (bt_default_SetMDRegMaskBits(pBtDevice, 0x2e, 15, 0, 0x0070) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    //Back to Shut Down mode
    if (bt_default_SetRFRegMaskBits(pBtDevice, 0x00, 15, 0, 0x0000) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    SYSLOGI("-BTDevice_SetPktRxStop");
    return BT_FUNCTION_SUCCESS;

exit:
    SYSLOGI("-BTDevice_SetPktRxStop: ERROR");
    return FUNCTION_ERROR;
}

int
BTDevice_SetPktRxBegin(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        )
{
    int rtn=BT_FUNCTION_SUCCESS;
    unsigned long btClockTime=0;
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
    if (bt_default_SetMDRegMaskBits(pBtDevice,0x3c,12,12,0x0) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    // set payload type
    if (BTDevice_SetPayloadType(pBtDevice,pParam->mPayloadType) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    // set rate and payload length
    if (BTDevice_SetPacketType(pBtDevice,pParam->mPacketType) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    // set packet header
    if (BTDevice_SetPacketHeader(pBtDevice,pParam->mPacketHeader)!= BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    //set target bd address
    if (BTDevice_SetHitTarget(pBtDevice,pParam->mHitTarget) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    // set WhiteningCoeffValue
    if (BTDevice_SetWhiteningCoeff(pBtDevice,pParam->mWhiteningCoeffValue) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    //set channel
    if (BTDevice_SetRxChannel(pBtDevice,pParam->mChannelNumber) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    //multi-packet Rx
    if (BTDevice_SetMutiRxEnable(pBtDevice, 1) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    //set test mode
    if (BTDevice_SetTestMode(pBtDevice, BT_PSEUDO_MODE) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    if (BTDevice_SetResetMDCount(pBtDevice) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    if (bt_default_SetMDRegMaskBits(pBtDevice, 0x2e, 1, 1, 0x00) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    if (bt_default_SetMDRegMaskBits(pBtDevice, 0x2e, 1, 1, 0x01) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    SYSLOGI("-BTDevice_SetPktRxBegin");
    return BT_FUNCTION_SUCCESS;

exit:
    SYSLOGI("-BTDevice_SetPktRxBegin: ERROR");
    return FUNCTION_ERROR;
}

int
BTDevice_SetPktRxUpdate(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        )
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
    if (bt_default_GetMDRegMaskBits(pBtDevice,0x70,15,10,&rxPin) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    pBtReport->RxRssi = (int)(rxPin);
    pBtReport->RxRssi = pBtReport->RxRssi * 2 - 96;

    //if (pBtReport->RxRssi > -89)
    {
        //rx Count
        if (bt_default_GetMDRegMaskBits(pBtDevice, 0x72, 15, 0, &rxCount) != BT_FUNCTION_SUCCESS)
        {
            goto exit;
        }
        //rx error Bit
        if (bt_default_GetMDRegMaskBits(pBtDevice, 0x78, 15, 0, &rxErrbits) != BT_FUNCTION_SUCCESS)
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

            if (bt_default_SetMDRegMaskBits(pBtDevice,0x2e,10,9,0x00) != BT_FUNCTION_SUCCESS)
            {
                goto exit;
            }
            if (bt_default_SetMDRegMaskBits(pBtDevice,0x2e,10,9,0x03) != BT_FUNCTION_SUCCESS)
            {
                goto exit;
            }
            PktRxCount = pBtReport->TotalRxCounts;
            PktRxErrBits = pBtReport->TotalRxErrorBits;
        }
    }

    SYSLOGI("-BTDevice_Set Pkt Rx Update: RxRssi= %d, rxCount = %d, ber=%f",
            pBtReport->RxRssi, rxCount, pBtReport->ber);
    return BT_FUNCTION_SUCCESS;

exit:
    SYSLOGI("-BTDevice_Set Pkt Rx Update: ERROR");
    return FUNCTION_ERROR;
}

static int
BTDevice_GetBTClockTime(
        BT_DEVICE *pBtDevice,
        unsigned long *btClockTime
        )
{
    int rtn=BT_FUNCTION_SUCCESS;
    int i=0;
    unsigned long Time=0;
    unsigned char pPayload[HCI_CMD_LEN_MAX];
    unsigned char pEvent[HCI_EVT_LEN_MAX];
    uint32_t EventLen = 0;
    pPayload[0]=0x00;
    pPayload[1]=0x00;
    pPayload[2]=0x00;

    if (bt_default_SendHciCommandWithEvent(pBtDevice,0x1407,3,pPayload,0x0E,pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
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

static int
BTDevice_CalculatedTxBits(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport,
        int pktTx_conTx,
        uint32_t *txbits,
        uint32_t *txpkt_cnt
        )
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

int
BTDevice_SetPktTxStop(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        )
{
    uint16_t OpCode=0x0000;
    int pktType = pParam->mPacketType;

    SYSLOGI("+BTDevice_SetPktTxStop");

    //disable multi-packet Tx
    if (BTDevice_SetMutiRxEnable(pBtDevice, 0x00) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    if (bt_default_SetMDRegMaskBits(pBtDevice, 0x2e, 15, 0, 0x0070) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    //Back to Shut Down mode
    if (bt_default_SetRFRegMaskBits(pBtDevice, 0x00, 15, 0, 0x0000) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    SYSLOGI("-BTDevice_SetPktTxStop");
    return BT_FUNCTION_SUCCESS;

exit:
    SYSLOGI("-BTDevice_SetPktTxStop: ERROR");
    return FUNCTION_ERROR;
}

static int
BTDevice_SetPktTxBegin_PSEUDOMODE(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam
        )
{
    unsigned long NewModemReg4Value =0;

    /* disable continous tx mode */
    if ( bt_default_SetMDRegMaskBits(pBtDevice,0x2e,12,12,0x00) != BT_FUNCTION_SUCCESS)
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
    if (bt_default_SetMDRegMaskBits(pBtDevice,0x04,15,0,NewModemReg4Value) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    //Dummy Tx bits
    //[a] switch page-2
    if (bt_default_SetMDRegMaskBits(pBtDevice,0x00,15,0,0x02) != BT_FUNCTION_SUCCESS) 
    {
        goto exit;
    }

    if (bt_default_SetMDRegMaskBits(pBtDevice,0x3e,6,0, ((TXDUMMYPATTEN & 0x03)<<5)|(TXDUMMYBITS & 0x1F)) != BT_FUNCTION_SUCCESS) 
    {
        goto exit;
    }

    //[b] switch page-0
    if (bt_default_SetMDRegMaskBits(pBtDevice,0x00,15,0,0x00) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    /* generate neg-edge pulse to trigger */
    if (bt_default_SetMDRegMaskBits(pBtDevice,0x2e,0,0,0x01) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    if (bt_default_SetMDRegMaskBits(pBtDevice,0x2e,0,0,0x00) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    return BT_FUNCTION_SUCCESS;

exit:
    return FUNCTION_ERROR;
}

int
BTDevice_SetPktTxBegin(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        )
{
    int rtn = BT_FUNCTION_SUCCESS;
    unsigned long btClockTime = 0;
    uint16_t tmp = 0;
    BT_TRX_TIME *pTxTime = &pBtDevice->TRxTime[TX_TIME_RUNING];
    uint32_t ChipType;

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

    ChipType = pBtDevice->pBTInfo->ChipType;

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
    if (bt_default_SetMDRegMaskBits(pBtDevice,0x3c,12,12,0x0) != BT_FUNCTION_SUCCESS) {
        goto exit;
    }

    // set payload type
    if (BTDevice_SetPayloadType(pBtDevice,pParam->mPayloadType)  != BT_FUNCTION_SUCCESS) {
        goto exit;
    }

    // set rate and payload length
    if (BTDevice_SetPacketType(pBtDevice,pParam->mPacketType) != BT_FUNCTION_SUCCESS) {
        goto exit;
    }

    // set packet header
    if (BTDevice_SetPacketHeader(pBtDevice,pParam->mPacketHeader)!= BT_FUNCTION_SUCCESS) {
        goto exit;
    }

    //set target bd address
    if (BTDevice_SetHitTarget(pBtDevice,pParam->mHitTarget) != BT_FUNCTION_SUCCESS) {
        goto exit;
    }

    // set WhiteningCoeffValue
    if (BTDevice_SetWhiteningCoeff(pBtDevice,pParam->mWhiteningCoeffValue) != BT_FUNCTION_SUCCESS) {
        goto exit;
    }

    //set channel
    if (BTDevice_SetTxChannel(pBtDevice,pParam->mChannelNumber) != BT_FUNCTION_SUCCESS) {
        goto exit;
    }

    //set outpower
    if(ChipType < RTK_BT_CHIP_ID_RTL8822B){
        if ((pParam->mTxGainIndex > 0) && (pParam->mTxGainIndex <= 7))
            rtn = BTDevice_SetPowerGainIndex(pBtDevice,pParam->mTxGainIndex);
        else
            rtn = BTDevice_SetPowerGain(pBtDevice,pParam->mTxGainValue);

        if (rtn != BT_FUNCTION_SUCCESS){
            goto exit;
        }
    } else {
        rtn = BTDevice_SetPowerGainIndex(pBtDevice,pParam->mTxGainIndex);
        if (rtn != BT_FUNCTION_SUCCESS){
            goto exit;
        }
    }

    //set test mode
    if (BTDevice_SetTestMode(pBtDevice, BT_PSEUDO_MODE) != BT_FUNCTION_SUCCESS) {
        goto exit;
    }

    if (BTDevice_SetMutiRxEnable(pBtDevice, 1) != BT_FUNCTION_SUCCESS) {
        goto exit;
    }


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

int
BTDevice_SetPktTxUpdate(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        )
{
    unsigned long NewModemReg4Value = 0;
    uint16_t tmp = 0;
    uint32_t TXUpdateBits = 0;
    uint32_t TXPktUpdateCnts = 0;

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
        if (bt_default_SetMDRegMaskBits(pBtDevice,0x04,15,0,NewModemReg4Value) != BT_FUNCTION_SUCCESS)
        {
            goto exit;
        }
        if (bt_default_SetMDRegMaskBits(pBtDevice,0x2e,0,0,0x01) != BT_FUNCTION_SUCCESS)
        {
            goto exit;
        }
        if (bt_default_SetMDRegMaskBits(pBtDevice,0x2e,0,0,0x00) != BT_FUNCTION_SUCCESS)
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

int
BTDevice_SetHoppingMode(
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
    uint8_t pEvent[HCI_EVT_LEN_MAX];
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

        if (bt_default_SendHciCommandWithEvent(pBtDevice, 0xfc75, LEN_11_BYTE, pPayLoad, 0x0E, pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
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

    if (bt_default_SendHciCommandWithEvent(pBtDevice, 0xFD45, 1, pPayLoad, 0x0E, pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
    {
        rtn = FUNCTION_HCISEND_ERROR;
        goto exit;
    }

    if (pEvent[5] != 0x00)
        rtn = FUNCTION_HCISEND_STAUTS_ERROR;

exit:
    return rtn;
}

static int
BTDevice_SetContinueTxBegin_PSEUDOMODE(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam
        )
{
    int PacketType = pParam->mPacketType;
    //LE
    if (PacketType == BT_PKT_LE)
    {
        if (pParam->mChannelNumber > 39)
        {
            goto error;
        }
        if (bt_default_SetMDRegMaskBits(pBtDevice,0x3c,5,5,0x01) != BT_FUNCTION_SUCCESS)
        {
            goto error;
        }
    }
    //Continue Tx mode
    if (bt_default_SetMDRegMaskBits(pBtDevice,0x2e,12,12,0x01) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    //Generate Negedge Pulse
    if (bt_default_SetMDRegMaskBits(pBtDevice,0x2e,0,0,0x01) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    if (bt_default_SetMDRegMaskBits(pBtDevice,0x2e,0,0,0x00) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
BTDevice_SetContinueTxBegin(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        )
{
    int rtn = BT_FUNCTION_SUCCESS;
    unsigned long btClockTime = 0;
    BT_TRX_TIME *pTxTime = &pBtDevice->TRxTime[TX_TIME_RUNING];
    uint32_t ChipType;

    ChipType = pBtDevice->pBTInfo->ChipType;

    pTxTime->beginTimeClockCnt=0;
    pTxTime->UseTimeClockCnt=0;
    pTxTime->endTimeClockCnt=0;

    if (pBtReport != NULL)
    {
        pBtReport->TotalTXBits=0;
        pBtReport->TotalTxCounts=0;
    }

    SYSLOGI("+BTDevice_SetContinueTxBegin: mChannelNumber 0x%02x, mPacketType 0x%02x, "
          "mTxGainIndex 0x%02x, mTxGainValue 0x%02x, mTxPacketCount 0x%04x, mPayloadType 0x%02x, "
          "mPacketHeader 0x%08x, mWhiteningCoeffValue 0x%02x, mTxDAC 0x%02x, mHitTarget 0x%012llx",
          pParam->mChannelNumber, pParam->mPacketType, pParam->mTxGainIndex, pParam->mTxGainValue,
          pParam->mTxPacketCount, pParam->mPayloadType, pParam->mPacketHeader,
          pParam->mWhiteningCoeffValue, pParam->mTxDAC, pParam->mHitTarget);

    //disable modem fix tx
    // RTK_UPDATE_MODEM_REG(TRANS_MODEM_REG(0x3C), BIT12, 0);
    if (bt_default_SetMDRegMaskBits(pBtDevice,0x3c,12,12,0x0) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    // set payload type
    if (BTDevice_SetPayloadType(pBtDevice,pParam->mPayloadType) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    // set rate and payload length
    if (BTDevice_SetPacketType(pBtDevice,pParam->mPacketType) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    // set packet header
    if (BTDevice_SetPacketHeader(pBtDevice,pParam->mPacketHeader) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    //set target bd address
    if (BTDevice_SetHitTarget(pBtDevice,pParam->mHitTarget) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    // set WhiteningCoeffValue
    if (BTDevice_SetWhiteningCoeff(pBtDevice, pParam->mWhiteningCoeffValue) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    //set test mode
    if (BTDevice_SetTestMode(pBtDevice, BT_PSEUDO_MODE) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    //set channel
    if (BTDevice_SetTxChannel(pBtDevice,pParam->mChannelNumber) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }
    //set outpower
    if(ChipType < RTK_BT_CHIP_ID_RTL8822B){
        if ((pParam->mTxGainIndex > 0) && (pParam->mTxGainIndex <= 7))
            rtn = BTDevice_SetPowerGainIndex(pBtDevice,pParam->mTxGainIndex);
        else
            rtn = BTDevice_SetPowerGain(pBtDevice,pParam->mTxGainValue);

        if (rtn != BT_FUNCTION_SUCCESS){
            goto error;
        }
    } else {
        rtn = BTDevice_SetPowerGainIndex(pBtDevice,pParam->mTxGainIndex);
        if (rtn != BT_FUNCTION_SUCCESS){
            goto error;
        }
    }

    if (BTDevice_SetMutiRxEnable(pBtDevice,0) != BT_FUNCTION_SUCCESS)
    {
        goto error;
    }

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

int
BTDevice_SetContinueTxStop(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        )
{
    SYSLOGI("+BTDevice_SetContinueTxStop");

    if (bt_default_SetMDRegMaskBits(pBtDevice, 0x2e, 15, 0, 0x0070) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    //Back to Standby Mode from Shut Down mode
    if (bt_default_SetRFRegMaskBits(pBtDevice,0x00,15,0,0x0000) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    SYSLOGI("-BTDevice_SetContinueTxStop");
    return BT_FUNCTION_SUCCESS;

exit:
    SYSLOGI("-BTDevice_SetContinueTxStop: ERROR");
    return FUNCTION_ERROR;
}

int
BTDevice_SetContinueTxUpdate(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        )
{
    uint32_t TXUpdateBits = 0;
    uint32_t TXPktUpdateCnts = 0;

    if ((pBtReport == NULL) || (pParam == NULL))
        goto exit;

    //report
    BTDevice_CalculatedTxBits(pBtDevice, pParam, pBtReport, CON_TX, &TXUpdateBits, &TXPktUpdateCnts);

    pBtReport->TotalTXBits += TXUpdateBits;

    pBtReport->TotalTxCounts += TXPktUpdateCnts;

    SYSLOGI("BTDevice_SetContinueTxUpdate");

    return BT_FUNCTION_SUCCESS;

exit:
    SYSLOGI("BTDevice_SetContinueTxUpdate: ERROR");
    return FUNCTION_ERROR;
}

int
BTDevice_LeTxTestCmd(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        )
{
    uint8_t pPayload[HCI_CMD_LEN_MAX];
    uint8_t pEvent[HCI_EVT_LEN_MAX];
    uint32_t EventLen = 0;
    unsigned long TxGain;
    uint8_t TxGainIndex;
    uint32_t ChipType;

    ChipType = pBtDevice->pBTInfo->ChipType;

    pPayload[0] = pParam->mChannelNumber; // channel: 0~39
    pPayload[1] = pParam->mParamData[0]; // Length_Of_Test_Data: 0x00~0x25
    pPayload[2] = pParam->mPayloadType; // Packet_Payload

    SYSLOGI("BTDevice_LeTxTestCmd: Channel %d, Data length %d, Pkt Payload %d",
            pPayload[0], pPayload[1], pPayload[2]);

    if (pPayload[0] > 39 || pPayload[1] > 0x25 || pPayload[2] > 7)
    {
        goto exit;
    }

    if (pParam->mPacketType == BT_PKT_LE)
    {
        if (ChipType < RTK_BT_CHIP_ID_RTL8822B)
        {
            TxGain = (pBtDevice->TXGainTable[pParam->mTxGainIndex-1] << 8);

            bt_default_SetMDRegMaskBits(pBtDevice, 0x3C, 12, 12, 0);
            bt_default_SetBBRegMaskBits(pBtDevice, 0, 0x178, 15, 0, TxGain);
            bt_default_SetBBRegMaskBits(pBtDevice, 0, 0x17A, 15, 0, 0x382);
        }
        else
        {
            TxGainIndex = pParam->mTxGainIndex-1;

            if(bt_default_SendHciCommandWithEvent(pBtDevice, 0xfce7, LEN_1_BYTE, &TxGainIndex, 0x0E, pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
            {
                goto exit;
            }
        }
    }

    if (bt_default_SendHciCommandWithEvent(pBtDevice, 0x201E, LEN_3_BYTE, pPayload, 0x0E, pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
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

int
BTDevice_LeRxTestCmd(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        )
{
    uint8_t pPayload[HCI_CMD_LEN_MAX];
    uint8_t pEvent[HCI_EVT_LEN_MAX];
    uint32_t EventLen = 0;

    pPayload[0] = pParam->mChannelNumber; // channel: 0~39
    pBtReport->TotalRxCounts  = 0;

    if (pPayload[0] > 39)
    {
        SYSLOGI("BTDevice_LeRxTestCmd: Channel error %d", pPayload[0]);
        goto exit;
    }

    if (bt_default_SendHciCommandWithEvent(pBtDevice, 0x201D, LEN_1_BYTE, pPayload, 0x0E, pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
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

int
BTDevice_LeTestEndCmd(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        )
{
    uint8_t pPayload[HCI_CMD_LEN_MAX];
    uint8_t pEvent[HCI_EVT_LEN_MAX];
    uint32_t EventLen = 0;

    if (bt_default_SendHciCommandWithEvent(pBtDevice, 0x201F, LEN_0_BYTE, pPayload, 0x0E, pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
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

unsigned char BT_DEFAULT_TX_GAIN_TABLE[][MAX_TXGAIN_TABLE_SIZE] = {
    {0x2d,0x30,0x6d,0x70,0xb0,0xcd,0xd0},   //RTL8723A
    {0x2b,0x2e,0x6b,0x6e,0x8b,0x8e,0xce},   //RTL8723B
    {0x28,0x2b,0x48,0x4b,0xc8,0xcc,0xe9},   //RTL8821A
    {0x0d,0x49,0x4d,0x69,0x89,0x8d,0xa9},   //RTL8761A
    {0x2b,0x2e,0x6b,0x6e,0x8b,0x8e,0xce},   //RTL8703A
    {0x0d,0x49,0x4d,0x69,0x89,0x8d,0xa9},   //RTL8763A
    {0x08,0x26,0x36,0x38,0x3a,0x48,0x4a},   //RTL8703B
    {0x08,0x26,0x36,0x38,0x3a,0x48,0x4a},   //RTL8723C
    {0x16,0x18,0x32,0x34,0x36,0x38,0x54},   //RTL8822B -->TODO
    {0x58,0x70,0x76,0x7c,0x96,0x9c,0xb8},   //RTL8723D
};

int
BTDevice_SetTxGainTable(
        BT_DEVICE *pBtDevice,
        unsigned char *pTable
        )
{
    int n=0;
    unsigned int ChipID;

    if (pTable == NULL)
    {
        SYSLOGI("BTDevice_SetTxGainTable: default");
        if (BTDevice_GetBTChipVersionInfo(pBtDevice) != BT_FUNCTION_SUCCESS)
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

unsigned char BT_DEFAULT_TX_DAC_TABLE[][MAX_TXDAC_TABLE_SIZE]= {
    {0x0b,0x0c,0x0d,0x0e,0x0f}, //RTL8723A
    {0x11,0x12,0x13,0x14,0x15}, //RTL8723B
    {0x11,0x12,0x13,0x14,0x16}, //RTL8821A
    {0x0b,0x0c,0x0d,0x0e,0x0f}, //RTL8761A
    {0x0b,0x0c,0x0d,0x0e,0x0f}, //RTL8703A -->TODO
    {0x0b,0x0c,0x0d,0x0e,0x0f}, //RTL8763A
    {0x0a,0x0b,0x0c,0x0d,0x0e}, //RTL8703B
    {0x0a,0x0b,0x0c,0x0d,0x0e}, //RTL8723C
    {0x0f,0x10,0x11,0x12,0x13}, //RTL8822B
    {0x0b,0x0c,0x0d,0x0e,0x0f}, //RTL8723D
};

int
BTDevice_SetTxDACTable(
        BT_DEVICE *pBtDevice,
        unsigned char *pTable
        )
{
    int n=0;
    unsigned int ChipID;

    if (pTable == NULL)
    {
        SYSLOGI("BTDevice_SetTxDACTable: default");
        if (BTDevice_GetBTChipVersionInfo(pBtDevice) !=BT_FUNCTION_SUCCESS)
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

int
BTDevice_SetHciReset(
        BT_DEVICE *pBtDevice,
        int Delay_mSec
        )
{
    uint8_t pEvent[HCI_EVT_LEN_MAX];
    uint32_t EventLen = 0;
    uint8_t pPayload[1]={0};

    //Send reset command
    if (bt_default_SendHciCommandWithEvent(pBtDevice,0x0C03,0,pPayload,0x0E,pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
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

int
BTDevice_GetBTChipVersionInfo(
        BT_DEVICE *pBtDevice
        )
{
    uint8_t pPayload[HCI_CMD_LEN_MAX];
    uint8_t pEvent[HCI_EVT_LEN_MAX];
    uint32_t EventLen = 0;
    uint16_t OpCode=0x1001;
    uint8_t pPayload_Len=0;
    int rtn=BT_FUNCTION_SUCCESS;
    BT_CHIPINFO *pBTInfo = pBtDevice->pBTInfo;

    if (bt_default_SendHciCommandWithEvent(pBtDevice,OpCode,pPayload_Len,pPayload,0x0E,pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
    {
        rtn=FUNCTION_HCISEND_ERROR;
        goto exit;
    }

    pBTInfo->HCI_Version =pEvent[EVT_HCI_VERSION];
    pBTInfo->HCI_SubVersion=(pEvent[EVT_HCI_SUBVERSION+1] << 8) | pEvent[EVT_HCI_SUBVERSION];
    pBTInfo->LMP_Version=pEvent[EVT_HCI_LMPVERSION];
    pBTInfo->LMP_SubVersion=(pEvent[EVT_HCI_LMPSUBVERSION+1]<<8) | pEvent[EVT_HCI_LMPSUBVERSION];

    pBTInfo->ChipType=RTK_BT_CHIP_ID_UNKNOWCHIP;
    pBTInfo->Is_After_PatchCode=0;
    pBTInfo->Version=0;

    if (pBTInfo->LMP_SubVersion == 0x1200) {
        pBTInfo->ChipType = RTK_BT_CHIP_ID_RTL8723A;
    } else if ((pBTInfo->LMP_SubVersion == 0x8723) && (pBTInfo->HCI_SubVersion == 0x000B)) {
        pBTInfo->ChipType = RTK_BT_CHIP_ID_RTL8723B;
    } else if ((pBTInfo->LMP_SubVersion == 0x8821) && (pBTInfo->HCI_SubVersion == 0x000A)) {
        pBTInfo->ChipType = RTK_BT_CHIP_ID_RTL8821A;
    } else if ((pBTInfo->LMP_SubVersion == 0x8761) && (pBTInfo->HCI_SubVersion == 0x000A)) {
        pBTInfo->ChipType = RTK_BT_CHIP_ID_RTL8761A;
    } else if ((pBTInfo->LMP_SubVersion == 0x8723) && (pBTInfo->HCI_SubVersion == 0x000A)) {
        pBTInfo->ChipType = RTK_BT_CHIP_ID_RTL8703A;
    } else if ((pBTInfo->LMP_SubVersion == 0x8763) && (pBTInfo->HCI_SubVersion == 0x000A)) {
        pBTInfo->ChipType = RTK_BT_CHIP_ID_RTL8763A;
    } else if ((pBTInfo->LMP_SubVersion == 0x8703) && (pBTInfo->HCI_SubVersion == 0x000B)) {
        pBTInfo->ChipType = RTK_BT_CHIP_ID_RTL8703B;
    } else if ((pBTInfo->LMP_SubVersion == 0x8703) && (pBTInfo->HCI_SubVersion == 0x000B)) {
        pBTInfo->ChipType = RTK_BT_CHIP_ID_RTL8723C;
    } else if ((pBTInfo->LMP_SubVersion == 0x8822) && (pBTInfo->HCI_SubVersion == 0x000B)) {
        pBTInfo->ChipType = RTK_BT_CHIP_ID_RTL8822B;
    } else if ((pBTInfo->LMP_SubVersion == 0x8723) && (pBTInfo->HCI_SubVersion == 0x000D)) {
        pBTInfo->ChipType = RTK_BT_CHIP_ID_RTL8723D;
    } else if ((pBTInfo->LMP_SubVersion == 0x8821) && (pBTInfo->HCI_SubVersion == 0x000C)) {
        pBTInfo->ChipType = RTK_BT_CHIP_ID_RTL8821C;
    } else {
        if (bt_default_GetChipId(pBtDevice) != BT_FUNCTION_SUCCESS) {
            rtn= FUNCTION_ERROR;
            goto exit;
        }
    }

    if (bt_default_GetECOVersion(pBtDevice) != BT_FUNCTION_SUCCESS) {
        rtn= FUNCTION_ERROR;
        goto exit;
    }

    rtn = BT_FUNCTION_SUCCESS;

exit:
    return rtn;
}

int
BTDevice_BTDlFW(
        BT_DEVICE *pBtDevice,
        uint8_t *pPatchcode,
        int patchLength
        )
{
    uint8_t pPayload[HCI_CMD_LEN_MAX];
    uint8_t pEvent[HCI_EVT_LEN_MAX];
    uint16_t OpCode=0xFC20;
    uint8_t pPayload_Len=0;
    uint8_t EventType=0x0e;
    uint32_t EvtLen;
    int rtn=BT_FUNCTION_SUCCESS;
    int StartIndex=0;//,m=0;
    int flag=0,n=0;
    int SegmentIndex=0;
    int LastSegment=patchLength/252;

    if ((patchLength%252) != 0)
        LastSegment+=1;

    while(!flag)
    {
        if ((StartIndex+252) < patchLength)
        {
            pPayload_Len = 252;
        }
        else
        {
            pPayload_Len =  patchLength-StartIndex+1;
            flag=1;
        }
        if (flag)
        {
            pPayload[0] = (SegmentIndex | 0x80);
        }
        else
        {
            pPayload[0] = (SegmentIndex &0x7f);
        }

        for (n=0;n<pPayload_Len;n++)
        {
            pPayload[n+1]=pPatchcode[StartIndex+n];
        }

        if (bt_default_SendHciCommandWithEvent(pBtDevice,OpCode,pPayload_Len+1,pPayload,EventType,pEvent,&EvtLen) != BT_FUNCTION_SUCCESS)
        {
            if (flag)
                rtn=BT_FUNCTION_SUCCESS;
            else
                rtn=FUNCTION_HCISEND_ERROR;

            goto exit;
        }

        //SYSLOGI("[%d][Len=%d][offset=%d][ret=%x](%d)",SegmentIndex,pPayload_Len,StartIndex,pEvent[5],LastSegment);

        if (pEvent[5] != 0)
        {
            rtn=FUNCTION_ERROR;
            goto exit;
        }

        StartIndex+=pPayload_Len;
        SegmentIndex++;

    }

exit:
    return rtn;
}

int
BTDevice_BTDlMergerFW(
        BT_DEVICE *pBtDevice,
        uint8_t *pPatchcode,
        int patchLength
        )
{
    int rtn=BT_FUNCTION_SUCCESS;
    BT_CHIPINFO *pBTInfo = pBtDevice->pBTInfo;
    int Signature_Dword_Len=8;
    int Project_ID_Len=4;
    int Number_of_Total_Patch_Len=2;
    int Chip_ID_Len=2;
    int Patch_Code_Length_Len=2;
    int Start_OffSet_Len=4;

    int Signature_Dword_BaseAddr=0;
    int Project_ID_BaseAddr=Signature_Dword_Len;
    int Number_of_Total_Patch_BaseAddr=Project_ID_BaseAddr+Project_ID_Len;
    int Chip_ID_LEN_BaseAddr =Number_of_Total_Patch_BaseAddr+Number_of_Total_Patch_Len;
    int Start_OffSet_BaseAddr=0;
    int Patch_Code_Length_BaseAddr=0;


    int NumOfTotalPatch=0;
    unsigned int Chip_ID=0x0000;
    int i=0,n=0,BaseAdr=0,Len=0;
    int Patch_Code_Length=0;
    int Start_Offset=0;

    if (pBTInfo->Is_After_PatchCode)
        goto exit;

    //RTL8723B /RTL8821A /RTL8761A
    //"Realtech"
    Start_Offset=0;
    Patch_Code_Length=patchLength;
    if ((pPatchcode[0] == 'R') && (pPatchcode[1] == 'e') && (pPatchcode[2] == 'a') && (pPatchcode[3] == 'l')
            &&(pPatchcode[4] == 't') && (pPatchcode[5] == 'e') && (pPatchcode[6] == 'c') && (pPatchcode[7] == 'h')
            && (pBTInfo->ChipType != RTK_BT_CHIP_ID_RTL8723A))
    {
        //download patch
        for (i=0; i<Number_of_Total_Patch_Len  ; i++) {
            NumOfTotalPatch  +=   (pPatchcode[Number_of_Total_Patch_BaseAddr+i]<<(8*i));
        }
        Patch_Code_Length_BaseAddr=Chip_ID_LEN_BaseAddr+ (NumOfTotalPatch* Chip_ID_Len );
        Start_OffSet_BaseAddr=Patch_Code_Length_BaseAddr+(NumOfTotalPatch* Patch_Code_Length_Len );

        for (i=0;i<NumOfTotalPatch;i++)
        {

            BaseAdr=Chip_ID_LEN_BaseAddr;
            Len=Chip_ID_Len;
            Chip_ID=0;
            for (n=0;n<Len;n++){
                Chip_ID +=((pPatchcode[BaseAdr + (Len*i)+n]&0xFF) <<(n*8));
            }

            if (Chip_ID == pBTInfo->Version)
            {

                BaseAdr=Patch_Code_Length_BaseAddr;
                Len=Patch_Code_Length_Len;
                Patch_Code_Length=0;
                for (n=0;n<Len;n++){
                    Patch_Code_Length+=((pPatchcode[BaseAdr + (Len*i)+n]&0xFF) <<(n*8));
                }
                Start_Offset=0;
                BaseAdr=Start_OffSet_BaseAddr;
                Len=Start_OffSet_Len;
                for (n=0;n<Len;n++){
                    Start_Offset +=((pPatchcode[BaseAdr + (Len*i)+n]&0xFF) <<(n*8));
                }
                break;
            }
        }
    }

    if (Patch_Code_Length ==0) {
        rtn = FUNCTION_ERROR;
        goto exit;
    }

    SYSLOGI(">>Patch offset=%x Len=%d",Start_Offset,Patch_Code_Length);

    if (BTDevice_BTDlFW(pBtDevice,&pPatchcode[Start_Offset],Patch_Code_Length) != BT_FUNCTION_SUCCESS)
    {
        rtn=FUNCTION_ERROR;
        goto exit;
    }

exit:
    return rtn;
}

static int
BTDevice_PGEfuse_ReadBytes(
        BT_DEVICE *pBtDevice,
        uint8_t Bank,
        uint16_t StartAddress,
        uint16_t ReadLen,
        uint8_t *pReadingData
        )
{
    int rtn = BT_FUNCTION_SUCCESS;
    uint16_t OpCode=0xFc6c;
    uint8_t pPayLoad[512];
    uint8_t pEvent[512];
    uint32_t length=0;
    uint32_t EvtLen;
    memset(pPayLoad,0,26);
    memset(pEvent,0,0xff);

    // Close Power Cut
    rtn = bt_default_SetSysRegMaskBits(pBtDevice,0x35,3,3,0x0);
    if (rtn != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    pPayLoad[0]=Bank; //Bank
    pPayLoad[1]=StartAddress&0xff;
    pPayLoad[2]=(StartAddress>>8)&0xff;
    pPayLoad[3]=ReadLen&0xff;
    pPayLoad[4]=(ReadLen>>8)&0xff;

    rtn = bt_default_SendHciCommandWithEvent(pBtDevice,OpCode,5,pPayLoad,0x0e,pEvent,&EvtLen);
    if ((rtn != BT_FUNCTION_SUCCESS) || (pEvent[5] != 0))
    {
        rtn = FUNCTION_ERROR;
        goto exit;
    }
    length = pEvent[7]<<8 | pEvent[6];
    if (ReadLen != length)
    {
        rtn=FUNCTION_ERROR;
        goto exit;
    }
    memcpy(pReadingData,&pEvent[8],length);

exit:
    return rtn;
}

static int
BTDevice_PGEfuse_WriteBytes(
        BT_DEVICE *pBtDevice,
        uint8_t Bank,
        uint16_t StartAddress,
        uint16_t WriteLen,
        uint8_t *pWriteData
        )
{
    int rtn = BT_FUNCTION_SUCCESS;
    uint16_t OpCode=0xFc6b;
    uint8_t pPayLoad[512];
    uint8_t pEvent[512];
    uint8_t length=0;
    uint32_t EvtLen;
    memset(pPayLoad,0,26);
    memset(pEvent,0,0xff);

    // open Power Cut
    rtn = bt_default_SetSysRegMaskBits(pBtDevice,0x35,3,3,0x1);
    if (rtn != BT_FUNCTION_SUCCESS) {
        return rtn;
    }

    pPayLoad[0]=2; //Bank
    pPayLoad[1]=0xff;
    pPayLoad[2]=0x01;
    pPayLoad[3]=0x01;
    pPayLoad[4]=0x00;
    pPayLoad[5]=0x85;
    length=6;

    rtn = bt_default_SendHciCommandWithEvent(pBtDevice,OpCode,1,pPayLoad,0x0e,pEvent,&EvtLen);

    pPayLoad[0]=Bank; //Bank
    pPayLoad[1]=StartAddress&0xff;
    pPayLoad[2]=(StartAddress>>8)&0xff;
    pPayLoad[3]=WriteLen&0xff;
    pPayLoad[4]=(WriteLen>>8)&0xff;
    memcpy(&pPayLoad[5],pWriteData,WriteLen);
    length = 5 + WriteLen;

    rtn = bt_default_SendHciCommandWithEvent(pBtDevice,OpCode,length,pPayLoad,0x0e,pEvent,&EvtLen);
    if ((rtn != BT_FUNCTION_SUCCESS) || (pEvent[5] != 0))
    {
        rtn = FUNCTION_ERROR;
        goto exit;
    }
    length= pEvent[7]<<8|pEvent[6];
    if (WriteLen != length)
    {
        rtn = FUNCTION_ERROR;
        goto exit;
    }

exit:
    // close power cut for read, reWrite the bank for c cut
    Bank = Bank & 0x07;
    bt_default_SetSysRegMaskBits(pBtDevice,0x35,3,0,Bank);
    return rtn;
}

static int
BTDevice_PGEfuse_GetLength(
        BT_DEVICE *pBtDevice,
        uint8_t Bank,
        unsigned int *PGLength
        )
{
    int rtn = BT_FUNCTION_SUCCESS;
    uint32_t n=0,i=0;
    unsigned int  address=0;
    uint8_t pData[512];
    unsigned int  DataBytesLen=0;
    uint8_t Entry_Header[2];
    uint8_t Entry_length=0;
    uint8_t WordSelect=0;
    unsigned int  bflag=0;
    unsigned int  Count=0;

    address=0;
    *PGLength=0;

    while (!bflag)
    {
        Count++;
        rtn = BTDevice_PGEfuse_ReadBytes(pBtDevice,Bank,address,2,pData);
        if (rtn != BT_FUNCTION_SUCCESS)
        {
            goto exit;
        }
        Entry_Header[0]=pData[0];
        Entry_Header[1]=pData[1];

        //[check header]
        if (Entry_Header[0] == 0xFF)
        {
            bflag=1;
            break;
        }
        if (!bflag)
        {
            if ((Entry_Header[0] & 0xF0) == 0xF0)
            { //2 header
                WordSelect = Entry_Header[1];
                Entry_length=2;
            }
            else
            { //1 header
                WordSelect = Entry_Header[0];
                Entry_length=1;
            }
            for (i=0;i<4;i++)
            {
                if (((WordSelect >> i) & 0x01) == 0x00)
                {
                    Entry_length+=2;
                }
            }
            address+=Entry_length;
        }
        if (address > 1024)
        {
            goto exit;
        }

        if (Count > 2048)
        {
            rtn = FUNCTION_ERROR;
            goto exit;
        }
    }
    *PGLength = address;
    rtn = BT_FUNCTION_SUCCESS;

exit:
    return rtn;
}

/**
 *  BD_ADDRESS[0X3C]: Phyical map[1][000][Len=5][73,34,12,78,56,]-->[n=0,03c-34 12][n=1,03e-78 56]
 *  [161]Phyical map[1][00c][Len=4][8f,5e,01,22,]-->[n=0,160-01 22]
 *  [181]Phyical map[1][010][Len=4][0f,6e,44,33,]-->[n=0,180-44 33]
 */

#define MAX_BANK_SIZE 1024

static int
BTDevice_SubPGEfuseRawData(
        BT_DEVICE *pBtDevice,
        int MapType,
        uint8_t *PGData,
        uint32_t PGDataLength
        )
{
    int rtn = BT_FUNCTION_SUCCESS;
    uint32_t Bank1_PGlength=0;
    uint32_t Bank2_PGlength=0;
    uint8_t Bank_Use=2;
    uint32_t Bank_Use_PGlength=0;
    uint32_t address=0;

    if (MapType == SYS_EFUSE)
    {
        rtn = BTDevice_PGEfuse_GetLength(pBtDevice,0,&Bank_Use_PGlength);
        if (rtn != BT_FUNCTION_SUCCESS)
        {
            goto exit;
        }
        Bank_Use=0;
    }
    else // BT_EFUSE
    {
        rtn = BTDevice_PGEfuse_GetLength(pBtDevice,2,&Bank2_PGlength);
        if (rtn != BT_FUNCTION_SUCCESS)
        {
            goto exit;
        }

        if (Bank2_PGlength > 0)
        {
            Bank_Use=2;
            Bank_Use_PGlength=Bank2_PGlength;
        }
        else
        {
            rtn = BTDevice_PGEfuse_GetLength(pBtDevice,1,&Bank1_PGlength);
            if (rtn != BT_FUNCTION_SUCCESS)
            {
                goto exit;
            }
            if (Bank1_PGlength < MAX_BANK_SIZE - 16)
            {
                Bank_Use=1;
                Bank_Use_PGlength=Bank1_PGlength;
            }
        }
    }
    //Dum
    //Write Efuse
    rtn = BTDevice_PGEfuse_WriteBytes(pBtDevice,Bank_Use,Bank_Use_PGlength,PGDataLength,PGData);

exit:
    return rtn;
}

int
BTDevice_PGEfuseRawData(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam
        )
{
    int rtn=BT_FUNCTION_SUCCESS;

    uint8_t Command;

    Command= pParam->mPGRawData[0];
    switch (Command)
    {
        case BT_EFUSE:
            rtn = BTDevice_SubPGEfuseRawData(pBtDevice,BT_EFUSE,&pParam->mPGRawData[2],pParam->mPGRawData[1]);
            break;

        case SYS_EFUSE:
            rtn = BTDevice_SubPGEfuseRawData(pBtDevice,SYS_EFUSE,&pParam->mPGRawData[2],pParam->mPGRawData[1]);
            break;

        default:
            rtn= FUNCTION_ERROR;
            break;
    }

    return rtn;
}


int
BTDevice_WriteEfuseLogicalData(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam
        )
{
    uint8_t Cmd;
    EFUSE_UNIT *pEfuse;
    uint16_t Addr;
    uint8_t  Len;
    uint8_t *pBuf;

    Cmd = pParam->mPGRawData[0];
    switch (Cmd) {
    case BT_EFUSE:
        pEfuse = pBtDevice->pBtEfuse;
        break;

    case SYS_EFUSE:
        pEfuse = pBtDevice->pSysEfuse;
        break;

    default:
        goto error;
    }

    if (BTDevice_Efuse_LoadPhyMem(pEfuse))
        goto error;

    if (BTDevice_Efuse_Phy2LogMap(pEfuse))
        goto error;

    Addr = pParam->mPGRawData[1] | (pParam->mPGRawData[2] << 8);
    Len = pParam->mPGRawData[3];
    pBuf = &(pParam->mPGRawData[4]);
    BTDevice_Efuse_UpdateLogMem(pEfuse, Addr, Len, pBuf);

    if (BTDevice_Efuse_Log2PhyMap(pEfuse))
        goto error;

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
BTDevice_ReadEfuseLogicalData(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        BT_DEVICE_REPORT *pBtReport
        )
{
    int rtn = BT_FUNCTION_SUCCESS;

    uint8_t Cmd;
    uint16_t EfuseLogicalAddr;
    EFUSE_UNIT *pEfuse;
    uint8_t i, Len;

    Cmd = pParam->mPGRawData[0];
    EfuseLogicalAddr = (pParam->mPGRawData[2]<<8) | pParam->mPGRawData[1];
    Len = pParam->mPGRawData[3];

    switch (Cmd) {
    case BT_EFUSE:
        pEfuse = pBtDevice->pBtEfuse;
        break;

    case SYS_EFUSE:
        pEfuse = pBtDevice->pSysEfuse;
        break;

    default:
        goto error;
        break;
    }

    if (BTDevice_Efuse_LoadPhyMem(pEfuse))
        goto error;

    if (BTDevice_Efuse_Phy2LogMap(pEfuse))
        goto error;

    memcpy(pBtReport->ReportData, pParam->mPGRawData, LEN_4_BYTE);

    for (i = 0; i < Len; i++) {
        pBtReport->ReportData[i+LEN_4_BYTE] = pEfuse->pEfuseLogMem[i+EfuseLogicalAddr].OldValue;
    }

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}
