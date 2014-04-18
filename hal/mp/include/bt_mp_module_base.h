#ifndef _BT_MP_MODULE_BASE_H
#define _BT_MP_MODULE_BASE_H

#include "bt_mp_base.h"

int
BTModule_ActionControlExcute(
        BT_MODULE *pBtModule
        );

int
BTModule_ActionReport(
        BT_MODULE *pBtModule,
        int ActiceItem,
        BT_DEVICE_REPORT *pReport
        );

int
BTModule_UpDataParameter(
        BT_MODULE *pBtModule,
        BT_PARAMETER *pParam
        );


int
BTModule_DownloadPatchCode(
        BT_MODULE *pBtModule,
        unsigned char *pPatchcode,
        int patchLength,
        int Mode);

int
BTModule_SendHciCommandWithEvent(
    BT_MODULE *pBtModule,
    unsigned int  OpCode,
    unsigned char PayLoadLength,
    unsigned char *pPayLoad,
    unsigned char EventType,
    unsigned char *pEvent,
    unsigned long *pEventLen
    );

int
BTModule_RecvAnyHciEvent(
        BT_MODULE *pBtModule,
        unsigned char *pEvent
        );

int
BTModule_GetMDRegMaskBits(
        BT_MODULE *pBtModule,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        );

int
BTModule_SetMDRegMaskBits(
        BT_MODULE *pBtModule,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        const uint16_t UserValue
        );

int
BTModule_GetRFRegMaskBits(
        BT_MODULE *pBtModule,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        );

int
BTModule_SetRFRegMaskBits(
        BT_MODULE *pBtModule,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        const uint16_t UserValue
        );

int
BTModule_GetSysRegMaskBits(
        BT_MODULE *pBtModule,
        unsigned char Addr,
        unsigned char Msb,
        unsigned char Lsb,
        uint32_t *pUserValue
        );

int
BTModule_SetSysRegMaskBits(
        BT_MODULE *pBtModule,
        unsigned char Addr,
        unsigned char Msb,
        unsigned char Lsb,
        const uint32_t UserValue
        );

int
BTModule_GetBBRegMaskBits(
        BT_MODULE *pBtModule,
        uint8_t Page,
        unsigned char Addr,
        unsigned char Msb,
        unsigned char Lsb,
        uint32_t *pUserValue
        );

int
BTModule_SetBBRegMaskBits(
        BT_MODULE *pBtModule,
        uint8_t Page,
        unsigned char Addr,
        unsigned char Msb,
        unsigned char Lsb,
        const uint32_t UserValue
        );

#endif