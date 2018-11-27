
#ifndef __BT_MP_DEVICE_RTL8822C_H
#define __BT_MP_DEVICE_RTL8822C_H

#include "bt_mp_base.h"
#include "bt_mp_device_base.h"



int
BTDevice_RTL8822C_SetAntDiffS0S1(
    BT_DEVICE *pBtDevice,
    unsigned char s0_s1,
    unsigned char tbt_diff_sos1
    );


#endif
