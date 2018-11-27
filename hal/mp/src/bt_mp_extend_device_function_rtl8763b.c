#include "bt_mp_extend_device_function_rtl8763b.h"
#include "bt_syslog.h"

//new

/*XTAL Track enable bit: 0x496[3]
XTAL Track table: 0x112 ~ 0x12B ?Hbyte???????@26byte
SET_XTAL_TRACK_EN:

opcode: 0xfd17

param_len: 2

param[0] = 0 //subopcode

param[1] = enable(1) or disable(0)

e.g. [e] 0x17, 0xfd, 2, 0, 1;

GET_XTAL_TRACK_EN:

opcode: 0xfd17

param_len: 1

param[0] = 1 //subopcode

e.g. [e] 0x17, 0xfd, 1, 1;

SET_XTAL_TRACK_TABLE:

opcode: 0xfd17

param_len: 27

param[0] = 2 //subopcode

param[1] ~ param[26] = tracking table entry

e.g. [e] 0x17, 0xfd, 27, 2, 1,2,3,4,5,6,7,8,9,10, 1,2,3,4,5,6,7,8,9,10, 1,2,3,4,5,6;

GET_XTAL_TRACK_TABLE:

opcode: 0xfd17

param_len: 1

param[0] = 3 //subopcode

e.g. [e] 0x17, 0xfd, 1, 3;

*/
  //


//ADDRESS DEFINE






//------------------------------------------------
typedef struct _BT_EXTEND_DEVICE_CONTEXT_TAG{
    unsigned char  TX_POWER_GAIN_K;
    unsigned char  TX_POWER_FLATNESS[2];
    unsigned char  TX_PATH_LOSS_MODULE;

    unsigned char pExtendData[256];

}BT_EXTEND_DEVICE_CONTEXT;

static BT_EXTEND_DEVICE_CONTEXT  bt_extend_device_context;



/*------------------------------------------------------*/
//  RTL8763B
/*------------------------------------------------------*/
int
BTDevice_Extend_XTAL_TRACK_Func(
    BT_DEVICE *pBtDevice,
    unsigned char  SubIndex,
    unsigned char *pData
    )
{
    int rtn= FUNCTION_ERROR;
    unsigned char pPayload[LEN_512_BYTE];
    unsigned char pEvtBuf[LEN_512_BYTE];
    unsigned char length=0;
    unsigned int ChipType;
    unsigned int EvtLen;

    SYSLOGI("+BTDevice_Extend_XTAL_TRACK_Func() SubIndex = %d\n",SubIndex );
    ChipType = pBtDevice->pBTInfo->ChipType;
    if(ChipType != RTK_BT_CHIP_ID_RTL8763B)
    {
         goto error;
    }
    memset(pPayload,0,LEN_512_BYTE);

    switch (SubIndex)
    {case EXTEND_XTAL_TRACK_TABLE_GET:   //GET_XTAL_TRACK
        pPayload[0] = 3 ; //SubIndex
        length =1;
     break;
     case EXTEND_XTAL_TRACK_TABLE_SET:   //SET_XTAL_TRACK
        pPayload[0] = 2 ; //SubIndex
        memcpy(&pPayload[1], pData ,26);
        length =27;
     break;
     case EXTEND_XTAL_TRACK_EN_SET:
        pPayload[0] = 0 ; //SET ENABLE/DISABLE
        if (*pData != 0)
            pPayload[1] = 0x01;
        else
            pPayload[1] = 0x00;
        length =2;
     break;
     case EXTEND_XTAL_TRACK_EN_GET:
        pPayload[0] = 1 ; //GET ENABLE
        length =1;
     break;
     default:
           goto error;
    }

    if(pBtDevice->SendHciCommandWithEvent(pBtDevice, HCI_VENDOR_MP_XTAL_TRACK, length, pPayload, 0x0E, pEvtBuf, &EvtLen))
        goto error;

    if (pEvtBuf[5] != 0)
       goto error;

    switch (SubIndex)
    {case EXTEND_XTAL_TRACK_TABLE_GET:   //GET_XTAL_TRACK
                memcpy(pData,&pEvtBuf[6],26);
     case EXTEND_XTAL_TRACK_EN_GET:
                memcpy(pData,&pEvtBuf[6],1);
     break;
    }

    rtn= BT_FUNCTION_SUCCESS;

error:
    SYSLOGI("-BTDevice_Extend_XTAL_TRACK_Func()\n");
    return rtn;

}

int
BTDevice_Extend_TxGainKValue(
    BT_DEVICE *pBtDevice,
    unsigned char *pData
    )
{
    unsigned char pPayload[LEN_512_BYTE];
    unsigned char pEvtBuf[LEN_512_BYTE];
    unsigned int EvtLen;

    SYSLOGI("+BTDevice_Extend_TxGainKValue() \n");
    if (pData[0] == 0)
    {
        //Get
        pPayload[0] =0 ;
        if(pBtDevice->SendHciCommandWithEvent(pBtDevice, HCI_VENDOR_MP_K_POWER_SETTING, LEN_1_BYTE, pPayload, 0x0E, pEvtBuf, &EvtLen))
            goto error;
        memcpy( pData,&pEvtBuf[6],LEN_4_BYTE);
        SYSLOGI(" Tx Gain K= %x ,Flness = %x%x , Path loss = %x \n",pEvtBuf[6],pEvtBuf[8],pEvtBuf[7],pEvtBuf[9]);
        bt_extend_device_context.TX_POWER_GAIN_K =  pEvtBuf[6];
        bt_extend_device_context.TX_POWER_FLATNESS[0] = pEvtBuf[7];
        bt_extend_device_context.TX_POWER_FLATNESS[1] = pEvtBuf[8];
        bt_extend_device_context.TX_PATH_LOSS_MODULE =  pEvtBuf[9];
    }
    else
    {   //Set
        pPayload[0] =1 ;
        pPayload[1] = pData[1];
        if(pBtDevice->SendHciCommandWithEvent(pBtDevice, HCI_VENDOR_MP_K_POWER_SETTING, LEN_2_BYTE, pPayload, 0x0E, pEvtBuf, &EvtLen) != BT_FUNCTION_SUCCESS)
            goto error;
    }
    SYSLOGI("-BTDevice_Extend_TxGainKValue()\n");
    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;
}

int
BTDevice_Extend_TxFlatnessValue(
    BT_DEVICE *pBtDevice,
    unsigned char *pData
    )
{
    unsigned char pPayload[LEN_512_BYTE];
    unsigned char pEvtBuf[LEN_512_BYTE];
    unsigned int EvtLen;

    SYSLOGI("+BTDevice_Extend_TxFlatnessValue()");
    if (pData[0] == 0)
    {
        //Get
        pPayload[0] =0 ;
        if(pBtDevice->SendHciCommandWithEvent(pBtDevice, HCI_VENDOR_MP_K_POWER_SETTING, LEN_1_BYTE, pPayload, 0x0E, pEvtBuf, &EvtLen) != BT_FUNCTION_SUCCESS)
            goto error;
        memcpy( pData,&pEvtBuf[6],LEN_4_BYTE);
        SYSLOGI(" Tx Gain K= %x ,Flness = %x%x , Path loss = %x \n",pEvtBuf[6],pEvtBuf[8],pEvtBuf[7],pEvtBuf[9]);
        bt_extend_device_context.TX_POWER_GAIN_K =  pEvtBuf[6];
        bt_extend_device_context.TX_POWER_FLATNESS[0] = pEvtBuf[7];
        bt_extend_device_context.TX_POWER_FLATNESS[1] = pEvtBuf[8];
        bt_extend_device_context.TX_PATH_LOSS_MODULE =  pEvtBuf[9];
    }
    else
    {   //Set
        pPayload[0] =2 ;
        pPayload[1] = pData[1];
        pPayload[2] = pData[2];
        if(pBtDevice->SendHciCommandWithEvent(pBtDevice, HCI_VENDOR_MP_K_POWER_SETTING, LEN_3_BYTE, pPayload, 0x0E, pEvtBuf, &EvtLen))
            goto error;
    }
    SYSLOGI("-BTDevice_Extend_TxFlatnessValue()\n");
    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;
}


int
BTDevice_Extend_TxPathLossModule(
    BT_DEVICE *pBtDevice,
    unsigned char *pData
    )
{
    unsigned char pPayload[LEN_512_BYTE];
    unsigned char pEvtBuf[LEN_512_BYTE];
    unsigned int EvtLen;
    SYSLOGI("+BTDevice_Extend_TxPathLossModule()");
    if (pData[0] == 0)
    {
        //Get
        pPayload[0] =0 ;
        if(pBtDevice->SendHciCommandWithEvent(pBtDevice, HCI_VENDOR_MP_K_POWER_SETTING, LEN_1_BYTE, pPayload, 0x0E, pEvtBuf, &EvtLen) != BT_FUNCTION_SUCCESS)
            goto error;
        memcpy( pData,&pEvtBuf[6],LEN_4_BYTE);
        SYSLOGI(" Tx Gain K= %x ,Flness = %x %x , Path loss = %x \n",pEvtBuf[6],pEvtBuf[8],pEvtBuf[7],pEvtBuf[9]);
        bt_extend_device_context.TX_POWER_GAIN_K =  pEvtBuf[6];
        bt_extend_device_context.TX_POWER_FLATNESS[0] = pEvtBuf[7];
        bt_extend_device_context.TX_POWER_FLATNESS[1] = pEvtBuf[8];
        bt_extend_device_context.TX_PATH_LOSS_MODULE =  pEvtBuf[9];

    }
    else
    {   //Set
        pPayload[0] =3 ;
        pPayload[1] = pData[1];
        if(pBtDevice->SendHciCommandWithEvent(pBtDevice, HCI_VENDOR_MP_K_POWER_SETTING, LEN_2_BYTE, pPayload, 0x0E, pEvtBuf, &EvtLen))
            goto error;
    }
    SYSLOGI("-BTDevice_Extend_TxPathLossModule()");
    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}
int
BTDevice_Extend_ReportTxInfo(
    BT_DEVICE *pBtDevice,
    unsigned char *pData
    )
{

    unsigned char pPayload[LEN_512_BYTE];
    unsigned char pEvtBuf[LEN_512_BYTE];
    unsigned int EvtLen;
    SYSLOGI("+BTDevice_Extend_ReportTxInfo()");
    //Get
    pPayload[0] =0 ;
    if(pBtDevice->SendHciCommandWithEvent(pBtDevice, HCI_VENDOR_MP_K_POWER_SETTING, LEN_1_BYTE, pPayload, 0x0E, pEvtBuf, &EvtLen) != BT_FUNCTION_SUCCESS)
        goto error;
    memcpy( pData,&pEvtBuf[6],LEN_4_BYTE);
    SYSLOGI(" Tx Gain K= %x ,Flness = %x %x , Path loss = %x \n",pEvtBuf[6],pEvtBuf[8],pEvtBuf[7],pEvtBuf[9]);
    bt_extend_device_context.TX_POWER_GAIN_K =      pEvtBuf[6];
    bt_extend_device_context.TX_POWER_FLATNESS[0] = pEvtBuf[7];
    bt_extend_device_context.TX_POWER_FLATNESS[1] = pEvtBuf[8];
    bt_extend_device_context.TX_PATH_LOSS_MODULE =  pEvtBuf[9];

    SYSLOGI("-BTDevice_Extend_ReportTxInfo()\n");
    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;
}


static unsigned int EXTEND_CONFIG_TXPOWER = 0;
static unsigned int EXTEND_CONFIG_TXFLNESS = 0;
static unsigned int EXTEND_CONFIG_TXFLNESS_ENABLE = 0;
static unsigned int EXTEND_CONFIG_BDADDR = 0;
static unsigned int EXTEND_CONFIG_TXGAINK = 0;
static unsigned int EXTEND_CONFIG_XTALCAP = 0;
static unsigned int EXTEND_CONFIG_TXGAINK_ENABLE = 0;
static unsigned int EXTEND_CONFIG_XTAL32K =0;
static unsigned int EXTEND_CONFIG_XTAL_EN =0;
static unsigned int EXTEND_CONFIG_XTAL_TABLE =0;
static unsigned int EXTEND_CONFIG_TXGAINK_TMETER=0;

static unsigned int EXTEND_CONFIG_BLE2M_2402_2480=0;
static unsigned int EXTEND_CONFIG_PC_SETTING=0;
static unsigned int EXTEND_CONFIG_LBT_ANT_GAIN_SETTING=0;
static unsigned int EXTEND_CONFIG_LBT_EN_SETTING=0;


int
BTDevice_Extend_ConfigExtend_Function(
    BT_DEVICE *pBtDevice,
    unsigned char *pData
    )
{
    int rtn= FUNCTION_ERROR;
    int n=0;
    int subindex = pData[0];
    unsigned int  offset = 0x00;
    unsigned char shift_pWriting =2;
    unsigned char pWriting_len=0;
    unsigned char pWriting[LEN_512_BYTE];
    int Direct_write_to_Device=0;
    BT_CHIPINFO *pBTInfo = pBtDevice->pBTInfo;

    memset(pWriting,0,LEN_512_BYTE);

    if(pBTInfo->Version > 3) //BBPro D cut
    {
        EXTEND_CONFIG_TXPOWER = D_EXTEND_CONFIG_TXPOWER;
        EXTEND_CONFIG_TXFLNESS = D_EXTEND_CONFIG_TXFLNESS;
        EXTEND_CONFIG_TXFLNESS_ENABLE = D_EXTEND_CONFIG_TXFLNESS_ENABLE;
        EXTEND_CONFIG_BDADDR = D_EXTEND_CONFIG_BDADDR;
        EXTEND_CONFIG_TXGAINK = D_EXTEND_CONFIG_TXGAINK;
        EXTEND_CONFIG_XTALCAP = D_EXTEND_CONFIG_XTALCAP;
        EXTEND_CONFIG_TXGAINK_ENABLE = D_EXTEND_CONFIG_TXGAINK_ENABLE;
        EXTEND_CONFIG_XTAL32K = D_EXTEND_CONFIG_XTAL32K;

        EXTEND_CONFIG_XTAL_EN =D_EXTEND_CONFIG_XTAL_TRACK_EN;
        EXTEND_CONFIG_XTAL_TABLE =D_EXTEND_CONFIG_XTAL_TRACK_TABLE;
        EXTEND_CONFIG_TXGAINK_TMETER = D_EXTEND_CONFIG_TxGainK_TMETER;

        EXTEND_CONFIG_BLE2M_2402_2480=D_EXTEND_CONFIG_BLE2M_2402_2480;
        EXTEND_CONFIG_PC_SETTING=D_EXTEND_CONFIG_PC_SETTING;
        EXTEND_CONFIG_LBT_ANT_GAIN_SETTING= D_EXTEND_CONFIG_LBT_ANT_GAIN_SETTING;
        EXTEND_CONFIG_LBT_EN_SETTING=D_EXTEND_CONFIG_LBT_EN_SETTING;

    }
    else
    {
        EXTEND_CONFIG_TXPOWER = C_EXTEND_CONFIG_TXPOWER;
        EXTEND_CONFIG_TXFLNESS = C_EXTEND_CONFIG_TXFLNESS;
        EXTEND_CONFIG_TXFLNESS_ENABLE = C_EXTEND_CONFIG_TXFLNESS_ENABLE;
        EXTEND_CONFIG_BDADDR = C_EXTEND_CONFIG_BDADDR;
        EXTEND_CONFIG_TXGAINK = C_EXTEND_CONFIG_TXGAINK;
        EXTEND_CONFIG_XTALCAP = C_EXTEND_CONFIG_XTALCAP;
        EXTEND_CONFIG_TXGAINK_ENABLE = C_EXTEND_CONFIG_TXGAINK_ENABLE;
        EXTEND_CONFIG_XTAL32K = C_EXTEND_CONFIG_XTAL32K;
        //c cut use D cut layout
        EXTEND_CONFIG_XTAL_EN =D_EXTEND_CONFIG_XTAL_TRACK_EN;
        EXTEND_CONFIG_XTAL_TABLE =D_EXTEND_CONFIG_XTAL_TRACK_TABLE;
        EXTEND_CONFIG_TXGAINK_TMETER = C_EXTEND_CONFIG_TxGainK_TMETER;

        EXTEND_CONFIG_BLE2M_2402_2480=C_EXTEND_CONFIG_BLE2M_2402_2480;
        EXTEND_CONFIG_PC_SETTING=C_EXTEND_CONFIG_PC_SETTING;
        EXTEND_CONFIG_LBT_ANT_GAIN_SETTING= C_EXTEND_CONFIG_LBT_ANT_GAIN_SETTING;
        EXTEND_CONFIG_LBT_EN_SETTING=C_EXTEND_CONFIG_LBT_EN_SETTING;
    }

    switch (subindex)
    {
        case EXTEND_WRITE_CONFIG_BDADDR://  0x10
        Direct_write_to_Device= 1;
        case EXTEND_WRITE_CONFIG_BUF_BDADDR:

        offset = EXTEND_CONFIG_BDADDR;
        pWriting[shift_pWriting+0] =offset & 0xFF ;
        pWriting[shift_pWriting+1] =(offset>>8) & 0xFF;
        pWriting[shift_pWriting+2] =LEN_6_BYTE ;
        memcpy(&pWriting[shift_pWriting+3],&pData[1],LEN_6_BYTE );
        memset(&pWriting[shift_pWriting+3+LEN_6_BYTE ],0xFF,LEN_6_BYTE );
        pWriting_len =  3 +LEN_6_BYTE+LEN_6_BYTE;
        break;

        //---------------------------------------------------------//
        case EXTEND_WRITE_CONFIG_XTALCAP:// 0x11
        Direct_write_to_Device= 1;
        case EXTEND_WRITE_CONFIG_BUF_XTALCAP:

        offset =EXTEND_CONFIG_XTALCAP;  //0x045b 0x045c
/*      pWriting[shift_pWriting+0] =offset & 0xFF ;
        pWriting[shift_pWriting+1] =(offset>>8) & 0xFF;
        pWriting[shift_pWriting+2] = LEN_2_BYTE;
        pWriting[shift_pWriting+3] = pData[1];
        pWriting[shift_pWriting+4] = pData[1];
        pWriting[shift_pWriting+5] = 0xFF;
        pWriting[shift_pWriting+6] = 0XFF;

        pWriting_len =  LEN_7_BYTE; 
*/
        pWriting[shift_pWriting+0] =offset & 0xFF ;  ////0x045b 0x045c
        pWriting[shift_pWriting+1] =(offset>>8) & 0xFF;
        pWriting[shift_pWriting+2] = LEN_1_BYTE;
        pWriting[shift_pWriting+3] = pData[1];
        pWriting[shift_pWriting+4] =0xFF;
        pWriting[shift_pWriting+5] =(offset+1)& 0xFF ;
        pWriting[shift_pWriting+6] =((offset+1)>>8) & 0xFF;
        pWriting[shift_pWriting+7] = LEN_1_BYTE;
        pWriting[shift_pWriting+8] = pData[1];
        pWriting[shift_pWriting+9] =0xFF;

        pWriting_len =  LEN_10_BYTE;

        break;

        //---------------------------------------------------------//
        case  EXTEND_WRITE_CONFIG_TXGAINK:// 0x12
        Direct_write_to_Device= 1;
        case EXTEND_WRITE_CONFIG_BUF_TXGAINK:

        offset = EXTEND_CONFIG_TXGAINK;
        pWriting[shift_pWriting+0] =offset & 0xFF ;
        pWriting[shift_pWriting+1] =(offset>>8) & 0xFF;
        pWriting[shift_pWriting+2] = LEN_1_BYTE;
        pWriting[shift_pWriting+3] = pData[1];
        pWriting[shift_pWriting+4] = 0xFF;  

        offset = EXTEND_CONFIG_TXGAINK_ENABLE;
        pWriting[shift_pWriting+5] =offset & 0xFF ;
        pWriting[shift_pWriting+6] =(offset>>8) & 0xFF;
        pWriting[shift_pWriting+7] = LEN_1_BYTE;
        pWriting[shift_pWriting+8] = 0x01;
        pWriting[shift_pWriting+9] = 0x01;
        pWriting_len = 10;
        break;
        //---------------------------------------------------------//
        case  EXTEND_WRITE_CONFIG_TXFLNESS:// 0x13
        Direct_write_to_Device= 1;
        case EXTEND_WRITE_CONFIG_BUF_TXFLNESS:

        offset = EXTEND_CONFIG_TXFLNESS;
        pWriting[shift_pWriting+0] =offset & 0xFF ;
        pWriting[shift_pWriting+1] =(offset>>8) & 0xFF;
        pWriting[shift_pWriting+2] = LEN_2_BYTE;
        pWriting[shift_pWriting+3] = pData[1] ;//(unsigned char)pData&0xFF;
        pWriting[shift_pWriting+4] = pData[2];//(unsigned char)((FlatnessVal>>8)&0xFF);
        pWriting[shift_pWriting+5] = 0xFF;//(unsigned char)FlatnessMask&0xFF;
        pWriting[shift_pWriting+6] = 0xFF;//(unsigned char)((FlatnessMask>>8)&0xFF);

        //0x419 BIT1 = 1;
        offset = EXTEND_CONFIG_TXFLNESS_ENABLE;
        pWriting[shift_pWriting+7] =offset & 0xFF ;
        pWriting[shift_pWriting+8] =(offset>>8) & 0xFF;
        pWriting[shift_pWriting+9] = LEN_1_BYTE;
        pWriting[shift_pWriting+10] = 0x02;
        pWriting[shift_pWriting+11] = 0x02;
        pWriting_len=  LEN_7_BYTE+LEN_5_BYTE;
        break;

        //---------------------------------------------------------//
        case  EXTEND_WRITE_CONFIG_TXPOWER:// 0x14
        Direct_write_to_Device= 1;
        case EXTEND_WRITE_CONFIG_BUF_TXPOWER:
        offset =EXTEND_CONFIG_TXPOWER;
        /*pWriting[shift_pWriting+0] =offset &0xFF;
        pWriting[shift_pWriting+1] =(offset>>8) & 0xFF;
        pWriting[shift_pWriting+2] =LEN_5_BYTE ;
        memcpy(&pWriting[shift_pWriting+3],&pData[1],LEN_5_BYTE );
        memset(&pWriting[shift_pWriting+3+LEN_5_BYTE ],0xFF,LEN_5_BYTE );
        pWriting_len =  3 +LEN_5_BYTE+LEN_5_BYTE;
        */
        //1M/2M/3M/BLE1M/BLE2M
        pWriting_len =0;
        for (n=0;n<5;n++)
        {
          if (pData[1+n] != 0xFF)
          {
            pWriting[shift_pWriting+pWriting_len+0] =(offset+n) &0xFF;
            pWriting[shift_pWriting+pWriting_len+1] =((offset+n)>>8) & 0xFF;
            pWriting[shift_pWriting+pWriting_len+2] =LEN_1_BYTE ;
            pWriting[shift_pWriting+pWriting_len+3] =pData[1+n];
            pWriting[shift_pWriting+pWriting_len+4] = 0xFF;
            pWriting_len +=5;
          }
        }
        break;
        case EXTEND_WRITE_CONFIG_XTAL32k:// 0x15
        Direct_write_to_Device= 1;
        case EXTEND_WRITE_CONFIG_BUF_XTAL32k:

        offset =EXTEND_CONFIG_XTAL32K;  //0x04DD 0x04DE
/*      pWriting[shift_pWriting+0] =offset & 0xFF ;
        pWriting[shift_pWriting+1] =(offset>>8) & 0xFF;
        pWriting[shift_pWriting+2] = LEN_2_BYTE;
        pWriting[shift_pWriting+3] = pData[1];
        pWriting[shift_pWriting+4] = pData[1];
        pWriting[shift_pWriting+5] = 0xFF;
        pWriting[shift_pWriting+6] = 0XFF;
        pWriting_len =  LEN_7_BYTE;
*/
        pWriting[shift_pWriting+0] =offset & 0xFF ;
        pWriting[shift_pWriting+1] =(offset>>8) & 0xFF;
        pWriting[shift_pWriting+2] = LEN_1_BYTE;
        pWriting[shift_pWriting+3] = pData[1];
        pWriting[shift_pWriting+4] = 0xFF;
        pWriting[shift_pWriting+5] = (offset+1) & 0xFF ;
        pWriting[shift_pWriting+6] = ((offset+1)>>8) & 0xFF;
        pWriting[shift_pWriting+7] = LEN_1_BYTE;
        pWriting[shift_pWriting+8] = pData[1];
        pWriting[shift_pWriting+9] = 0xFF;
        pWriting_len =  LEN_10_BYTE;

        break;

        case EXTEND_ERASE_FLASH_CONFIG_PAGE:// 0x16

        if (BTDevice_Erase_ConfigPage_Entry_bp_level(pBtDevice) != BT_FUNCTION_SUCCESS)
            goto error;

        offset =0;
        break;
        //---------------------------------------------------------//
        case  EXTEND_WRITE_CONFIG_TXGAINK_Tmeter :// 0x17
        Direct_write_to_Device= 1;
        case EXTEND_WRITE_CONFIG_BUF_MODULEK_TMETER :  //0X37

        offset = EXTEND_CONFIG_TXGAINK_TMETER;
        pWriting[shift_pWriting+0] =offset & 0xFF ;
        pWriting[shift_pWriting+1] =(offset>>8) & 0xFF;
        pWriting[shift_pWriting+2] = LEN_1_BYTE;
        pWriting[shift_pWriting+3] = pData[1];
        pWriting[shift_pWriting+4] = 0xFF;
        pWriting_len = 5;
        break;

        case  EXTEND_ERASE_CONFIG_BUF_CLEAN  ://   0x36
              offset =0;
        BT_Device_FlashtempBuf_CleanBuffer(pBtDevice);
        break;

        case EXTEND_WRITE_CONFIG_BUF_WriteToDevice:
            offset =0;
            if (BT_Device_FlashtempBuf_WriteToDevice(pBtDevice)!= BT_FUNCTION_SUCCESS)
               goto error;
           BT_Device_FlashtempBuf_CleanBuffer(pBtDevice);   
           break;

        //------------------- XTAL TRACKING --------------
        case EXTEND_XTAL_TRACK_EN_WRITE_CONFIG:
            Direct_write_to_Device= 1;
        case EXTEND_XTAL_TRACK_EN_WRITE_CONFIG_BUF:
                offset = EXTEND_CONFIG_XTAL_EN;
                pWriting[shift_pWriting+0] =offset & 0xFF ;
                pWriting[shift_pWriting+1] =(offset>>8) & 0xFF;
                pWriting[shift_pWriting+2] = LEN_1_BYTE;
                if (pData[1] ==0)
                    pWriting[shift_pWriting+3] = 0;
                else
                    pWriting[shift_pWriting+3] = 0x08;

                pWriting[shift_pWriting+4] = 0x08;   //bit3
                pWriting_len =  LEN_5_BYTE;
        break;
        case EXTEND_WRITE_CONFIG_BUF_BLE2M_2402_2480:  // 0x40
        offset=EXTEND_CONFIG_BLE2M_2402_2480;
        pWriting_len=0;
        if (pData[1] != 0xFF)
        {
            pWriting[shift_pWriting+pWriting_len+0] =offset & 0xFF ;
            pWriting[shift_pWriting+pWriting_len+1] =(offset>>8) & 0xFF;
            pWriting[shift_pWriting+pWriting_len+2] = LEN_1_BYTE;
            pWriting[shift_pWriting+pWriting_len+3] = pData[1];
            pWriting[shift_pWriting+pWriting_len+4] = 0xFF;
            pWriting_len +=  LEN_5_BYTE;
        }
        if (pData[2] != 0xFF)
        {
            pWriting[shift_pWriting+pWriting_len+0] = (offset+1) & 0xFF ;
            pWriting[shift_pWriting+pWriting_len+1] = ((offset+1)>>8) & 0xFF;
            pWriting[shift_pWriting+pWriting_len+2] = LEN_1_BYTE;
            pWriting[shift_pWriting+pWriting_len+3] = pData[2];
            pWriting[shift_pWriting+pWriting_len+4] = 0xFF;
            pWriting_len +=  LEN_5_BYTE;
        }

        break;
        case EXTEND_WRITE_CONFIG_BUF_LBT_SETTING:      // 0x41
        //ENABLE

        pWriting_len=0;
        if (pData[1] != 0xFF)
        {
            offset= EXTEND_CONFIG_LBT_EN_SETTING;
            pWriting[shift_pWriting+pWriting_len+0] =offset & 0xFF ;
            pWriting[shift_pWriting+pWriting_len+1] =(offset>>8) & 0xFF;
            pWriting[shift_pWriting+pWriting_len+2] = LEN_1_BYTE;
            pWriting[shift_pWriting+pWriting_len+3] = pData[1];  //1 ENABEL 0:DISABLE
            if(pBTInfo->Version > 3)
            {
                pWriting[shift_pWriting+pWriting_len+4] = 0x80;
            }
            else
            {
                pWriting[shift_pWriting+pWriting_len+4] = 0x01;
            }
            pWriting_len += 5;
        }

        //ANT GAIN

       if (pData[2] != 0xFF)
        {
            offset= EXTEND_CONFIG_LBT_ANT_GAIN_SETTING;
            pWriting[shift_pWriting+pWriting_len+0] =offset & 0xFF ;
            pWriting[shift_pWriting+pWriting_len+1] =(offset>>8) & 0xFF;
            pWriting[shift_pWriting+pWriting_len+2] = LEN_1_BYTE;
            pWriting[shift_pWriting+pWriting_len+3] = pData[2];
            pWriting[shift_pWriting+pWriting_len+4] = 0xFF;
            pWriting_len += 5;
        }
        break;
        case EXTEND_WRITE_CONFIG_BUF_PC_SETTING:       // 0X42
        offset=EXTEND_CONFIG_PC_SETTING;
        pWriting[shift_pWriting+0] =offset & 0xFF ;
        pWriting[shift_pWriting+1] =(offset>>8) & 0xFF;
        pWriting[shift_pWriting+2] = LEN_1_BYTE;
        pWriting[shift_pWriting+3] = pData[1];
        pWriting[shift_pWriting+4] = 0xFF;
        pWriting_len =  LEN_5_BYTE;
        break;
        case EXTEND_XTAL_TRACK_TABLE_WRITE_CONFIG :  //           0x56
             Direct_write_to_Device= 1;
        case EXTEND_XTAL_TRACK_TABLE_WRITE_CONFIG_BUF:   //  0x57
                offset = EXTEND_CONFIG_XTAL_TABLE;
                pWriting[shift_pWriting+0] =offset & 0xFF ;
                pWriting[shift_pWriting+1] =(offset>>8) & 0xFF;
                pWriting[shift_pWriting+2] =26;
                memcpy(&pWriting[shift_pWriting+3],&pData[1],26);
                memset(&pWriting[shift_pWriting+3+26],0xFF,26);
                pWriting_len =  3+26+26;
        break;
        case EXTEND_XTAL_TRACK_EN_GET:  //                       0x50
        case EXTEND_XTAL_TRACK_EN_SET:  //                      0x51
        case EXTEND_XTAL_TRACK_TABLE_SET: //                       0x52
        case EXTEND_XTAL_TRACK_TABLE_GET:                   //   0x53
                offset=0;
                if (BTDevice_Extend_XTAL_TRACK_Func(pBtDevice,subindex,&pData[1]) != BT_FUNCTION_SUCCESS)
                {
                    rtn = FUNCTION_ERROR;
                    goto error;
                }
        break;

        //------------------- XTAL TRACKING --------------
    }
       if (offset > 0x00)
    {
           if (Direct_write_to_Device)
            {
                pWriting[0]=pWriting_len &0xFF;
                pWriting[1]=(pWriting_len>>8)&0xFF;
                if (BTDevice_Write_Data_Entry_to_Flash_Config(pBtDevice,pWriting) != BT_FUNCTION_SUCCESS)
                    goto error;
            }
          else
            {
                if(BT_Device_FlashtempBuf_AddEntry(pBtDevice,&pWriting[shift_pWriting],pWriting_len )  != BT_FUNCTION_SUCCESS)
                    goto error;
            }
    }
    rtn= BT_FUNCTION_SUCCESS;
error:

    return rtn;
}

/*-------------------------------------------*
RTL8723D
*---------------------------------------------*/
