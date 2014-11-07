#ifndef __BT_EFUSE_BASE_H
#define __BT_EFUSE_BASE_H

#include "bt_mp_base.h"

#define LEN_1_BYTE          1
#define LEN_2_BYTE          2
#define LEN_3_BYTE          3
#define LEN_4_BYTE          4
#define LEN_5_BYTE          5
#define LEN_6_BYTE          6
#define LEN_7_BYTE          7
#define LEN_8_BYTE          8
#define LEN_9_BYTE          9
#define LEN_10_BYTE         10
#define LEN_11_BYTE         11
#define LEN_14_BYTE         14
#define LEN_16_BYTE         16

#define DUMMY_EFUSE_LEN 16

int
BuildEfuseLogicModule(
        BT_DEVICE *pBtDevice,
        EFUSE_MODULE **ppEfuseModule,
        EFUSE_MODULE *pEfuseModuleMemory,
        unsigned int EfuseLogSize,
        unsigned int EfusePhySize,
        uint8_t StartBank,
        uint8_t BankNum
        );

int
BTDevice_Efuse_ReadData(
        EFUSE_MODULE *pEfuse
        );

int
BTDevice_Efuse_WriteData(
        EFUSE_MODULE *pEfuse
        );

int
BTDevice_Efuse_PhysicalToLogicalData(
        EFUSE_MODULE *pEfuse
        );

int
Efuse_SetValueToLogMem(
        EFUSE_MODULE *pEfuse,
        unsigned int Addr,
        uint8_t Value
        );

int
BTDevice_Efuse_LogicDataToWritingEntry(
        EFUSE_MODULE *pEfuse,
        unsigned int StartLogAddr,
        uint8_t *pWritingEntry,
        unsigned int *Len
        );

#endif
