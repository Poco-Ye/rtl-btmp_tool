#ifndef _BT_MP_BUILD_H
#define _BT_MP_BUILD_H

#include "bt_mp_device_base.h"
#include "bt_mp_device_general.h"
#include "bt_mp_module_base.h"

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
