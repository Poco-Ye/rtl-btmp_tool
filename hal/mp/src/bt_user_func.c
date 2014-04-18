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

unsigned int OsSleepSeconds(unsigned int seconds)
{
    return sleep(seconds);
}

unsigned long GetTickCount()
{
    struct timeval tv;
    if(gettimeofday(&tv, NULL) != 0)
        return 0;

    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
