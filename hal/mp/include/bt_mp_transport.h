#ifndef _BT_MP_TRANSPORT_H
#define _BT_MP_TRANSPORT_H

#include "foundation.h"

void bt_transport_WaitMs(
        BASE_INTERFACE_MODULE *pBaseInterface,
        unsigned long WaitTimeMs
        );

int bt_transport_SendHciCmd(
        BASE_INTERFACE_MODULE *pBaseInterface,
        uint8_t *cmdBuffer,
        uint32_t bufferLen
        );

void bt_transport_signal_event(
        BASE_INTERFACE_MODULE *pBaseInterface,
        unsigned short event
        );

int bt_transport_RecvHciEvt(
        BASE_INTERFACE_MODULE *pBaseInterface,
        uint8_t *pEvtBuffer,
        uint32_t bufferLen,
        uint32_t *pRetEvtLen
        );

#endif
