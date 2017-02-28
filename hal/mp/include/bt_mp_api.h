#ifndef _BT_MP_API_H
#define _BT_MP_API_H

#include "bt_mp_base.h"

int BT_GetParam(BT_MODULE *pBtModule, char *p, char *buf_cb);
int BT_SetParam(BT_MODULE *pBtModule, char *p, char *buf_cb);
int BT_SetConfig(BT_MODULE *pBtModule, char *p, char *buf_cb);
int BT_Exec(BT_MODULE *pBtModule, char *p, char *buf_cb);
int BT_Report(BT_MODULE *pBtModule, char *p, char *buf_cb);
int BT_RegRW(BT_MODULE *pBtModule, char *p, char *buf_cb);
int BT_SendHciCmd(BT_MODULE *pBtModule, char *p, char *buf_cb);
#if (MP_TOOL_COMMAND_SEARCH_EXIST_PERMISSION == 1)
int BT_search(BT_MODULE *pBtModule, char *p, char *buf_cb);
#endif
#if (MP_TOOL_COMMAND_READ_PERMISSION == 1)
int BT_read(BT_MODULE *pBtModule, char *p, char *buf_cb);
#endif

void bt_mp_module_init(BASE_INTERFACE_MODULE *pBaseInterfaceModuleMemory, BT_MODULE *pBtModuleMemory);

#endif
