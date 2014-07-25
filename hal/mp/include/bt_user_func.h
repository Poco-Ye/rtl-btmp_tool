#ifndef _BT_USER_FUNC_H
#define _BT_USER_FUNC_H

#include "bt_mp_base.h"

void
UserDefinedWaitMs(
        BASE_INTERFACE_MODULE *pBaseInterface,
        unsigned long WaitTimeMs
        );

int
UserDefined_Open_Func(
        BASE_INTERFACE_MODULE *pBaseInterface
        );

int
UserDefined_Send_Func(
        BASE_INTERFACE_MODULE *pBaseInterface,
        uint8_t *pWritingBuf,
        uint32_t Len
        );

int
UserDefined_Recv_Func(
        BASE_INTERFACE_MODULE *pBaseInterface,
        uint8_t *pReadingBuf,
        uint32_t Len,
        uint32_t *pRetLen
        );

int
UserDefined_Close_Func(
        BASE_INTERFACE_MODULE *pBaseInterface
        );

#endif
