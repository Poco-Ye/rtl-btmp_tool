#define LOG_TAG "bt_user_func"

#include <unistd.h>
#include <utils/Log.h>

#include "bt_user_func.h"

void
UserDefinedWaitMs(
        BASE_INTERFACE_MODULE *pBaseInterface,
        unsigned long WaitTimeMs
        )
{
    usleep(WaitTimeMs*1000);
    return;
}

int
UserDefined_Open_Func(
        BASE_INTERFACE_MODULE *pBaseInterface
        )
{
    return 0;
}

int
UserDefined_Send_Func(
        BASE_INTERFACE_MODULE *pBaseInterface,
        uint8_t *pWritingBuf,
        uint32_t Len
        )
{
    return 0;
}

int
UserDefined_Recv_Func(
        BASE_INTERFACE_MODULE *pBaseInterface,
        uint8_t *pReadingBuf,
        uint32_t Len,
        uint32_t *pRetLen
        )
{

    return 0;
}

int
UserDefined_Close_Func(
        BASE_INTERFACE_MODULE *pBaseInterface
        )
{
    return 0;
}
