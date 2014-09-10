#define LOG_TAG "bt_mp_transport"

#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "bt_syslog.h"
#include "bluetoothmp.h"
#include "bt_mp_transport.h"
#include "btif_api.h"

int bt_transport_SendHciCmd(
    BASE_INTERFACE_MODULE *pBaseInterface,
    uint8_t *pCmdBuffer,
    uint32_t bufferLen
    )
{
    uint16_t opcode;
    uint8_t paraLen = 0;
    uint8_t *pParaBuffer = NULL;

    opcode = *pCmdBuffer;
    opcode |= *(pCmdBuffer + 1) << 8;
    paraLen = *(uint8_t *)(pCmdBuffer + sizeof(opcode));

    pParaBuffer = pCmdBuffer +sizeof(opcode) + sizeof(paraLen);

    return btif_dut_mode_send(opcode, pParaBuffer, paraLen);
}


void bt_transport_signal_event(BASE_INTERFACE_MODULE *pBaseInterface, unsigned short event)
{
    pthread_mutex_lock(&pBaseInterface->mutex);
    pBaseInterface->rx_ready_events |= event;
    pthread_cond_signal(&pBaseInterface->cond);
    pthread_mutex_unlock(&pBaseInterface->mutex);
}

int bt_transport_RecvHciEvt(
        BASE_INTERFACE_MODULE *pBaseInterface,
        uint8_t *pEvtBuffer,
        uint32_t bufferLen,
        uint32_t *pRetEvtLen
        )
{
    unsigned short events = 0;

    while(1)
    {
        pthread_mutex_lock(&pBaseInterface->mutex);
        while (pBaseInterface->rx_ready_events == 0)
        {
            pthread_cond_wait(&pBaseInterface->cond, &pBaseInterface->mutex);
        }

        events = pBaseInterface->rx_ready_events;
        pBaseInterface->rx_ready_events = 0;
        pthread_mutex_unlock(&pBaseInterface->mutex);

        if(events & MP_TRANSPORT_EVENT_RX_HCIEVT)
        {
            *pRetEvtLen = pBaseInterface->evtLen;
            SYSLOGI("pEvtBuffer %p, pBaseInterface->evtBuffer %p, pBaseInterface->evtLen %d",
                    pEvtBuffer, pBaseInterface->evtBuffer, pBaseInterface->evtLen);
            memcpy(pEvtBuffer, pBaseInterface->evtBuffer, pBaseInterface->evtLen);
            break;
        }
        else
        if(events & MP_TRANSPORT_EVENT_RX_EXIT)
        {
            break;
        }

    }
    return 0;
}
