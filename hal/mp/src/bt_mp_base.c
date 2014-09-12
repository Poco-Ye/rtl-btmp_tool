#define LOG_TAG "bt_mp_base"

#include <stdio.h>

#include "bt_syslog.h"
#include "bluetoothmp.h"
#include "bt_mp_base.h"

#define EVT_HCI_VERSION         6
#define EVT_HCI_SUBVERSION      7
#define EVT_HCI_LMPVERSION      9
#define EVT_HCI_LMPSUBVERSION   12
#define EVT_CHIP_ECO_VERSION    6

//-----------------------------------------------------------------------------------------------------
void
BTHCI_EvtReport(
        uint8_t *pEvtCode,
        uint32_t EvtCodeLen
        )
{
#if 0
    unsigned long i;

    SYSLOGI("<-rev hci event : ");
    for( i = 0; i < EvtCodeLen; i++)
    {
        SYSLOGI("0x%x ", pEvtCode[i]);
    }
    SYSLOGI("\n");
#endif
}

int
bt_Send(
        BT_DEVICE *pBt,
        uint8_t PktType,
        uint8_t *pWritingBuf,
        uint32_t Len
       )
{
    BASE_INTERFACE_MODULE *pBaseInterface;

    pBaseInterface = pBt->pBaseInterface;

    if (pBaseInterface->Send(pBaseInterface, pWritingBuf, Len) != BT_FUNCTION_SUCCESS)
        goto error;

    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;
}

int
bt_Recv(
        BT_DEVICE *pBt,
        uint8_t PktType,
        uint8_t *pReadingBuf,
        uint32_t *pLen
       )
{
    BASE_INTERFACE_MODULE *pBaseInterface;
    uint8_t ucRecvBuf[MAX_HCI_EVENT_BUF_SIZ];
    uint32_t Retlen = 0;

    pBaseInterface = pBt->pBaseInterface;

    if (pBaseInterface->Recv(pBaseInterface, ucRecvBuf, MAX_HCI_EVENT_BUF_SIZ, &Retlen) != BT_FUNCTION_SUCCESS)
        goto error;

    switch (PktType) {
    case HCIIO_BTEVT:
        if (Retlen <= 0x02) //Event Header
        {
            goto error;
        }

        memcpy(pReadingBuf, ucRecvBuf, Retlen);
        *pLen = Retlen;
        break;

    case HCIIO_BTACLIN:
        if (Retlen <= 0x04) //ACL Header
        {
            goto error;
        }

        memcpy(pReadingBuf, ucRecvBuf, Retlen);
        *pLen = Retlen;
        break;


    case HCIIO_BTSCOIN:
        if (Retlen <= 0x03) //SCO Header
        {
            goto error;
        }

        memcpy(pReadingBuf, ucRecvBuf, Retlen);
        *pLen = Retlen;
        break;

    case HCIIO_BTCMD:
    case HCIIO_BTACLOUT:
    case HCIIO_BTSCOOUT:
    default:
        goto error;
    }

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
bt_uart_Send(
        BT_DEVICE *pBt,
        uint8_t PktType,
        uint8_t *pWritingBuf,
        uint32_t Len
        )
{
    BASE_INTERFACE_MODULE *pBaseInterface;
    uint8_t ucWriteBuf[HCI_CMD_LEN_MAX];

    pBaseInterface = pBt->pBaseInterface;

    switch (PktType) {
    case HCIIO_BTCMD:
        *ucWriteBuf = IF_UART_CMD;
        memcpy((ucWriteBuf + 0x01), pWritingBuf, Len);
        Len++;
        break;

    case HCIIO_BTACLOUT:
        *ucWriteBuf = IF_UART_ACL;
        memcpy((ucWriteBuf + 0x01), pWritingBuf, Len);
        Len++;
        break;

    case HCIIO_BTSCOOUT:
        *ucWriteBuf = IF_UART_SCO;
        memcpy((ucWriteBuf + 0x01), pWritingBuf, Len);
        Len++;
        break;

    case HCIIO_BTEVT:
    case HCIIO_BTACLIN:
    case HCIIO_BTSCOIN:
    default:
        goto error;
    }


    if(pBaseInterface->Send(pBaseInterface, ucWriteBuf, Len) != BT_FUNCTION_SUCCESS)
        goto error;

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
bt_uart_Recv(
        BT_DEVICE *pBt,
        uint8_t PktType,
        uint8_t *pReadingBuf,
        uint32_t *pLen
        )
{
    BASE_INTERFACE_MODULE *pBaseInterface;
    uint8_t ucRecvBuf[HCI_BUF_LEN_MAX];
    uint32_t Retlen = 0;

    pBaseInterface = pBt->pBaseInterface;

    if (pBaseInterface->Recv(pBaseInterface, ucRecvBuf, HCI_BUF_LEN_MAX, &Retlen) != BT_FUNCTION_SUCCESS)
        goto error;

    switch (PktType) {
    case HCIIO_BTEVT:
        if (Retlen <= 0x03) //PKT Indicator + Event Header
        {
            goto error;
        }

        if (ucRecvBuf[0] != IF_UART_EVT) //Check PKT Indicator
        {
            goto error;
        }
        --Retlen;
        memcpy(pReadingBuf, (ucRecvBuf + 1), Retlen);
        *pLen = Retlen;
        break;

    case HCIIO_BTACLIN:
        if (Retlen <= 0x05) //PKT Indicator + ACL Header
        {
            goto error;
        }

        if (ucRecvBuf[0] != IF_UART_ACL) //Check PKT Indicator
        {
            goto error;
        }
        --Retlen;
        memcpy(pReadingBuf, (ucRecvBuf + 1), Retlen);
        *pLen = Retlen;
        break;

    case HCIIO_BTSCOIN:
        if (Retlen <= 0x04) //PKT Indicator + SCO Header
        {
            goto error;
        }

        if (ucRecvBuf[0] != IF_UART_SCO) //Check PKT Indicator
        {
            goto error;
        }
        --Retlen;
        memcpy(pReadingBuf, (ucRecvBuf + 1), Retlen);
        *pLen = Retlen;
        break;

    case HCIIO_BTCMD:
    case HCIIO_BTACLOUT:
    case HCIIO_BTSCOOUT:
    default:
        goto error;
    }

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
bt_default_SendHCICmd(
        BT_DEVICE *pBt,
        uint8_t PktType,
        uint8_t *pWritingBuf,
        uint32_t Len
        )
{
    int rtn = BT_FUNCTION_SUCCESS;

    switch (pBt->InterfaceType) {
    case TYPE_USB:
    case TYPE_FILTER_UART:
    case TYPE_ADB_UART:
    case TYPE_ADB_USB:
    default:
        rtn = bt_Send(pBt, PktType, pWritingBuf, Len);
        break;

    case TYPE_UART:
        rtn = bt_uart_Send(pBt, PktType, pWritingBuf, Len);
        break;
    }

    return rtn;
}

int
bt_default_RecvHCIEvent(
        BT_DEVICE *pBt,
        uint8_t PktType,
        uint8_t *pReadingBuf,
        uint32_t *pLen
        )
{
    memset(pReadingBuf, 0, sizeof(uint8_t)*MAX_HCI_EVENT_BUF_SIZ);

    switch (pBt->InterfaceType) {
    case TYPE_USB:
    case TYPE_FILTER_UART:
    case TYPE_ADB_USB:
    case TYPE_ADB_UART:
    default:
        if (bt_Recv(pBt, PktType, pReadingBuf, pLen) != BT_FUNCTION_SUCCESS)
            goto error;

        break;

    case TYPE_UART:
        if (bt_uart_Recv(pBt, PktType, pReadingBuf, pLen) != BT_FUNCTION_SUCCESS)
            goto error;
        break;
    }

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
bt_default_SetBytes(
        BT_DEVICE *pBt,
        uint8_t Addr,
        uint32_t WritingValue,
        uint8_t *pEvtCode,
        uint32_t *pEvtCodeLen
        )
{
    int len;
    uint8_t pWritingBuf[HCI_CMD_LEN_MAX];
    uint8_t pEvtBuf[HCI_EVT_LEN_MAX];
    uint16_t OpCode;
    uint32_t Retlen;

    OpCode = 0xFD4A;
    len = 0x07; //PKT CMD Header(3 bytes) + PKT CMD(4 bytes)
    pWritingBuf[0x00] = OpCode & 0xff;
    pWritingBuf[0x01] = (OpCode >> 0x08) & 0xff;
    pWritingBuf[0x02] = 0x04;
    pWritingBuf[0x03] = Addr;
    pWritingBuf[0x04] = (uint8_t)(WritingValue&0xff);
    pWritingBuf[0x05] = (uint8_t)((WritingValue>>8)&0xff);
    pWritingBuf[0x06] = 0x00;

    if (bt_default_SendHCICmd(pBt, HCIIO_BTCMD, pWritingBuf, len) != BT_FUNCTION_SUCCESS)
        goto error;

    if (bt_default_RecvHCIEvent(pBt, HCIIO_BTEVT, pEvtBuf, &Retlen) != BT_FUNCTION_SUCCESS)
        goto error;

    if (Retlen <= 0) {
        goto error;
    }

    if ((*pEvtBuf != 0x0E) || (pEvtBuf[EVT_STATUS] != 0x00)) {
        goto error;
    }

    memcpy(pEvtCode, pEvtBuf, Retlen);
    *pEvtCodeLen = Retlen;

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
bt_default_GetBytes(
        BT_DEVICE *pBt,
        uint8_t Addr,
        uint32_t *pReadingValue,
        uint8_t *pEvtCode,
        uint32_t *pEvtCodeLen
        )
{
    int len = 0;
    uint8_t pWritingBuf[HCI_CMD_LEN_MAX];
    uint8_t pEvtBuf[HCI_EVT_LEN_MAX];
    uint16_t OpCode;
    uint32_t Retlen;

    *pReadingValue = 0x00;

    OpCode = 0xFD49;
    len = 0x04; //PKT CMD Header(3 bytes) + PKT CMD Param(1 bytes)
    pWritingBuf[0x00] = OpCode & 0xff;
    pWritingBuf[0x01] = (OpCode >> 0x08) & 0xff;
    pWritingBuf[0x02] = 0x01;
    pWritingBuf[0x03] = Addr;

    if (bt_default_SendHCICmd(pBt, HCIIO_BTCMD, pWritingBuf, len) != BT_FUNCTION_SUCCESS)
        goto error;

    if (bt_default_RecvHCIEvent(pBt, HCIIO_BTEVT, pEvtBuf, &Retlen) != BT_FUNCTION_SUCCESS)
        goto error;

    if (Retlen == 0)
        goto error;

    if ((*pEvtBuf != 0x0E) || (pEvtBuf[EVT_STATUS] != 0x00)) {
        goto error;
    } else {
        *pReadingValue = ((pEvtBuf[EVT_BYTE1]<<8) + pEvtBuf[EVT_BYTE0]);
    }

    memcpy(pEvtCode, pEvtBuf, Retlen);
    *pEvtCodeLen = Retlen;

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
bt_default_SetRFRegMaskBits(
        BT_DEVICE *pBt,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        )
{
    uint8_t i;
    uint32_t ReadingValue;
    uint32_t WritingValue;
    uint8_t RegAddr;
    uint8_t pEvtBuf[HCI_EVT_LEN_MAX];
    uint32_t len;
    uint32_t Mask;
    uint8_t Shift;

    RegAddr = Addr & 0x3f;

    // Generate mask and shift according to MSB and LSB.
    Mask = 0;

    for (i = Lsb; i <= Msb; i++)
        Mask |= 0x1 << i;

    Shift = Lsb;

    if (Mask != 0xffff) {
        if (bt_default_GetBytes(pBt, RegAddr, &ReadingValue, pEvtBuf, &len)) {
            goto error;
        }
        BTHCI_EvtReport(pEvtBuf, len);

        WritingValue = ((ReadingValue & (~Mask)) | (UserValue << Shift));

        if (bt_default_SetBytes(pBt, RegAddr, WritingValue, pEvtBuf, &len)) {
            goto error;
        }
    } else {
        if (bt_default_SetBytes(pBt, RegAddr, UserValue, pEvtBuf, &len)) {
            goto error;
        }
    }
    BTHCI_EvtReport(pEvtBuf, len);
#ifdef DBG_REG_SETTING
    {
        unsigned long rUserValue =0;
        bt_default_GetRFRegMaskBits(
                pBt,
                Addr,
                Msb,
                Lsb,
                &rUserValue
                );
        SYSLOGI("[RF][Reg=0x%.2x][%2d:%2d][Data=0x%.2x <-- 0x%.2x]",Addr,Msb,Lsb,rUserValue,UserValue);

    }
#endif

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
bt_default_GetRFRegMaskBits(
        BT_DEVICE *pBt,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        )
{
    uint8_t i;
    uint32_t ReadingValue;
    uint8_t RegAddr;
    uint8_t pEvtBuf[HCI_EVT_LEN_MAX];
    uint32_t len;
    uint32_t Mask;
    uint8_t Shift;


    RegAddr = Addr & 0x3f;

    // Generate mask and shift according to MSB and LSB.
    Mask = 0;

    for (i = Lsb; i <= Msb; i++)
        Mask |= 0x1 << i;

    Shift = Lsb;

    if (bt_default_GetBytes(pBt, RegAddr, &ReadingValue, pEvtBuf, &len)) {
        goto error;
    }

    BTHCI_EvtReport(pEvtBuf, len);

    *pUserValue = (ReadingValue & Mask) >> Shift;

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
bt_default_SetMDRegMaskBits(
        BT_DEVICE *pBtDevice,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        )
{
    uint8_t i;
    uint32_t ReadingValue;
    uint32_t WritingValue;
    uint8_t RegAddr;
    uint8_t pEvtBuf[HCI_EVT_LEN_MAX];
    uint32_t len;
    uint32_t Mask;
    uint8_t Shift;

    RegAddr = ((Addr & 0x7f) >> 1) + 0x80;

    // Generate mask and shift according to MSB and LSB.
    Mask = 0;

    for (i = Lsb; i <= Msb; i++)
        Mask |= 0x1 << i;

    Shift = Lsb;

    if (Mask != 0xffff) {
        if (bt_default_GetBytes(pBtDevice, RegAddr, &ReadingValue, pEvtBuf, &len)) {
            goto error;
        }
        BTHCI_EvtReport(pEvtBuf, len);

        WritingValue = ((ReadingValue & (~Mask)) | (UserValue << Shift));

        if (bt_default_SetBytes(pBtDevice, RegAddr, WritingValue, pEvtBuf, &len)) {
            goto error;
        }
    } else {
        if (bt_default_SetBytes(pBtDevice, RegAddr, UserValue, pEvtBuf, &len)) {
            goto error;
        }
    }
    BTHCI_EvtReport(pEvtBuf, len);

#ifdef DBG_REG_SETTING
    {
        unsigned long rUserValue =0;
        bt_default_GetMDRegMaskBits(
                pBtDevice,
                Addr,
                Msb,
                Lsb,
                &rUserValue
                );
        SYSLOGI("[MD][Reg=0x%.2x][%2d:%2d][Data=0x%.2x <-- 0x%.2x]",Addr,Msb,Lsb,rUserValue,UserValue);

    }
#endif

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
bt_default_GetMDRegMaskBits(
        BT_DEVICE *pBt,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        )
{
    uint8_t i;
    uint32_t ReadingValue;
    uint8_t RegAddr;
    uint8_t pEvtBuf[HCI_EVT_LEN_MAX];
    uint32_t len;
    uint32_t Mask;
    uint8_t Shift;


    RegAddr = ((Addr & 0x7f) >> 1) + 0x80;

    // Generate mask and shift according to MSB and LSB.
    Mask = 0;

    for (i = Lsb; i <= Msb; i++)
        Mask |= 0x1 << i;

    Shift = Lsb;

    if (bt_default_GetBytes(pBt, RegAddr, &ReadingValue, pEvtBuf, &len)) {
        goto error;
    }

    BTHCI_EvtReport(pEvtBuf, len);

    *pUserValue = (ReadingValue & Mask) >> Shift;

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

static int
BT_WriteHCIVendor(
        BT_DEVICE *pBtDevice,
        int Type,
        int ByteNum,
        uint32_t Addr,
        uint8_t *data
        )
{
    uint8_t pData[512];
    uint8_t pEvtBuf[512];
    uint32_t EvtLen;
    uint8_t Len;
    int i;
    int accressMode;

    memset(pData,0,512);
    memset(pEvtBuf,0,512);

    pData[0] = Type&0x03;
    switch(ByteNum)
    {
        case 1: accressMode=0;    break;
        case 2: accressMode =1;   break;
        case 4: accressMode =2;   break;
        default:
                goto error;
    }
    pData[0]=pData[0] | (accressMode<<4);

    for ( i = 0 ; i < 4 ; i++)
    {
        pData[i+1] = (uint8_t)((Addr>>(i*8)) & 0xff);
    }

    for (i = 0 ; i < ByteNum ; i++)
        pData[5+i]= data[i];

    Len = LEN_5_BYTE + ByteNum;

    if (pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xfc62, Len, pData, 0x0E, pEvtBuf, &EvtLen))
        goto error;

    if(pEvtBuf[5] != 0)
    {
        goto error;
    }

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

static int
BT_ReadHCIVendor(
        BT_DEVICE *pBtDevice,
        int Type,
        int ByteNum,
        uint32_t Addr,
        uint8_t *data
        )
{
    uint8_t pData[512];
    uint8_t pEvtBuf[512];
    uint32_t EvtLen;
    int i;
    int accressMode;

    memset(pData,0,512);
    memset(pEvtBuf,0,512);

    pData[0]=Type&0x03;
    switch(ByteNum)
    {
        case 1: accressMode=0;     break;
        case 2: accressMode =1;     break;
        case 4: accressMode =2;     break;
        default:
                goto error;

    }
    pData[0]=pData[0] | (accressMode<<4);
    for (i = 0; i < 4; i++)
    {
        pData[i+1]=(uint8_t)((Addr>>(i*8)) & 0xff);
    }

    if (pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xfc61, LEN_5_BYTE, pData, 0x0E, pEvtBuf, &EvtLen))
        goto error;

    if (pEvtBuf[5] != 0)
    {
        goto error;
    }

    for ( i = 0 ; i < ByteNum ; i++ )
        data[i] = pEvtBuf[6+i];

    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;
}

#define BT_REGISTER_IO_ACCESS_MODE  0
#define BT_SYSTEM_ACCESS_MODE       2

static int
BT_SetBBRegBytes(
        BT_DEVICE *pBtDevice,
        int Page,
        int Address,
        int ByteNumber,
        uint8_t *data
        )
{
    unsigned long baseaddress;
    int i;

    switch (Page)
    {
        case 0:   baseaddress= BT_BLUEWIZ0_ADDR; break;
        case 1:   baseaddress= BT_HCI_DMA_ADDR; break;
        case 2:   baseaddress= BT_TIMER_ADDR; break;
        case 3:   baseaddress= BT_GPIO_ADDR; break;
        case 4:   baseaddress= BT_UART_ADDR; break;
        case 5:   baseaddress= BT_H_UART_ADDR; break;
        case 6:   baseaddress= BT_SPIC_ADDR; break;
        case 7:   baseaddress= BT_LE_ADDR; break;
        default:
        case 8:   baseaddress= BT_VENDOR_BZDMA_ADDR; break;
        case 9:   baseaddress= BT_BLUEWIZ9_15_ADDR; break;
    }
    Address = baseaddress | Address;
    for (i = 0 ; i < ByteNumber ; i += 2)
    {
        if( BT_WriteHCIVendor(pBtDevice, BT_REGISTER_IO_ACCESS_MODE, LEN_2_BYTE, Address+i, &data[i]))
            goto error;
    }

    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;
}

static int
BT_GetBBRegBytes(
        BT_DEVICE *pBtDevice,
        int Page,
        int Address,
        int ByteNumber,
        uint8_t *data
        )
{
    unsigned long baseaddress;
    int i;

    switch (Page)
    {
        case 0:   baseaddress= BT_BLUEWIZ0_ADDR; break;
        case 1:   baseaddress= BT_HCI_DMA_ADDR; break;
        case 2:   baseaddress= BT_TIMER_ADDR; break;
        case 3:   baseaddress= BT_GPIO_ADDR; break;
        case 4:   baseaddress= BT_UART_ADDR; break;
        case 5:   baseaddress= BT_H_UART_ADDR; break;
        case 6:   baseaddress= BT_SPIC_ADDR; break;
        case 7:   baseaddress= BT_LE_ADDR; break;
        default:
        case 8:   baseaddress= BT_VENDOR_BZDMA_ADDR; break;
        case 9:   baseaddress= BT_BLUEWIZ9_15_ADDR; break;
    }
    Address = baseaddress |Address;

    for (i = 0 ; i < ByteNumber ; i += 2)
    {
        if( BT_ReadHCIVendor(pBtDevice, BT_REGISTER_IO_ACCESS_MODE, LEN_2_BYTE, Address+i, &data[i]))
            goto error;
    }

    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;
}

int
BT_SetSysBytes(
        BT_DEVICE *pBtDevice,
        int Address,
        int ByteNumber,
        uint8_t *data
        )
{
    int i;

    for ( i = 0 ; i < ByteNumber ; i++ )
    {
        if( BT_WriteHCIVendor(pBtDevice, BT_SYSTEM_ACCESS_MODE, LEN_1_BYTE, Address+i, &data[i]))
            goto error;
    }

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
BT_GetSysBytes(
        BT_DEVICE *pBtDevice,
        int Address,
        int ByteNumber,
        uint8_t *data
        )
{
    int i;

    for ( i = 0 ; i < ByteNumber ; i++ )
    {
        if( BT_ReadHCIVendor(pBtDevice, BT_SYSTEM_ACCESS_MODE, LEN_1_BYTE, Address+i, &data[i]))
            goto error;
    }

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
BT_SetSysRegMaskBits(
        BT_DEVICE *pBtDevice,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        )
{
    uint8_t pBuf[LEN_4_BYTE];
    uint16_t ReadingValue = 0;
    uint16_t WritingValue;
    uint16_t Mask = 0;
    uint8_t Len = (Msb / 8) + 1;
    uint8_t Shift = Lsb;
    uint8_t i;

    for (i = Lsb; i < Msb + 1; i++)
        Mask |= 0x1 << i;

    if (BT_GetSysBytes(pBtDevice, Addr, Len, pBuf))
        goto error;

    for (i = 0; i < Len; i++)
    {
        ReadingValue += (pBuf[i]<<(i*8));
    }

    WritingValue = (((ReadingValue) & (~Mask)) | (UserValue << Shift));

    for (i = 0; i < Len; i++)
    {
        pBuf[i] = (uint8_t)((WritingValue>>(i*8)) & 0xff);
    }

    if (BT_SetSysBytes(pBtDevice, Addr, Len, pBuf))
        goto error;

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
BT_GetSysRegMaskBits(
        BT_DEVICE *pBtDevice,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        )
{
    uint8_t pBuf[LEN_4_BYTE];
    uint16_t ReadingValue = 0;
    uint16_t Mask = 0;
    uint8_t Len = (Msb / 8) + 1;
    uint8_t Shift = Lsb;
    uint8_t i;

    for (i = Lsb; i < Msb + 1; i++)
        Mask |= 0x1 << i;

    if (BT_GetSysBytes(pBtDevice, Addr, Len, pBuf))
        goto error;

    for (i = 0; i < Len; i++)
    {
        ReadingValue += (pBuf[i]<<(i*8)) ;
    }

    *pUserValue = (ReadingValue & Mask) >> Shift;

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
BT_SetBBRegMaskBits(
        BT_DEVICE *pBtDevice,
        int Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        )
{
    uint8_t pBuf[LEN_4_BYTE];
    uint16_t ReadingValue = 0;
    uint16_t WritingValue;
    uint16_t Mask = 0;
    uint8_t Shift = Lsb;
    uint8_t i;

    if (Addr & 0x1)
    {
        SYSLOGE("BT_SetBBRegMaskBits: Addr should be 2-byte align");
        goto error;
    }

    if ((Msb < Lsb) || (Msb > 15))
    {
        SYSLOGE("BT_SetBBRegMaskBits: ERROR: Msb %d, Lsb %d", Msb, Lsb);
        goto error;
    }

    for (i = Lsb; i < Msb + 1; i++)
        Mask |= 0x1 << i;

    if (BT_GetBBRegBytes(pBtDevice, Page, Addr, LEN_2_BYTE, pBuf))
        goto error;

    for (i = 0; i < LEN_2_BYTE; i++)
    {
        ReadingValue += (pBuf[i]<<(i*8)) ;
    }

    WritingValue = (((ReadingValue) & (~Mask)) | ((UserValue << Shift) & Mask));

    for (i = 0; i < LEN_2_BYTE; i++)
    {
        pBuf[i] = (WritingValue>>(i*8))&0xff;
    }

    if (BT_SetBBRegBytes(pBtDevice, Page, Addr, LEN_2_BYTE, pBuf))
        goto error;

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
BT_GetBBRegMaskBits(
        BT_DEVICE *pBtDevice,
        int Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        )
{
    uint8_t pBuf[LEN_4_BYTE];
    uint16_t ReadingValue = 0;
    uint16_t Mask = 0;
    uint8_t Shift = Lsb;
    uint8_t i;

    if (Addr & 0x1)
    {
        SYSLOGE("BT_SetBBRegMaskBits: Addr should be 2-byte align");
        goto error;
    }

    if ((Msb < Lsb) || (Msb > 15))
    {
        SYSLOGE("BT_SetBBRegMaskBits: ERROR: Msb %d, Lsb %d", Msb, Lsb);
        goto error;
    }

    for (i = Lsb; i < Msb + 1; i++)
        Mask |= 0x1 << i;

    if (BT_GetBBRegBytes(pBtDevice, Page, Addr, LEN_2_BYTE, pBuf))
        goto error;

    for (i = 0; i < LEN_2_BYTE; i++)
    {
        ReadingValue += (pBuf[i]<<(i*8)) ;
    }

    *pUserValue = (ReadingValue & Mask) >> Shift;

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
bt_default_GetChipId(
        BT_DEVICE *pBtDevice
        )
{
    uint8_t pPayload[MAX_HCI_COMANND_BUF_SIZ];
    uint8_t pEvent[MAX_HCI_EVENT_BUF_SIZ];
    uint16_t OpCode=0xfc6f;
    uint8_t pPayload_Len=0;
    uint32_t EvtLen;
    int rtn=BT_FUNCTION_SUCCESS;
    BT_CHIPINFO *pBTInfo = pBtDevice->pBTInfo;

    if (pBtDevice->SendHciCommandWithEvent(pBtDevice,OpCode,pPayload_Len,pPayload,0x0E,pEvent,&EvtLen) != BT_FUNCTION_SUCCESS)
    {
        rtn=FUNCTION_HCISEND_ERROR;
        goto exit;
    }

    pBTInfo->ChipType=pEvent[EVT_CHIP_ECO_VERSION];
    pBTInfo->Is_After_PatchCode=1;
    rtn= BT_FUNCTION_SUCCESS;

exit:
    return rtn;
}

int
bt_default_GetECOVersion(
        BT_DEVICE *pBtDevice
        )
{
    uint8_t pPayload[MAX_HCI_COMANND_BUF_SIZ];
    uint8_t pEvent[MAX_HCI_EVENT_BUF_SIZ];
    uint16_t OpCode=0xfc6d;
    uint8_t pPayload_Len=0;
    uint32_t EvtLen;
    int rtn=BT_FUNCTION_SUCCESS;
    BT_CHIPINFO *pBTInfo = pBtDevice->pBTInfo;

    if (pBtDevice->SendHciCommandWithEvent(pBtDevice,OpCode,pPayload_Len,pPayload,0x0E,pEvent,&EvtLen) != BT_FUNCTION_SUCCESS)
    {
        rtn=FUNCTION_HCISEND_ERROR;
        goto exit;
    }

    pBTInfo->Version=pEvent[EVT_CHIP_ECO_VERSION]+1;
    rtn= BT_FUNCTION_SUCCESS;

exit:
    return rtn;
}

int bt_default_GetBTChipVersionInfo(
        BT_DEVICE *pBtDevice
        )
{
    uint8_t pPayload[MAX_HCI_COMANND_BUF_SIZ];
    uint8_t pEvent[MAX_HCI_EVENT_BUF_SIZ];
    uint32_t EventLen = 0;
    uint16_t OpCode=0x1001;
    uint8_t pPayload_Len=0;
    int rtn=BT_FUNCTION_SUCCESS;
    BT_CHIPINFO *pBTInfo = pBtDevice->pBTInfo;

    if (pBtDevice->SendHciCommandWithEvent(pBtDevice,OpCode,pPayload_Len,pPayload,0x0E,pEvent, &EventLen) != BT_FUNCTION_SUCCESS)
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
    if ( (pBTInfo->LMP_SubVersion == 0x1200) && (pBTInfo->HCI_SubVersion == 0x000B) ){
            pBTInfo->ChipType = RTK_BT_CHIP_ID_RTL8723A;
    }else if ( (pBTInfo->LMP_SubVersion == 0x8723) && (pBTInfo->HCI_SubVersion == 0x000A) ){
            pBTInfo->ChipType = RTK_BT_CHIP_ID_RTL8723A;
    }else if ( (pBTInfo->LMP_SubVersion == 0x8723) && (pBTInfo->HCI_SubVersion == 0x000B) ){
            pBTInfo->ChipType = RTK_BT_CHIP_ID_RTL8723B;
    }else if ( (pBTInfo->LMP_SubVersion == 0x8821) && (pBTInfo->HCI_SubVersion == 0x000A) ){
            pBTInfo->ChipType = RTK_BT_CHIP_ID_RTL8821A;
    }else if ( (pBTInfo->LMP_SubVersion == 0x8761) && (pBTInfo->HCI_SubVersion == 0x000A) ){
        pBTInfo->ChipType = RTK_BT_CHIP_ID_RTL8761A;
    }
    else
    {
        if (pBtDevice->GetChipId(pBtDevice) != BT_FUNCTION_SUCCESS)
        {
            rtn= FUNCTION_ERROR;
            goto exit;
        }
    }
    if (pBtDevice->GetECOVersion(pBtDevice) != BT_FUNCTION_SUCCESS)
    {
        rtn= FUNCTION_ERROR;
        goto exit;
    }

    rtn=BT_FUNCTION_SUCCESS;

exit:
    return rtn;
}

int
bt_default_GetBTChipDesc(
        BT_DEVICE *pBtDevice,
        int ID,
        char *ChipName)
{
    return BT_FUNCTION_SUCCESS;
}

int
bt_default_BTDlFW(
        BT_DEVICE *pBtDevice,
        uint8_t *pPatchcode,
        int patchLength
        )
{
    uint8_t pPayload[MAX_HCI_COMANND_BUF_SIZ];
    uint8_t pEvent[MAX_HCI_EVENT_BUF_SIZ];
    uint16_t OpCode=0xFC20;
    uint8_t pPayload_Len=0;
    uint8_t EventType=0x0e;
    uint32_t EvtLen;
    int rtn=BT_FUNCTION_SUCCESS;
    int StartIndex=0;//,m=0;
    int flag=0,n=0;
    int SegmentIndex=0;
    int LastSegment=patchLength/252;

    if ((patchLength%252) !=0)
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
            pPayload[0]= (SegmentIndex | 0x80);
        }
        else
            pPayload[0]= (SegmentIndex &0x7f);

        for (n=0;n<pPayload_Len;n++)
        {
            pPayload[n+1]=pPatchcode[StartIndex+n];
        }

        if (pBtDevice->SendHciCommandWithEvent(pBtDevice,OpCode,pPayload_Len+1,pPayload,EventType,pEvent,&EvtLen) != BT_FUNCTION_SUCCESS)
        {
            if (flag)
                rtn=BT_FUNCTION_SUCCESS;
            else
                rtn=FUNCTION_HCISEND_ERROR;

            goto exit;
        }

        SYSLOGI("[%d][Len=%d][offset=%d][ret=%x](%d)",SegmentIndex,pPayload_Len,StartIndex,pEvent[5],LastSegment);

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
bt_default_BTDlMergerFW(
        BT_DEVICE *pBtDevice,
        uint8_t *pPatchcode,
        int patchLength)
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

    if ((Patch_Code_Length ==0) || (Patch_Code_Length >40000))
    {
        rtn=FUNCTION_ERROR;
        goto exit;
    }

    SYSLOGI(">>Patch offset=%x Len=%d",Start_Offset,Patch_Code_Length);

    if (pBtDevice->BTDlFW(pBtDevice,&pPatchcode[Start_Offset],Patch_Code_Length) != BT_FUNCTION_SUCCESS)
    {
        rtn=FUNCTION_ERROR;
        goto exit;
    }

exit:
    return rtn;
}
