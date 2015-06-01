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
BuildEfuseLogicUnit(
        BT_DEVICE *pBtDevice,
        EFUSE_UNIT **ppEfuseModule,
        EFUSE_UNIT *pEfuseModuleMemory,
        uint16_t EfuseLogSize,
        uint16_t EfusePhySize,
        uint8_t StartBank,
        uint8_t BankNum
        );

int
BTDevice_Efuse_LoadPhyMem(
        EFUSE_UNIT *pEfuse
        );

int
BTDevice_Efuse_Phy2LogMap(
        EFUSE_UNIT *pEfuse
        );

int
BTDevice_Efuse_UpdateLogMem(
        EFUSE_UNIT *pEfuse,
        uint16_t Addr,
        uint8_t Len,
        uint8_t *pBuf
        );

int
BTDevice_Efuse_Log2PhyMap(
        EFUSE_UNIT *pEfuse
        );

#endif
