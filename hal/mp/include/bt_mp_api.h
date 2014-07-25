#ifndef _BT_MP_API_H
#define _BT_MP_API_H

#include "bt_mp_base.h"

int BT_GetParam(BT_MODULE *pBtModule, char *pNotifyBuffer);
int BT_SetParam(BT_MODULE *pBtModule, char *p, char *pNotifyBuffer);
int BT_SetParam1(BT_MODULE *pBtModule, char *p, char *pNotifyBuffer);
int BT_SetParam2(BT_MODULE *pBtModule, char *p, char *pNotifyBuffer);
int BT_SetConfig(BT_MODULE *pBtModule, char *p, char *pNotifyBuffer);
int BT_SetGainTable(BT_MODULE *pBtModule, char *p, char *pNotifyBuffer);
int BT_SetDacTable(BT_MODULE *pBtModule, char *p, char *pNotifyBuffer);
int BT_Exec(BT_MODULE *pBtModule, char *p, char *pNotifyBuffer);
int BT_ReportTx(BT_MODULE *pBtModule, char *pNotifyBuffer);
int BT_ReportContTx(BT_MODULE *pBtModule, char *pNotifyBuffer);
int BT_ReportRx(BT_MODULE *pBtModule, char *pNotifyBuffer);
int BT_RegRW(BT_MODULE *pBtModule, char *p, char *pNotifyBuffer);

int BT_SendHciCmd(BT_MODULE *pBtModule, char *p, char *pNotifyBuffer);

void BT_GetBDAddr(BT_MODULE *pBtModule);
void bt_mp_module_init(BASE_INTERFACE_MODULE *pBaseInterfaceModuleMemory, BT_MODULE *pBtModuleMemory);

#endif
