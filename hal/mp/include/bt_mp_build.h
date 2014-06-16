#ifndef BT_MP_BUILD_H
#include "bt_mp_base.h"
#include "bt_user_func.h"
#include "bt_mp_device_general.h"
#include "bt_mp_device_base.h"
#include "bt_mp_device_skip.h"
#include "bt_mp_device_rtl8723a.h"
#include "bt_mp_module_base.h"

#include "foundation.h"

int
BuildBluetoothModule(
        BASE_INTERFACE_MODULE    *pBaseInterfaceModule,
        BT_MODULE                *pBtModule,
        void                     *pExtra,
        uint8_t                  *pTxGainTable,
        uint8_t                  *pTxDACTable
        );

int
BuildBluetoothDevice(
        BASE_INTERFACE_MODULE    *pBaseInterface,
        BT_DEVICE                **ppBtDeviceBase,
        BT_DEVICE                *pDeviceBasememory,
        void                     *pExtra,
        uint8_t                  *pTxGainTable,
        uint8_t                  *pTxDACTable
        );


#endif
