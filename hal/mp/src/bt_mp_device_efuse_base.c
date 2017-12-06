#define LOG_TAG "bt_mp_device_efuse_base"

#include "bt_syslog.h"
#include "bluetoothmp.h"
#include "bt_mp_device_efuse_base.h"

enum
{
    TYPE_15K = 0,   // 1.5k
    TYPE_10K,       // 10K
};

#define _DO_WRITE_   0
#define _DO_READ_    1

#define DEFAULT_WINDOWS_SIZE 4


int
BTDevice_Efuse_SetLdo225_10K15K(
        BT_DEVICE *pBtDevice,
        int ReadWriteMode,
        int R0
        )
{
    uint16_t data;

    if (bt_default_GetSysRegMaskBits(pBtDevice, 0x37, 7, 0, &data)!=BT_FUNCTION_SUCCESS)
        goto error;

    // LDO V = 2.25 v
    if (ReadWriteMode == _DO_WRITE_)
    {
        data = (data & 0x83) | 0x80;
    }
    else
    {
        data  &= 0x83;
    }

    if ( R0 == TYPE_15K )
        data = data | 0x74;    // 1.5k
    else
        data = data | 0x70;   // 10k

    if (bt_default_SetSysRegMaskBits(pBtDevice, 0x37, 7, 0, data) !=BT_FUNCTION_SUCCESS)
        goto error;


    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;
}



#define READ_WRITE_EFUSE_REG_MAX_LEN 32
int
BTDevice_Efuse_HCIIO_GetBytes(
        BT_DEVICE *pBtDevice,
        int Bank,
        unsigned int address,
        unsigned char *pDataBytes,
        unsigned int DataBytesLen
        )
{
    uint8_t pData[HCI_CMD_LEN_MAX];
    uint8_t pEvent[HCI_EVT_LEN_MAX];
    uint32_t EvtLen;

    unsigned int length;

    memset(pData, 0, HCI_CMD_LEN_MAX);
    memset(pEvent, 0, HCI_EVT_LEN_MAX);

    pData[0]=Bank;
    pData[1]=address&0xff;
    pData[2]=(address>>8)&0xff;
    pData[3]=DataBytesLen&0xff;
    pData[4]=(DataBytesLen>>8)&0xff;

    if (bt_default_SendHciCommandWithEvent(pBtDevice, 0xFC6C, LEN_5_BYTE, pData, 0x0e, pEvent, &EvtLen)!=BT_FUNCTION_SUCCESS)
        goto error;

    //check event
    if (pEvent[5] != 0)
    {
        goto error;
    }

    length = (pEvent[7]<<8) | pEvent[6];
    if (DataBytesLen != length)
        goto error;

    memcpy(pDataBytes,&pEvent[8],length);

    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;
}



int
BTDevice_Efuse_HCIIO_SetBytes(
        BT_DEVICE *pBtDevice,
        int Bank,
        unsigned int address,
        unsigned char *pDataBytes,
        unsigned int DataBytesLen
        )
{
    uint8_t pData[HCI_CMD_LEN_MAX];
    uint8_t pEvent[HCI_EVT_LEN_MAX];
    unsigned char length;
    uint32_t EvtLen;

    memset(pData, 0, HCI_CMD_LEN_MAX);
    memset(pEvent, 0, HCI_EVT_LEN_MAX);

    pData[0]=Bank;
    pData[1]=address&0xff;
    pData[2]=(address>>8)&0xff;
    pData[3]=DataBytesLen&0xff;
    pData[4]=(DataBytesLen>>8)&0xff;
    memcpy(&pData[5],pDataBytes,DataBytesLen);
    length=LEN_5_BYTE + DataBytesLen;

    if (bt_default_SendHciCommandWithEvent(pBtDevice, 0xFC6B, length, pData, 0x0e, pEvent, &EvtLen)!=BT_FUNCTION_SUCCESS)
        goto error;

    //check event
    if (pEvent[5] != 0)
        goto error;

    length = (pEvent[7]<<8) | pEvent[6];
    if (DataBytesLen != length)
        goto error;


    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;
}

int
BTDevice_Efuse_IO_SetBytes(
        BT_DEVICE *pBtDevice,
        int Bank,
        unsigned int address,
        unsigned char *pDataBytes,
        unsigned int DataBytesLen
        )
{

    uint16_t tmpdata;

    if (bt_default_GetSysRegMaskBits(pBtDevice, 0x35, 7, 0, &tmpdata)!=BT_FUNCTION_SUCCESS)
        goto error;

    tmpdata= (tmpdata&0xFC) | 0x08 |Bank ;

    if (bt_default_SetSysRegMaskBits(pBtDevice, 0x35, 7, 0, tmpdata) !=BT_FUNCTION_SUCCESS)
        goto error;

    if (BTDevice_Efuse_HCIIO_SetBytes(pBtDevice, Bank, address, pDataBytes, DataBytesLen)!=BT_FUNCTION_SUCCESS)
        goto error;

    if (bt_default_GetSysRegMaskBits(pBtDevice, 0x35, 7, 0, &tmpdata)!=BT_FUNCTION_SUCCESS)
        goto error;

    tmpdata = tmpdata & 0xF4;
    tmpdata |= (Bank & 0x0f);

    if (bt_default_SetSysRegMaskBits(pBtDevice, 0x35, 7, 0, tmpdata) !=BT_FUNCTION_SUCCESS)
        goto error;

    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;

}



int
BTDevice_Efuse_IO_GetBytes(
        BT_DEVICE *pBtDevice,
        int Bank,
        unsigned int address,
        unsigned char *pDataBytes,
        unsigned int DataBytesLen
        )
{

    if (BTDevice_Efuse_HCIIO_GetBytes(pBtDevice, Bank, address, pDataBytes, DataBytesLen)!=BT_FUNCTION_SUCCESS)
        goto error;

    bt_default_SetSysRegMaskBits(pBtDevice, 0x35, 3, 0, Bank&0x07);

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}


int
BTDevice_Efuse_GetBytes(
        BT_DEVICE *pBtDevice,
        uint8_t Bank,
        int RegStartAddr,
        uint8_t *pReadingBytes,
        unsigned int ByteNum
        )
{
    unsigned int i;

    unsigned int ReadingByteNum, ReadingByteNumRem;
    unsigned int RegReadingAddr;

    unsigned char pReadingCmpBuf[READ_WRITE_EFUSE_REG_MAX_LEN];

    memset(pReadingCmpBuf, 0xff, READ_WRITE_EFUSE_REG_MAX_LEN);
	//FillMemory(pReadingCmpBuf, READ_WRITE_EFUSE_REG_MAX_LEN, 0xFF);

    for(i = 0; i < ByteNum; i += READ_WRITE_EFUSE_REG_MAX_LEN)
    {
        // Set register reading address.
        RegReadingAddr = RegStartAddr + i;

        // Calculate remainder reading byte number.
        ReadingByteNumRem = ByteNum - i;

        // Determine reading byte number.
        ReadingByteNum = (ReadingByteNumRem > READ_WRITE_EFUSE_REG_MAX_LEN) ? READ_WRITE_EFUSE_REG_MAX_LEN : ReadingByteNumRem;

        BTDevice_Efuse_SetLdo225_10K15K(pBtDevice, _DO_READ_, TYPE_15K);    // 1.5K

        if (BTDevice_Efuse_IO_GetBytes(pBtDevice, Bank, RegReadingAddr, pReadingBytes+i, ReadingByteNum)!=BT_FUNCTION_SUCCESS)
            goto error;

        if( memcmp(pReadingBytes+i, pReadingCmpBuf, ReadingByteNum) == 0 )
		{
			goto exit;
		}

    }

exit:

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
BTDevice_Efuse_SetBytes(
        BT_DEVICE *pBtDevice,
        int Bank,
        int RegStartAddr,
        unsigned char *pWritingBytes,
        unsigned int ByteNum
        )
{
    unsigned int i, j;

    unsigned int WritingByteNum, WritingByteNumRem;
    unsigned int RegWritingAddr;

    unsigned char pReadingBytes[READ_WRITE_EFUSE_REG_MAX_LEN];
    unsigned char times;

    for(i = 0; i < ByteNum; i += READ_WRITE_EFUSE_REG_MAX_LEN)
    {

        times =0;

        // Set register reading address.
        RegWritingAddr = RegStartAddr + i;

        // Calculate remainder reading byte number.
        WritingByteNumRem = ByteNum - i;

        // Determine reading byte number.
        WritingByteNum = (WritingByteNumRem > READ_WRITE_EFUSE_REG_MAX_LEN) ? READ_WRITE_EFUSE_REG_MAX_LEN : WritingByteNumRem;

retry:
        times++;
        if (times == 1 )
        {
            //set 1.5K
            if (BTDevice_Efuse_SetLdo225_10K15K(pBtDevice, _DO_WRITE_, TYPE_15K))
                goto error;
        }
        else if (times ==2)
        {
            //set 10K
            if (BTDevice_Efuse_SetLdo225_10K15K(pBtDevice, _DO_WRITE_, TYPE_10K))
                goto error;
        }
        else
        {
            goto error;
        }

        if (BTDevice_Efuse_IO_SetBytes(pBtDevice, Bank, RegWritingAddr, pWritingBytes+i, WritingByteNum)!=BT_FUNCTION_SUCCESS)
        {
            goto retry;
        }

        if (BTDevice_Efuse_IO_GetBytes(pBtDevice, Bank, RegWritingAddr, pReadingBytes, WritingByteNum)!=BT_FUNCTION_SUCCESS)
            goto error;

        for (j=0; j<WritingByteNum; j++)
        {
            if (pReadingBytes[j] != pWritingBytes[i+j])
            {
                goto retry;
            }
        }

    }

    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;

}



int
BTDevice_Efuse_Log2EntryMap(
        EFUSE_UNIT *pEfuse,
        uint16_t StartLogAddr,
        uint8_t  *pWritingEntry,
        uint16_t *Len
        )
{
    uint8_t BaseAddress, BaseAddress_x, BaseAddress_y, BaseAddress_A;
    uint8_t Header1, Header2, WordSelect;
    uint8_t tmp[LEN_8_BYTE];
    uint16_t i, j, count;


    count = 0;
    WordSelect = 0xff;
    *Len = 0;

    memset(tmp, 0, sizeof(tmp));

    for (j = 0; j < LEN_4_BYTE; j++)
    {
        if( (pEfuse->EfuseType == BT_EFUSE) && ( ( (StartLogAddr == 0x0038) && ( (j ==2) ||(j ==3) ) ) || ( (StartLogAddr == 0x0040) && (j==0) )  )
            && (pEfuse->pEfuseLogMem[StartLogAddr+j*2].NewValue==0xff)  && (pEfuse->pEfuseLogMem[StartLogAddr+j*2+1].NewValue==0xff) )
        {
            goto create_efuse_entry;
        }
        if ((pEfuse->pEfuseLogMem[StartLogAddr+j*2].NewValue == 0xff) && (pEfuse->pEfuseLogMem[StartLogAddr+j*2+1].NewValue == 0xff))
        {
            pEfuse->pEfuseLogMem[StartLogAddr+j*2].NewValue = pEfuse->pEfuseLogMem[StartLogAddr+j*2].OldValue;
            pEfuse->pEfuseLogMem[StartLogAddr+j*2+1].NewValue = pEfuse->pEfuseLogMem[StartLogAddr+j*2+1].OldValue;
            continue;
        }

        if ((pEfuse->pEfuseLogMem[StartLogAddr+j*2].OldValue != pEfuse->pEfuseLogMem[StartLogAddr+j*2].NewValue) ||
            (pEfuse->pEfuseLogMem[StartLogAddr+j*2+1].OldValue != pEfuse->pEfuseLogMem[StartLogAddr+j*2+1].NewValue))
        {
create_efuse_entry:
            WordSelect=WordSelect & ~(0x01<<(j));
            tmp[count] = pEfuse->pEfuseLogMem[StartLogAddr+j*2].NewValue;
            tmp[count+1] = pEfuse->pEfuseLogMem[StartLogAddr+j*2+1].NewValue;
            count += 2;
        }
    }

    if ((WordSelect & 0x0f) == 0x0f)
    {
        return BT_FUNCTION_SUCCESS;
    }

    if (StartLogAddr < 128)
    {
        // one-byte mode
        BaseAddress = StartLogAddr / 8;
        Header1 = (BaseAddress&0x0f) << 4;
        Header1 |= (WordSelect&0x0f);
        pWritingEntry[0] = Header1;
        for (i = 0; i < count; i++)
        {
            pWritingEntry[1+i] = tmp[i];
        }
        *Len = count + 1;
    }
    else
    {
        // two-byte mode
        BaseAddress_A = (StartLogAddr/8) - 16;
        BaseAddress_x = (BaseAddress_A%8) * 2;
        BaseAddress_y = (BaseAddress_A/8) + 2;

        Header1 = (BaseAddress_x&0x0F)<<4 | 0x0F;
        Header2 = (BaseAddress_y&0x0F)<<4;
        Header2 |= (WordSelect&0x0f);
        pWritingEntry[0] = Header1;
        pWritingEntry[1] = Header2;
        for (i = 0; i < count; i++)
        {
            pWritingEntry[2+i] = tmp[i];
        }
        *Len = count + 2;
    }

    return BT_FUNCTION_SUCCESS;
}

int
BuildEfuseLogicUnit(
        BT_DEVICE *pBtDevice,
        EFUSE_UNIT **ppEfuseModule,
        EFUSE_UNIT *pEfuseModuleMemory,
        uint16_t EfuseType,
        uint16_t EfuseLogSize,
        uint16_t EfusePhySize,
        uint8_t StartBank,
        uint8_t BankNum
        )
{
    EFUSE_UNIT *pEfuse;
    uint16_t i;
    uint8_t Bank;

    *ppEfuseModule = pEfuseModuleMemory;

    pEfuse = *ppEfuseModule;
    pEfuse->pBtDevice = pBtDevice;

    if (EfuseLogSize > MAX_EFUSE_LOG_LEN)
        goto error;

    if (EfusePhySize > MAX_EFUSE_PHY_LEN)
        goto error;

    pEfuse->EfuseType = EfuseType;
    pEfuse->EfuseLogSize = EfuseLogSize;
    pEfuse->EfusePhySize = EfusePhySize;
    pEfuse->StartBank = StartBank;
    pEfuse->BankNum = BankNum;
    pEfuse->CurrBank = pEfuse->StartBank;

    for (i = 0; i < pEfuse->EfuseLogSize; i++)
    {
        pEfuse->pEfuseLogMem[i].NewValue = 0xff;
        pEfuse->pEfuseLogMem[i].OldValue = 0xff;
    }

    for (i = 0; i < MAX_EFUSE_BANK_NUM; i++)
        pEfuse->pEfusePhyDataLen[i] = 0;

    for( Bank=StartBank; Bank<StartBank+BankNum; Bank++)
	{
        memset(pEfuse->pEfusePhyMem+Bank*MAX_EFUSE_PHY_LEN, 0xFF, MAX_EFUSE_PHY_LEN);
		//FillMemory(pEfuse->pEfusePhyMem+Bank*MAX_EFUSE_PHY_LEN, MAX_EFUSE_PHY_LEN, 0xFF);
	}
    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}



int
BTDevice_Efuse_LoadPhyMem(
        EFUSE_UNIT *pEfuse
        )
{
    uint8_t Bank;
    uint8_t StartBank;
    uint8_t BankNum;

    StartBank = pEfuse->StartBank;
    BankNum = pEfuse->BankNum;

    for (Bank = StartBank; Bank < StartBank + BankNum; Bank++)
    {
        if (BTDevice_Efuse_GetBytes(
                    pEfuse->pBtDevice,
                    Bank,
                    0,
                    pEfuse->pEfusePhyMem+Bank*MAX_EFUSE_PHY_LEN,
                    pEfuse->EfusePhySize) != BT_FUNCTION_SUCCESS)
            goto error;
    }

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}


int
BTDevice_Efuse_Log2PhyMap(
        EFUSE_UNIT *pEfuse
        )
{
    uint16_t i, j;
    uint8_t Bank;
    uint8_t StartBank;
    uint8_t BankNum;
    uint8_t pWritingEntry[LEN_10_BYTE];
    uint16_t WritingLen;


    StartBank = pEfuse->StartBank;
    BankNum = pEfuse->BankNum;

    for (i = 0; i < pEfuse->EfuseLogSize; i += LEN_8_BYTE)
    {
        if (BTDevice_Efuse_Log2EntryMap(pEfuse, i, pWritingEntry, &WritingLen) != BT_FUNCTION_SUCCESS)
            goto error;

        if (WritingLen > 0)
        {
            Bank = pEfuse->CurrBank;
re_check:
            if ((pEfuse->pEfusePhyDataLen[Bank] + WritingLen) > (pEfuse->EfusePhySize - DUMMY_EFUSE_LEN))
            {
                Bank++;

                if (Bank >= StartBank + BankNum)
                    goto error;
                else
                    goto re_check;
            }

            if (BTDevice_Efuse_SetBytes(pEfuse->pBtDevice, Bank, pEfuse->pEfusePhyDataLen[Bank], pWritingEntry, WritingLen) != BT_FUNCTION_SUCCESS)
                goto error;

            for (j = i; j < i + LEN_8_BYTE; j++)
            {
                pEfuse->pEfuseLogMem[j].OldValue = pEfuse->pEfuseLogMem[j].NewValue;
            }

            memcpy(pEfuse->pEfusePhyMem+pEfuse->pEfusePhyDataLen[Bank], pWritingEntry, WritingLen);

            pEfuse->pEfusePhyDataLen[Bank] += WritingLen;
            pEfuse->CurrBank = Bank;
        }
    }

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
BTDevice_Efuse_Phy2LogMap(
        EFUSE_UNIT *pEfuse
        )
{
    uint16_t i, j;
    uint8_t b;
    uint8_t Header1;
    uint8_t Header2;
    uint8_t BaseAddress_x, BaseAddress_y;
    uint16_t BaseAddress;
    uint8_t WordSelect;

    for (i = 0; i < pEfuse->EfuseLogSize; i++)
    {
        pEfuse->pEfuseLogMem[i].OldValue = 0xff;
        pEfuse->pEfuseLogMem[i].NewValue = 0xff;
    }

    for (b = pEfuse->StartBank; b < pEfuse->StartBank + pEfuse->BankNum; b++)
    {
        i = 0;

        while (i < pEfuse->EfusePhySize - DUMMY_EFUSE_LEN)
        {
            if ((pEfuse->pEfusePhyMem[b*MAX_EFUSE_PHY_LEN+i+0] == 0xff) &&
                (pEfuse->pEfusePhyMem[b*MAX_EFUSE_PHY_LEN+i+1] == 0xff) &&
                (pEfuse->pEfusePhyMem[b*MAX_EFUSE_PHY_LEN+i+2] == 0xff) &&
                (pEfuse->pEfusePhyMem[b*MAX_EFUSE_PHY_LEN+i+3] == 0xff) &&
                (pEfuse->pEfusePhyMem[b*MAX_EFUSE_PHY_LEN+i+4] == 0xff) &&
                (pEfuse->pEfusePhyMem[b*MAX_EFUSE_PHY_LEN+i+5] == 0xff) &&
                (pEfuse->pEfusePhyMem[b*MAX_EFUSE_PHY_LEN+i+6] == 0xff) &&
                (pEfuse->pEfusePhyMem[b*MAX_EFUSE_PHY_LEN+i+7] == 0xff) )
                {
                pEfuse->pEfusePhyDataLen[b] = i;

                if (pEfuse->pEfusePhyDataLen[b])
                    pEfuse->CurrBank = b;
                break;
            }

            Header1 = pEfuse->pEfusePhyMem[b*MAX_EFUSE_PHY_LEN + i++];

            if ((Header1&0x0f) != 0x0f)
            {
                // one-byte mode
                BaseAddress = ((Header1>>4)&0x0f) * 8;
                WordSelect =  Header1 & 0x0f;
            }
            else
            {
                // two-byte mode
                Header2 = pEfuse->pEfusePhyMem[b*MAX_EFUSE_PHY_LEN + i++];

                BaseAddress_x = (Header1>>4)&0x0f;
                BaseAddress_y = (Header2>>4)&0x0f;
                BaseAddress = ((BaseAddress_x/2) + ((BaseAddress_y-2)*8) + 16) * 8;
                WordSelect = Header2 & 0x0f;
            }

            for (j = 0; j < LEN_4_BYTE; j++)
            {
                if (((WordSelect>>j) & 0x01) == 0x00)
                {
                    pEfuse->pEfuseLogMem[BaseAddress + (j*2)].OldValue = pEfuse->pEfusePhyMem[b*MAX_EFUSE_PHY_LEN + i++];
                    pEfuse->pEfuseLogMem[BaseAddress + (j*2)+1].OldValue = pEfuse->pEfusePhyMem[b*MAX_EFUSE_PHY_LEN + i++];
                    pEfuse->pEfuseLogMem[BaseAddress + (j*2)].NewValue = pEfuse->pEfuseLogMem[BaseAddress + (j*2)].OldValue;
                    pEfuse->pEfuseLogMem[BaseAddress + (j*2)+1].NewValue = pEfuse->pEfuseLogMem[BaseAddress + (j*2)+1].OldValue;
                }
            }
        }
    }

    return BT_FUNCTION_SUCCESS;
}

int
BTDevice_Efuse_UpdateLogMem(
        EFUSE_UNIT *pEfuse,
        uint16_t Addr,
        uint8_t Len,
        uint8_t *pBuf
        )
{
    uint8_t i;

    for (i = 0; i < Len; i++)
        pEfuse->pEfuseLogMem[Addr+i].NewValue = *(pBuf + i);

    return BT_FUNCTION_SUCCESS;
}
