#define LOG_TAG "bt_mp_base"

#include <stdio.h>

#include "bt_syslog.h"
#include "bluetoothmp.h"
#include "bt_mp_base.h"

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
    uint8_t ucRecvBuf[HCI_EVT_LEN_MAX];
    uint32_t Retlen = 0;

    pBaseInterface = pBt->pBaseInterface;

    if (pBaseInterface->Recv(pBaseInterface, ucRecvBuf, HCI_EVT_LEN_MAX, &Retlen) != BT_FUNCTION_SUCCESS)
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
    uint8_t ucRecvBuf[HCI_EVT_LEN_MAX];
    uint32_t Retlen = 0;

    pBaseInterface = pBt->pBaseInterface;

    if (pBaseInterface->Recv(pBaseInterface, ucRecvBuf, HCI_EVT_LEN_MAX, &Retlen) != BT_FUNCTION_SUCCESS)
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
    uint32_t Value;
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
        if (bt_default_GetBytes(pBt, RegAddr, &Value, pEvtBuf, &len)) {
            goto error;
        }

        Value &= ~Mask;
        Value |= (UserValue << Shift) & Mask;

        if (bt_default_SetBytes(pBt, RegAddr, Value, pEvtBuf, &len)) {
            goto error;
        }
    } else {
        if (bt_default_SetBytes(pBt, RegAddr, UserValue, pEvtBuf, &len)) {
            goto error;
        }
    }

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
    uint32_t Value;
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
        if (bt_default_GetBytes(pBtDevice, RegAddr, &Value, pEvtBuf, &len)) {
            goto error;
        }

        Value &= ~Mask;
        Value |= (UserValue << Shift) & Mask;

        if (bt_default_SetBytes(pBtDevice, RegAddr, Value, pEvtBuf, &len)) {
            goto error;
        }
    } else {
        if (bt_default_SetBytes(pBtDevice, RegAddr, UserValue, pEvtBuf, &len)) {
            goto error;
        }
    }

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

    *pUserValue = (ReadingValue & Mask) >> Shift;

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
bt_default_SendHciCommandWithEvent(
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
    uint8_t pWritingBuf[HCI_CMD_LEN_MAX];
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

    if (bt_default_SendHCICmd(pBtDevice, HCIIO_BTCMD, pWritingBuf, len) != BT_FUNCTION_SUCCESS)
    {
        SYSLOGI("bt_default_SendHciCommandWithEvent, ERROR: SendHciCmd");
        goto exit;
    }

    if (((OpCode == 0xFC20) && ((pPayLoad[0] & 0x80) != 0)) || (OpCode == 0x0C03))
    {
        pBaseInterface->WaitMs(pBaseInterface,965);
    }

    if (bt_default_RecvHCIEvent(pBtDevice, HCIIO_BTEVT, pEvent, pEventLen) != BT_FUNCTION_SUCCESS)
    {
        SYSLOGI("bt_default_SendHciCommandWithEvent, ERROR: RecvHciEvent");
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
        SYSLOGI("bt_default_SendHciCommandWithEvent, ERROR: hci_rtn %d", hci_rtn);
        goto exit;
    }

    return BT_FUNCTION_SUCCESS;

exit:
    return FUNCTION_ERROR;
}

int
bt_default_RecvAnyHciEvent(
        BT_DEVICE *pBtDevice,
        uint8_t *pEvent
        )
{
    uint32_t Retlen =0;
    if (bt_default_RecvHCIEvent(pBtDevice, HCIIO_BTEVT, pEvent, &Retlen) != BT_FUNCTION_SUCCESS)
    {
        goto exit;
    }
    return BT_FUNCTION_SUCCESS;

exit:
    return FUNCTION_ERROR;
}


static int
bt_default_WriteHCIVendor(
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

    if (bt_default_SendHciCommandWithEvent(pBtDevice, 0xfc62, Len, pData, 0x0E, pEvtBuf, &EvtLen))
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
bt_default_ReadHCIVendor(
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

    if (bt_default_SendHciCommandWithEvent(pBtDevice, 0xfc61, LEN_5_BYTE, pData, 0x0E, pEvtBuf, &EvtLen))
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
bt_default_SetBBRegBytes(
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
        if( bt_default_WriteHCIVendor(pBtDevice, BT_REGISTER_IO_ACCESS_MODE, LEN_2_BYTE, Address+i, &data[i]))
            goto error;
    }

    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;
}

static int
bt_default_GetBBRegBytes(
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
        if( bt_default_ReadHCIVendor(pBtDevice, BT_REGISTER_IO_ACCESS_MODE, LEN_2_BYTE, Address+i, &data[i]))
            goto error;
    }

    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;
}

int
bt_default_SetSysBytes(
        BT_DEVICE *pBtDevice,
        int Address,
        int ByteNumber,
        uint8_t *data
        )
{
    int i;

    for ( i = 0 ; i < ByteNumber ; i++ )
    {
        if( bt_default_WriteHCIVendor(pBtDevice, BT_SYSTEM_ACCESS_MODE, LEN_1_BYTE, Address+i, &data[i]))
            goto error;
    }

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
bt_default_GetSysBytes(
        BT_DEVICE *pBtDevice,
        int Address,
        int ByteNumber,
        uint8_t *data
        )
{
    int i;

    for ( i = 0 ; i < ByteNumber ; i++ )
    {
        if( bt_default_ReadHCIVendor(pBtDevice, BT_SYSTEM_ACCESS_MODE, LEN_1_BYTE, Address+i, &data[i]))
            goto error;
    }

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
bt_default_SetSysRegMaskBits(
        BT_DEVICE *pBtDevice,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        )
{
    uint8_t pBuf[LEN_4_BYTE];
    uint16_t Value = 0;
    uint16_t Mask = 0;
    uint8_t Len = (Msb / 8) + 1;
    uint8_t Shift = Lsb;
    uint8_t i;

    for (i = Lsb; i < Msb + 1; i++)
        Mask |= 0x1 << i;

    if (bt_default_GetSysBytes(pBtDevice, Addr, Len, pBuf))
        goto error;

    for (i = 0; i < Len; i++)
        Value |= pBuf[i] << (BYTE_SHIFT * i);

    Value &= ~Mask;
    Value |= (UserValue << Shift ) & Mask;

    for (i = 0; i < Len; i++)
        pBuf[i] = (uint8_t)(Value >> (BYTE_SHIFT * i));

    if (bt_default_SetSysBytes(pBtDevice, Addr, Len, pBuf))
        goto error;

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
bt_default_GetSysRegMaskBits(
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

    if (bt_default_GetSysBytes(pBtDevice, Addr, Len, pBuf))
        goto error;

    for (i = 0; i < Len; i++)
        ReadingValue |= pBuf[i] << (BYTE_SHIFT * i) ;

    *pUserValue = (ReadingValue & Mask) >> Shift;

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
bt_default_SetBBRegMaskBits(
        BT_DEVICE *pBtDevice,
        int Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        )
{
    uint8_t pBuf[LEN_4_BYTE];
    uint16_t Value = 0;
    uint16_t Mask = 0;
    uint8_t Shift = Lsb;
    uint8_t i;

    if (Addr & 0x1) {
        SYSLOGE("bt_default_SetBBRegMaskBits: Addr should be 2-byte align");
        goto error;
    }

    if ((Msb < Lsb) || (Msb > 15)) {
        SYSLOGE("bt_default_SetBBRegMaskBits: ERROR: Msb %d, Lsb %d", Msb, Lsb);
        goto error;
    }

    for (i = Lsb; i < Msb + 1; i++)
        Mask |= 0x1 << i;

    if (bt_default_GetBBRegBytes(pBtDevice, Page, Addr, LEN_2_BYTE, pBuf))
        goto error;

    for (i = 0; i < LEN_2_BYTE; i++)
        Value |= pBuf[i] << (BYTE_SHIFT * i) ;

    Value &= ~Mask;
    Value |= (UserValue << Shift) & Mask;

    for (i = 0; i < LEN_2_BYTE; i++)
        pBuf[i] = Value >> (BYTE_SHIFT * i);

    if (bt_default_SetBBRegBytes(pBtDevice, Page, Addr, LEN_2_BYTE, pBuf))
        goto error;

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}

int
bt_default_GetBBRegMaskBits(
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

    if (Addr & 0x1) {
        SYSLOGE("bt_default_SetBBRegMaskBits: Addr should be 2-byte align");
        goto error;
    }

    if ((Msb < Lsb) || (Msb > 15)) {
        SYSLOGE("bt_default_SetBBRegMaskBits: ERROR: Msb %d, Lsb %d", Msb, Lsb);
        goto error;
    }

    for (i = Lsb; i < Msb + 1; i++)
        Mask |= 0x1 << i;

    if (bt_default_GetBBRegBytes(pBtDevice, Page, Addr, LEN_2_BYTE, pBuf))
        goto error;

    for (i = 0; i < LEN_2_BYTE; i++)
        ReadingValue |= pBuf[i] << (BYTE_SHIFT * i);

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
    uint8_t pPayload[HCI_CMD_LEN_MAX];
    uint8_t pEvent[HCI_EVT_LEN_MAX];
    uint16_t OpCode=0xfc6f;
    uint8_t pPayload_Len=0;
    uint32_t EvtLen;
    int rtn=BT_FUNCTION_SUCCESS;
    BT_CHIPINFO *pBTInfo = pBtDevice->pBTInfo;

    if (bt_default_SendHciCommandWithEvent(pBtDevice,OpCode,pPayload_Len,pPayload,0x0E,pEvent,&EvtLen) != BT_FUNCTION_SUCCESS)
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
    uint8_t pPayload[HCI_CMD_LEN_MAX];
    uint8_t pEvent[HCI_EVT_LEN_MAX];
    uint16_t OpCode=0xfc6d;
    uint8_t pPayload_Len=0;
    uint32_t EvtLen;
    int rtn = BT_FUNCTION_SUCCESS;
    BT_CHIPINFO *pBTInfo = pBtDevice->pBTInfo;

    if (bt_default_SendHciCommandWithEvent(pBtDevice,OpCode,pPayload_Len,pPayload,0x0E,pEvent,&EvtLen) == BT_FUNCTION_SUCCESS)
    {
        pBTInfo->Version = pEvent[EVT_CHIP_ECO_VERSION] + 1;
    } else {
        pBTInfo->Version = 1;
    }

    return rtn;
}
