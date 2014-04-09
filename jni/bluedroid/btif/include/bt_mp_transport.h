#ifndef _BT_MP_TRANSPORT_H
#define _BT_MP_TRANSPORT_H
#include "foundation.h"


int bt_transport_SendHciCmd(
    BASE_INTERFACE_MODULE *pBaseInterface,
    unsigned char *cmdBuffer,
    unsigned long bufferLen
    );

void bt_transport_signal_event(
    BASE_INTERFACE_MODULE *pBaseInterface,
    unsigned short event
    );

int bt_transport_RecvHciEvt(
    BASE_INTERFACE_MODULE *pBaseInterface,
    unsigned char *pEvtBuffer,
    unsigned long bufferLen,
    unsigned long *pRetEvtLen
    );

#endif
