#include "bt_mp_device_rtl8822c.h"
#include "bt_syslog.h"

int
BTDevice_RTL8822C_SetAntDiffS0S1(
    BT_DEVICE *pBtDevice,
    unsigned char s0_s1,
    unsigned char tbt_diff_sos1
    )
{
    SYSLOGI(" BTDevice_SetAntDiffS0S1 : s0_s1 = %d, tbt_diff_sos1 = %d\n", s0_s1,tbt_diff_sos1);
    unsigned char pData[LEN_512_BYTE];
    unsigned char pEvtBuf[LEN_512_BYTE];

    pData[0] = 1;
    unsigned int EvtLen;

     // bit 7 =1, bit 6 =0
    pData[1] = 0;
    pData[1] |= BIT_7_MASK;


    // bit 4  : s0 = 1  s1 =1
    if(s0_s1 == 0)
    {
        pData[1] &= (~BIT_4_MASK);
    }
    else
    {
        pData[1] |= BIT_4_MASK;
    }
    // new don't diff
//  pData[1] |= (tbt_diff_sos1&0x0f);
    pData[1] |= 0x0F;

    SYSLOGI(" BTDevice_RTL8723D_SetAntDiffS0S1 : 0x%x\n", tbt_diff_sos1);

    if(pBtDevice->SendHciCommandWithEvent(pBtDevice, HCI_VENDOR_MP_SET_ANT_INFO, LEN_2_BYTE, pData, 0x0E, pEvtBuf, &EvtLen))
        goto error;

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}
