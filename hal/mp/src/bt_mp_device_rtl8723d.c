#include "bt_mp_device_rtl8723d.h"
#include "bt_syslog.h"

int
BTDevice_RTL8723D_SetAntInfo(
    BT_DEVICE *pBtDevice,
    unsigned char Data
    )
{
    unsigned char pData[LEN_512_BYTE];
    unsigned char pEvtBuf[LEN_512_BYTE];
    unsigned int EvtLen;
    SYSLOGI(" BTDevice_RTL8723D_SetAntInfo\n");

    pData[0] = 0;
    pData[1] = Data;
    if(pBtDevice->SendHciCommandWithEvent(pBtDevice, HCI_VENDOR_MP_SET_ANT_INFO, LEN_2_BYTE, pData, 0x0E, pEvtBuf, &EvtLen))
        goto error;

    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;

}

int
BTDevice_RTL8723D_SetAntDiffS0S1(
    BT_DEVICE *pBtDevice,
    unsigned char s0_s1,
    unsigned char tbt_diff_sos1
    )
{
    SYSLOGI(" BTDevice_SetAntDiffS0S1 : s0_s1 = %d, tbt_diff_sos1 = %d\n", s0_s1,tbt_diff_sos1);
    unsigned char pData[LEN_512_BYTE];
    unsigned char pEvtBuf[LEN_512_BYTE];
    unsigned int EvtLen;

    pData[0] = 1;

    pData[1] = 0;
    pData[1] |= BIT_7_MASK;

    if(s0_s1 == 0)
    {
        pData[1] &= (~BIT_4_MASK);
    }
    else
    {
        pData[1] |= BIT_4_MASK;
    }

    pData[1] |= (tbt_diff_sos1&0x0f);

    SYSLOGI(" BTDevice_RTL8723D_SetAntDiffS0S1 : 0x%x\n", pData[1]);

    if(pBtDevice->SendHciCommandWithEvent(pBtDevice, HCI_VENDOR_MP_SET_ANT_INFO, LEN_2_BYTE, pData, 0x0E, pEvtBuf, &EvtLen))
        goto error;

    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;

}
