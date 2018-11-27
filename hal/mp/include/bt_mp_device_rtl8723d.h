
#ifndef __BT_MP_DEVICE_RTL8723D_H
#define __BT_MP_DEVICE_RTL8723D_H

#include "bt_mp_base.h"
#include "bt_mp_device_base.h"



int
BTDevice_RTL8723D_SetAntInfo(
    BT_DEVICE *pBtDevice,
    unsigned char Data
    );


int
BTDevice_RTL8723D_SetAntDiffS0S1(
    BT_DEVICE *pBtDevice,
    unsigned char s0_s1,
    unsigned char tbt_diff_sos1
    );


#endif
