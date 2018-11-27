#ifndef _BT_MP_DEVICE_RTL8763B_H_
#define _BT_MP_DEVICE_RTL8763B_H_

#include "bt_mp_base.h"
#include "bt_mp_device_base.h"

int
BTDevice_ReadThermal_RTL8763B(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    unsigned char *pThermalValue
    );



#endif
