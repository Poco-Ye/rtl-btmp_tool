#ifndef _BT_MP_DEVICE_GENERAL_H
#define _BT_MP_DEVICE_GENERAL_H

#include "bt_mp_base.h"

int BTDevice_SetPayloadType(BT_DEVICE *pBtDevice, BT_PAYLOAD_TYPE PayloadType);
int BTDevice_SetTxChannel(BT_DEVICE *pBtDevice, uint8_t ChannelNumber);
int BTDevice_SetRxChannel(BT_DEVICE *pBtDevice, uint8_t ChannelNumber);
int BTDevice_SetPowerGain(BT_DEVICE *pBtDevice, uint8_t PowerGainValue);
int BTDevice_SetPowerDac(BT_DEVICE *pBtDevice, uint8_t DacValue);
int BTDevice_SetTestMode(BT_DEVICE *pBtDevice, BT_TEST_MODE TestMode);
int BTDevice_SetPowerGainIndex(BT_DEVICE *pBtDevice,int Index);
int BTDevice_SpecialFunction_Efuse_GetPGEfuseLength(BT_DEVICE *pBtDevice, uint8_t Bank, uint32_t *PGLength);
int BTDevice_PGEfuseRawData(BT_DEVICE *pBtDevice,int MapType, uint8_t *PGData, uint32_t PGDataLength);
int BTDevice_SpecialFunction_Efuse_WriteBytes(BT_DEVICE *pBtDevice, uint8_t Bank, uint16_t StartAddress, uint16_t WriteLen, uint8_t *pWriteData);

#endif
