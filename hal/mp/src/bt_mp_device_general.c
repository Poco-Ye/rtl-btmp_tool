#define LOG_TAG "bt_mp_device_general"

#include "bluetoothmp.h"
#include "bt_mp_device_general.h"

//------------------------------------------------------------------------------------------------------------------
//Device Level::member Funcion Base Function for
//
//-----------------------------------------------------------------------------------------------------


int BTDevice_SetPowerGainIndex(BT_DEVICE *pBtDevice, int Index)
{
    uint8_t PowerGainValue =0;
    int rtn=BT_FUNCTION_SUCCESS;
    if (Index > MAX_TXGAIN_TABLE_SIZE)
    {
        rtn=FUNCTION_PARAMETER_ERROR;
        goto exit;
    }

    PowerGainValue =pBtDevice->TXGainTable[Index - 1];
    /* set rf standby mode */
    rtn=pBtDevice->SetRfRegMaskBits(pBtDevice,0x00,15,0,0x1000);
    if (rtn != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    rtn=BTDevice_SetPowerGain(pBtDevice,PowerGainValue);

exit:
    return rtn;
}

int BTDevice_SetTestMode(BT_DEVICE *pBtDevice,BT_TEST_MODE TestMode)
{
    int rtn=BT_FUNCTION_SUCCESS;

    switch (TestMode) {
    case BT_DUT_MODE:
        if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x2e,11,8,0x0E) !=BT_FUNCTION_SUCCESS)
        {
            rtn=FUNCTION_ERROR;
            goto exit;
        }
        if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x3c,12,12,0x1) !=BT_FUNCTION_SUCCESS)
        {
            rtn=FUNCTION_ERROR;
            goto exit;
        }
        break;

    case BT_PSEUDO_MODE:
        /* disable modem fix tx */
        if (pBtDevice->SetMdRegMaskBits(pBtDevice,0x3c,12,12,0x0) !=BT_FUNCTION_SUCCESS)
        {
            rtn=FUNCTION_ERROR;
            goto exit;
        }
        /* enable pesudo outter mode */
        rtn=pBtDevice->SetMdRegMaskBits(pBtDevice,0x2e,8,8,0x1);
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

int BTDevice_SetLETxChannel(BT_DEVICE *pBtDevice, uint8_t ChannelNumber)
{
    int rtn=BT_FUNCTION_SUCCESS;
    /* set rf standby mode */
    ChannelNumber=ChannelNumber*2;
    if (ChannelNumber >=40)
        return FUNCTION_PARAMETER_INVALID_CHANNEL;

    rtn=pBtDevice->SetRfRegMaskBits(pBtDevice,0x00,15,0,0x1000);
    if (rtn != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    rtn=pBtDevice->SetRfRegMaskBits(pBtDevice,0x3F,15,0,ChannelNumber&0x7F);

exit:
    return rtn;
}

int BTDevice_SetTxChannel(BT_DEVICE *pBtDevice, uint8_t ChannelNumber)
{
    int rtn=BT_FUNCTION_SUCCESS;
    /* set rf standby mode */
    if (ChannelNumber >=79)
        return FUNCTION_PARAMETER_INVALID_CHANNEL;

    rtn=pBtDevice->SetRfRegMaskBits(pBtDevice,0x00,15,0,0x1000);
    if (rtn != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    rtn=pBtDevice->SetRfRegMaskBits(pBtDevice,0x3F,15,0,ChannelNumber&0x7F);

exit:
    return rtn;
}

int BTDevice_SetRxChannel(BT_DEVICE *pBtDevice, uint8_t ChannelNumber)
{
    int rtn=BT_FUNCTION_SUCCESS;
    if (ChannelNumber >=79)
        return FUNCTION_PARAMETER_INVALID_CHANNEL;
    /* set rf standby mode */
    rtn=pBtDevice->SetRfRegMaskBits(pBtDevice,0x00,15,0,0x1000);
    if (rtn != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    /* set rf channel */
    rtn=pBtDevice->SetRfRegMaskBits(pBtDevice,0x3F,15,0,0x80 | (ChannelNumber&0x7F));

exit:
    return rtn;
}

int BTDevice_SetPowerGain(BT_DEVICE *pBtDevice, uint8_t PowerGainValue)
{
    int rtn=BT_FUNCTION_SUCCESS;
    /* set rf standby mode */
    rtn=pBtDevice->SetRfRegMaskBits(pBtDevice,0x00,15,0,0x1000);
    if (rtn != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    rtn=pBtDevice->SetRfRegMaskBits(pBtDevice,0x02,15,8,PowerGainValue);

exit:
    return rtn;
}

int BTDevice_SetPowerDac(BT_DEVICE *pBtDevice, uint8_t DacValue)
{
    int rtn=BT_FUNCTION_SUCCESS;

    rtn=pBtDevice->SetMdRegMaskBits(pBtDevice,0x38,7,3,DacValue);

    return rtn;
}

/**
 *  BD_ADDRESS[0X3C]: Phyical map[1][000][Len=5][73,34,12,78,56,]-->[n=0,03c-34 12][n=1,03e-78 56]
 *  [161]Phyical map[1][00c][Len=4][8f,5e,01,22,]-->[n=0,160-01 22]
 *  [181]Phyical map[1][010][Len=4][0f,6e,44,33,]-->[n=0,180-44 33]
 */
#define MAX_BANK_SIZE 1024
int BTDevice_PGEfuseRawData(
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
        rtn = BTDevice_SpecialFunction_Efuse_GetPGEfuseLength(pBtDevice,0,&Bank_Use_PGlength);
        if (rtn != BT_FUNCTION_SUCCESS)
        {
            goto exit;
        }
        Bank_Use=0;
    }
    else // BT_EFUSE
    {
        rtn = BTDevice_SpecialFunction_Efuse_GetPGEfuseLength(pBtDevice,2,&Bank2_PGlength);
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
            rtn = BTDevice_SpecialFunction_Efuse_GetPGEfuseLength(pBtDevice,1,&Bank1_PGlength);
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
    rtn = BTDevice_SpecialFunction_Efuse_WriteBytes(pBtDevice,Bank_Use,Bank_Use_PGlength,PGDataLength,PGData);

exit:
    return rtn;
}

int BTDevice_SpecialFunction_Efuse_ReadBytes(BT_DEVICE *pBtDevice, uint8_t Bank, uint16_t StartAddress, uint16_t ReadLen, uint8_t *pReadingData)
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
    rtn = pBtDevice->SetSysRegMaskBits(pBtDevice,0x35,3,3,0x0);
    if (rtn != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    pPayLoad[0]=Bank; //Bank
    pPayLoad[1]=StartAddress&0xff;
    pPayLoad[2]=(StartAddress>>8)&0xff;
    pPayLoad[3]=ReadLen&0xff;
    pPayLoad[4]=(ReadLen>>8)&0xff;

    rtn = pBtDevice->SendHciCommandWithEvent(pBtDevice,OpCode,5,pPayLoad,0x0e,pEvent,&EvtLen);
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

int BTDevice_SpecialFunction_Efuse_WriteBytes(BT_DEVICE *pBtDevice, uint8_t Bank, uint16_t StartAddress, uint16_t WriteLen, uint8_t *pWriteData)
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
    rtn=pBtDevice->SetSysRegMaskBits(pBtDevice,0x35,3,3,0x1);
    if (rtn != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    pPayLoad[0]=2; //Bank
    pPayLoad[1]=0xff;
    pPayLoad[2]=0x01;
    pPayLoad[3]=0x01;
    pPayLoad[4]=0x00;
    pPayLoad[5]=0x85;
    length=6;

    rtn = pBtDevice->SendHciCommandWithEvent(pBtDevice,OpCode,1,pPayLoad,0x0e,pEvent,&EvtLen);

    rtn = pBtDevice->SetSysRegMaskBits(pBtDevice,0x35,3,3,0x1);
    if (rtn != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }

    pPayLoad[0]=Bank; //Bank
    pPayLoad[1]=StartAddress&0xff;
    pPayLoad[2]=(StartAddress>>8)&0xff;
    pPayLoad[3]=WriteLen&0xff;
    pPayLoad[4]=(WriteLen>>8)&0xff;
    memcpy(&pPayLoad[5],pWriteData,WriteLen);
    length = 5 + WriteLen;

    rtn = pBtDevice->SendHciCommandWithEvent(pBtDevice,OpCode,length,pPayLoad,0x0e,pEvent,&EvtLen);
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
    // Modify
    //
    //rtn=pBtDevice->SetSysRegMaskBits(pBtDevice,0x35,1,0,Bank);
    //if (rtn != BT_FUNCTION_SUCCESS)
    //{
    //    goto exit;
    //}
    //

exit:
    return rtn;
}

int BTDevice_SpecialFunction_Efuse_GetPGEfuseLength(BT_DEVICE *pBtDevice, uint8_t Bank, unsigned int *PGLength)
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
        rtn = BTDevice_SpecialFunction_Efuse_ReadBytes(pBtDevice,Bank,address,2,pData);
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
