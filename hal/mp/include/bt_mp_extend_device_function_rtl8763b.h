#ifndef _BT_MP_EXTEND_DEVICE_FUNCTION_RTL8763B_H
#define _BT_MP_EXTEND_DEVICE_FUNCTION_RTL8763B_H


#include "bt_mp_base.h"
#include "bt_mp_device_flash_rtl8763b.h"

/*-------------------------------------------*
RTL8763B SubIndex
*---------------------------------------------*/
#define HCI_VENDOR_MP_K_POWER_SETTING 0xFD14

#define HCI_VENDOR_MP_XTAL_TRACK    0xFD17

#define EXTEND_WRITE_CONFIG_BDADDR  0x10
#define EXTEND_WRITE_CONFIG_XTALCAP 0x11
#define EXTEND_WRITE_CONFIG_TXGAINK 0x12
#define EXTEND_WRITE_CONFIG_TXFLNESS 0x13
#define EXTEND_WRITE_CONFIG_TXPOWER 0x14
#define EXTEND_WRITE_CONFIG_XTAL32k 0x15
#define EXTEND_ERASE_FLASH_CONFIG_PAGE 0x16
//#define EXTEND_WRITE_SET_HEADER_SIZE  0X17
#define EXTEND_WRITE_CONFIG_TXGAINK_Tmeter  0x17


#define EXTEND_WRITE_CONFIG_BUF_BDADDR   0x30
#define EXTEND_WRITE_CONFIG_BUF_XTALCAP  0x31
#define EXTEND_WRITE_CONFIG_BUF_TXGAINK  0x32
#define EXTEND_WRITE_CONFIG_BUF_TXFLNESS 0x33
#define EXTEND_WRITE_CONFIG_BUF_TXPOWER  0x34
#define EXTEND_WRITE_CONFIG_BUF_XTAL32k   0x35
#define EXTEND_ERASE_CONFIG_BUF_CLEAN       0x36
#define EXTEND_WRITE_CONFIG_BUF_MODULEK_TMETER  0x37
#define EXTEND_WRITE_CONFIG_BUF_WriteToDevice       0x39
#define EXTEND_WRITE_CONFIG_BUF_BLE2M_2402_2480   0x40
#define EXTEND_WRITE_CONFIG_BUF_LBT_SETTING       0x41
#define EXTEND_WRITE_CONFIG_BUF_PC_SETTING        0X42

#define EXTEND_XTAL_TRACK_EN_GET                       0x50
#define EXTEND_XTAL_TRACK_EN_SET                       0x51
#define EXTEND_XTAL_TRACK_TABLE_SET                    0x52
#define EXTEND_XTAL_TRACK_TABLE_GET                    0x53
#define EXTEND_XTAL_TRACK_EN_WRITE_CONFIG              0x54
#define EXTEND_XTAL_TRACK_EN_WRITE_CONFIG_BUF          0x55
#define EXTEND_XTAL_TRACK_TABLE_WRITE_CONFIG           0x56
#define EXTEND_XTAL_TRACK_TABLE_WRITE_CONFIG_BUF       0x57
//XTAL Track enable bit: 0x496[3]
//XTAL Track table: 0x112 ~ 0x12B ?Hbyte???????@26byte
#define D_EXTEND_CONFIG_XTAL_TRACK_EN                  0x0496
#define D_EXTEND_CONFIG_XTAL_TRACK_TABLE               0x0112



//--------------------------------------------------------------------
// BBPro c Cut // efuse address
#define C_EXTEND_CONFIG_TXPOWER         0x03DD
#define C_EXTEND_CONFIG_TXFLNESS        0X03FB
#define C_EXTEND_CONFIG_TXFLNESS_ENABLE 0x0419
#define C_EXTEND_CONFIG_BDADDR          0X0044
#define C_EXTEND_CONFIG_TXGAINK         0x03FA
#define C_EXTEND_CONFIG_XTALCAP         0X045B
#define C_EXTEND_CONFIG_XTAL32K         0X045D
#define C_EXTEND_CONFIG_TXGAINK_ENABLE  0x0419
#define C_EXTEND_CONFIG_TxGainK_TMETER  0x041D  //NO DEFINE FLOW TXGAINK

#define D_EXTEND_CONFIG_BLE2M_2402_2480 0x045B
#define C_EXTEND_CONFIG_BLE2M_2402_2480 0x03E2

#define D_EXTEND_CONFIG_LBT_EN_SETTING  0x0496
#define C_EXTEND_CONFIG_LBT_EN_SETTING  0x0419
#define D_EXTEND_CONFIG_LBT_ANT_GAIN_SETTING    0x0498
#define C_EXTEND_CONFIG_LBT_ANT_GAIN_SETTING    0x041B

#define D_EXTEND_CONFIG_PC_SETTING  0x0463
#define C_EXTEND_CONFIG_PC_SETTING  0x03E8

 
#define D_EXTEND_CONFIG_TXPOWER         0x0456
#define D_EXTEND_CONFIG_TXFLNESS        0X0476
#define D_EXTEND_CONFIG_TXFLNESS_ENABLE 0x0496   //BIT1
#define D_EXTEND_CONFIG_BDADDR          0X0044
#define D_EXTEND_CONFIG_TXGAINK         0x0475
#define D_EXTEND_CONFIG_XTALCAP         0X04DB
#define D_EXTEND_CONFIG_XTAL32K         0X04DD
#define D_EXTEND_CONFIG_TXGAINK_ENABLE  0x0496
#define D_EXTEND_CONFIG_TxGainK_TMETER  0x049D


int
BTDevice_Extend_TxGainKValue(
    BT_DEVICE *pBtDevice,
    unsigned char *pData
    );

int
BTDevice_Extend_TxFlatnessValue(
    BT_DEVICE *pBtDevice,
    unsigned char *pData
    );
int
BTDevice_Extend_TxPathLossModule(
    BT_DEVICE *pBtDevice,
    unsigned char *pData
    );
int
BTDevice_Extend_ConfigExtend_Function(
    BT_DEVICE *pBtDevice,
    unsigned char *pData
    );


int
BTDevice_Extend_ReportTxInfo(
    BT_DEVICE *pBtDevice,
    unsigned char *pData
    );

int
BTDevice_Extend_XTAL_TRACK_Func(
    BT_DEVICE *pBtDevice,
    unsigned char  SubIndex,
    unsigned char *pData
    );





#endif
