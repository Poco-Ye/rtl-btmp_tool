#ifndef _BT_MP_DEVICE_SKIP_H
#define _BT_MP_DEVICE_SKIP_H

#include "bt_mp_base.h"

int BTDevice_SetPacketType_NOSUPPORT(BT_DEVICE *pBtDevice,BT_PKT_TYPE PktType);
int BTDevice_SetWhiteningCoeff_NOSUPPORT(BT_DEVICE *pBtDevice, uint8_t WhiteningCoeffValue);
int BTDevice_SetPayloadType_NOSUPPORT(BT_DEVICE *pBtDevice,BT_PAYLOAD_TYPE PayloadType);
int BTDevice_SetTxChannel_NOSUPPORT(BT_DEVICE *pBtDevice, uint8_t ChannelNumber);
int BTDevice_SetRxChannel_NOSUPPORT(BT_DEVICE *pBtDevice, uint8_t ChannelNumber);
int BTDevice_SetPowerGain_NOSUPPORT(BT_DEVICE *pBtDevice, uint8_t PowerGainValue);
int BTDevice_SetPowerDac_NOSUPPORT(BT_DEVICE *pBtDevice, uint8_t DacValue);
int BTDevice_SetTestMode_NOSUPPORT(BT_DEVICE *pBtDevice,BT_TEST_MODE TestMode);
int BTDevice_SetFWPowerTrackEnable_NOSUPPORT(BT_DEVICE *pBtDevice, uint8_t FWPowerTrackEnable);
int BTDevice_SetHitTarget_NOSUPPORT(BT_DEVICE *pBtDevice, uint64_t HitTarget);
int BTDevice_SetFWPowerTrackEnable_NOSUPPORT(BT_DEVICE *pBtDevice, uint8_t FWPowerTrackEnable);
int BTDevice_SetHoppingMode_NOSUPPORT(BT_DEVICE *pBtDevice,BT_PKT_TYPE pktType, uint8_t bHoppingFixChannel, uint8_t Channel, uint8_t WhiteningCoeffValue);
int BTDevice_SetHCIReset_NOSUPPORT(BT_DEVICE *pBtDevice,int Delay_mSec);
int BTDevice_SetPowerGainIndex_NOSUPPORT(BT_DEVICE *pBtDevice,int Index);
int BTDevice_GetBTClockTime_NOSUPPORT(BT_DEVICE *pBtDevice,unsigned long btClockTime);



#endif
