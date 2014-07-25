#ifndef __BT_MP_BASE_H
#define __BT_MP_BASE_H

#include <stdio.h>
#include <string.h>

#include "foundation.h"

#define USE_CHAR_STR

//#define BUILDER_C

//-----------------------------------------------------------------------------------------------------------------
//  Base  define
//-----------------------------------------------------------------------------------------------------------------
#ifdef BUILDER_C

typedef
enum _bool{ false, true }bool;

#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

//--------------------------------------------------
//  Realtek define
//--------------------------------------------------
#define MAX_USERAWDATA_SIZE     64
#define BT_EFUSE                1
#define SYS_EFUSE               2

#define MAX_HCI_COMANND_BUF_SIZ 256
#define MAX_HCI_EVENT_BUF_SIZ   256
#define MAX_TXGAIN_TABLE_SIZE   7
#define MAX_TXDAC_TABLE_SIZE    5
#define SEC_CLOCK_NUMBER        3200  // 1 clock =312.5u sec  1sec = 3200 

typedef enum {
    MD_REG = 0,
    RF_REG,
    SYS_REG,
    BB_REG,
} BT_REG_TYPE;

typedef enum {
    NOTTHING = 0,                       //0
    //Module Select Process
    MODULE_INIT,                        //1
    //set table
    SETTXGAINTABLE,                     //2
    SETTXDACTABLE,                      //3
    //Device Setting
    HCI_RESET,                          //4
    SET_TXCHANNEL,                      //5
    SET_RXCHANNEL,                      //6
    SET_LETXCHANNEL,                    //7
    SET_POWERGAININDEX,                 //8
    SET_POWERGAIN,                      //9
    SET_POWERDAC,                       //10
    SET_PAYLOADTYPE,                    //11
    SET_WHITENINGCOFF,                  //12
    SET_PACKETTYPE,                     //13
    SET_HITTARGET,                      //14
    SET_TESTMODE,                       //15
    SET_MUTIRXENABLE,                   //16
    //Module Control
    PACKET_TX_START,                    //17
    PACKET_TX_START_SET_CHANNEL_PKTTYPE,//18
    PACKET_TX_UPDATE,                   //19
    PACKET_TX_SEND_ONE,                 //20
    PACKET_TX_STOP,                     //21

    PACKET_RX_START,                    //22
    PACKET_RX_START_SET_CHANNEL_PKTTYPE,//23
    PACKET_RX_UPDATE,                   //24
    PACKET_RX_STOP,                     //25

    CONTINUE_TX_START,                  //26
    CONTINUE_TX_UPDATE,                 //27
    CONTINUE_TX_STOP,                   //28

    CONTINUE_TX_LE_START,               //29
    CONTINUE_TX_LE_UPDATE,              //30
    CONTINUE_TX_LE_STOP,                //31
    HOPPING_DWELL_TIME,                 //32
    //Report
    REPORT_CLEAR,                       //33

    TEST_MODE_ENABLE,                   //34
    SET_PACKET_HEADER,                  //35
    SET_DEFAULT_TX_GAIN_TABLE,          //36
    SET_DEFAULT_TX_DAC_TABLE,           //37
    SET_RTL8761_XTAL,                   //38
    // Efuse setting
    EXEC_USE_RAWDATA,                   //39
    // LE
    LE_TX_DUT_TEST_CMD,                 //40
    LE_RX_DUT_TEST_CMD,                 //41
    LE_DUT_TEST_END_CMD,                //42

    NUMBEROFBT_ACTIONCONTROL_TAG
} BT_ACTIONCONTROL_TAG;

typedef enum {
    BT_PAYLOAD_TYPE_ALL0 = 0,
    BT_PAYLOAD_TYPE_ALL1 = 1,
    BT_PAYLOAD_TYPE_0101 = 2,
    BT_PAYLOAD_TYPE_1010 = 3,
    BT_PAYLOAD_TYPE_0x0_0xF = 4,
    BT_PAYLOAD_TYPE_0000_1111 = 5,
    BT_PAYLOAD_TYPE_1111_0000 = 6,
    BT_PAYLOAD_TYPE_PRBS9 = 7,

    BT_PAYLOAD_TYPE_NUM = 8
} BT_PAYLOAD_TYPE;



typedef enum {
    BT_PKT_1DH1 = 0,
    BT_PKT_1DH3,
    BT_PKT_1DH5,
    BT_PKT_2DH1,
    BT_PKT_2DH3,
    BT_PKT_2DH5,
    BT_PKT_3DH1,
    BT_PKT_3DH3,
    BT_PKT_3DH5,
    BT_PKT_LE,

    BT_PKT_TYPE_NULL,
    BT_PKT_TYPE_RTL8723A,
    BT_PKT_TYPE_NUM
} BT_PKT_TYPE;

typedef enum {
    BT_DUT_MODE = 0,
    BT_PSEUDO_MODE =1,

    NUMBEROFBT_TEST_MODE
} BT_TEST_MODE;

typedef struct BT_PARAMETER_TAG   BT_PARAMETER;
typedef struct BT_DEVICE_REPORT_TAG BT_DEVICE_REPORT;
typedef struct BT_CHIPINFO_TAG   BT_CHIPINFO;

struct BT_PARAMETER_TAG {
    int ParameterIndex;

    uint8_t mPGRawData[MAX_USERAWDATA_SIZE];
    uint8_t mParamData[MAX_USERAWDATA_SIZE];
    uint8_t mChannelNumber;
    BT_PKT_TYPE mPacketType;
    BT_PAYLOAD_TYPE mPayloadType;
    uint16_t mTxPacketCount;
    uint8_t mTxGainValue;
    uint8_t mWhiteningCoeffValue;
    uint8_t mTxGainIndex;
    uint8_t mTxDAC;
    uint16_t mPacketHeader;
    uint8_t mHoppingFixChannel;
    uint64_t mHitTarget;
    uint8_t TXGainTable[MAX_TXGAIN_TABLE_SIZE];
    uint8_t TXDACTable[MAX_TXDAC_TABLE_SIZE];
    uint32_t Rtl8761Xtal;
};

struct BT_CHIPINFO_TAG {
    uint32_t HCI_Version;
    uint32_t HCI_SubVersion;
    uint32_t LMP_Version;
    uint32_t LMP_SubVersion;

    uint32_t ChipType;
    uint32_t Version;
    int Is_After_PatchCode;
};

struct BT_DEVICE_REPORT_TAG {
    uint32_t TotalTXBits;
    uint32_t TotalTxCounts;

    uint32_t RXRecvPktCnts;
    uint32_t TotalRXBits;
    uint32_t TotalRxCounts;
    uint32_t TotalRxErrorBits;
    int      RxRssi;
    float    ber;
    float    Cfo;

    uint8_t       CurrTXGainTable[MAX_TXGAIN_TABLE_SIZE];
    uint8_t       CurrTXDACTable[MAX_TXDAC_TABLE_SIZE];

    uint8_t       CurrThermalValue;
    uint32_t      CurrRtl8761Xtal;
    uint8_t       CurrStage;

    BT_CHIPINFO   *pBTInfo;
    BT_CHIPINFO   BTInfoMemory;
    uint8_t ReportData[MAX_USERAWDATA_SIZE];
};

enum RTK_BT_CHIP_ID_GROUP {
    RTK_BT_CHIP_ID_UNKNOWCHIP=0xFF,
    RTK_BT_CHIP_ID_RTL8723A=0,
    RTK_BT_CHIP_ID_RTL8723B=1,
    RTK_BT_CHIP_ID_RTL8821A=2,
    RTK_BT_CHIP_ID_RTL8761A=3,

    NumOfRTKCHID
};

//-----------------------------------------------------------------------------------------------------------------
//  Device
//-----------------------------------------------------------------------------------------------------------------
//Device Level::member Funcion
//-----------------------------------------------------------------------------------------------------------------
typedef struct BT_DEVICE_TAG   BT_DEVICE;

//-->Table
typedef int
(*BT_FP_SET_TXGAINTABLE)(
        BT_DEVICE *pBtDevice,
        uint8_t *pTable
        );

typedef int
(*BT_FP_SET_TXDACTABLE)(
        BT_DEVICE *pBtDevice,
        uint8_t *pTable
        );

typedef int
(*BT_FP_GET_PAYLOADLENTABLE)(
        BT_DEVICE *pBtDevice,
        uint8_t *pTable,
        int length
        );

//-->HCI Command & Event
typedef int
(*BT_FP_SEND_HCICOMMANDWITHEVENT)(
        BT_DEVICE *pBtDevice,
        uint16_t OpCode,
        uint8_t PayLoadLength,
        uint8_t *pPayLoad,
        uint8_t EventType,
        uint8_t *pEvent,
        uint32_t *pEventLen
        );

typedef int
(*BT_FP_RECV_ANYEVENT)(
        BT_DEVICE *pBtDevice,
        uint8_t *pEvent
        );

typedef int
(*BT_FB_SEND_HCI_CMD)(
        BT_DEVICE *pBt,
        uint8_t PktType,
        uint8_t *pWritingBuf,
        uint32_t Len
        );

typedef int
(*BT_FB_RECV_HCI_EVENT)(
        BT_DEVICE *pBt,
        uint8_t PktType,
        uint8_t *pReadingBuf,
        uint32_t *pLen
        );

//-->Vendor HCI Command Control
typedef int
(*BT_FP_SET_HOPPINGMODE)(
        BT_DEVICE *pBtDevice,
        uint8_t ChannelNumber,
        BT_PKT_TYPE PktType,
        BT_PAYLOAD_TYPE PayloadType,
        uint8_t TxGainValue,
        uint8_t WhiteningCoeffValue,
        uint8_t TxGainIndex,
        uint8_t TxDAC,
        uint8_t HoppingFixChannel
        );

typedef int
(*BT_FP_SET_FWPOWERTRACKENABLE)(
        BT_DEVICE *pBtDevice,
        uint8_t FWPowerTrackEnable
        );

typedef int
(*BT_FP_SET_HCIRESET)(
        BT_DEVICE *pBtDevice,
        int Delay_mSec
        );

typedef int
(*BT_FP_GET_BT_CLOCK_TIME)(
        BT_DEVICE *pBtDevice,
        unsigned long *btClockTime
        );

//----------------------------------------------------------------
typedef int
(*BT_FP_SET_TEST_MODE_ENABLE)(
        BT_DEVICE *pBtDevice
        );

typedef int
(*BT_FP_SET_RTL8761_XTAL)(
        BT_DEVICE *pBtDevice,
        uint32_t Value
        );

typedef int
(*BT_FP_GET_RTL8761_XTAL)(
        BT_DEVICE *pBtDevice,
        uint32_t *pValue
        );

typedef int
(*BT_FP_READ_THERMAL)(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam,
        uint8_t *pThermalValue
        );

typedef int
(*BT_FP_GET_STAGE)(
        BT_DEVICE *pBtDevice,
        uint8_t *pStage
        );

typedef int
(*BT_FP_SET_SYS_REG_MASK_BITS)(
        BT_DEVICE *pBtDevice,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint32_t UserValue
        );

typedef int
(*BT_FP_GET_SYS_REG_MASK_BITS)(
        BT_DEVICE *pBtDevice,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint32_t *pUserValue
        );

typedef int
(*BT_FP_SET_BB_REG_MASK_BITS)(
        BT_DEVICE *pBtDevice,
        int Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint32_t UserValue
        );

typedef int
(*BT_FP_GET_BB_REG_MASK_BITS)(
        BT_DEVICE *pBtDevice,
        int Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint32_t *pUserValue
        );

typedef int
(*BT_FP_SET_PESUDOOUTERSETUP)(
        BT_DEVICE *pBtDevice,
        BT_PARAMETER *pParam
        );

typedef int
(*BT_FP_SET_SETPACKHEADER)(
        BT_DEVICE *pBtDevice,
        unsigned int packHeader
        );

typedef int
(*BT_FP_SET_SETRESETMDCOUNT)(
        BT_DEVICE *pBtDevice
        );

typedef int
(*BT_FP_SET_POWERGAININDEX)(
        BT_DEVICE *pBtDevice,
        int Index
        );

typedef int
(*BT_FP_SET_MUTIRXENABLE)(
        BT_DEVICE *pBtDevice,
        int IsMultiPktRx
        );

typedef int
(*BT_FP_SET_TESTMODE)(
        BT_DEVICE *pBtDevice,
        BT_TEST_MODE TestMode
        );

typedef int
(*BT_FP_SET_HITTARGET)(
        BT_DEVICE *pBtDevice,
        uint64_t HitTarget
        );

typedef int
(*BT_FP_SET_PACKETTYPE)(
        BT_DEVICE *pBtDevice,
        BT_PKT_TYPE PktType
        );

typedef int
(*BT_FP_SET_WHITENINGCOFF)(
        BT_DEVICE *pBtDevice,
        uint8_t WhiteningCoeffValue
        );

typedef int
(*BT_FP_SET_PAYLOADTYPE)(
        BT_DEVICE *pBtDevice,
        BT_PAYLOAD_TYPE PayloadType
        );

typedef int
(*BT_FP_SET_TXCHANNEL)(
        BT_DEVICE *pBtDevice,
        uint8_t ChannelNumber
        );

typedef int
(*BT_FP_SET_RXCHANNEL)(
        BT_DEVICE *pBtDevice,
        uint8_t ChannelNumber
        );

typedef int
(*BT_FP_SET_POWERGAIN)(
        BT_DEVICE *pBtDevice,
        uint8_t PowerGainValue
        );

typedef int
(*BT_FP_SET_POWERDAC)(
        BT_DEVICE *pBtDevice,
        uint8_t DacValue
        );

//------------------------------------------------------------------------------------------------------------------
//-->Register Read/Write
typedef int
(*BT_FP_SET_MD_REG_MASK_BITS)(
        BT_DEVICE *pBtDevice,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        const uint16_t UserValue
        );

typedef int
(*BT_FP_GET_MD_REG_MASK_BITS)(
        BT_DEVICE *pBtDevice,
        uint8_t RegStartAddr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pReadingValue
        );

typedef int
(*BT_FP_SET_RF_REG_MASK_BITS)(
        BT_DEVICE *pBtDevice,
        uint8_t RegStartAddr,
        uint8_t Msb,
        uint8_t Lsb,
        const uint16_t WritingValue
        );

typedef int
(*BT_FP_GET_RF_REG_MASK_BITS)(
        BT_DEVICE *pBtDevice,
        uint8_t RegStartAddr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pReadingValue
        );

//------------------------------------------------------------------------------------------------------------------
//-->BASE
typedef int
(*BT_BASE_FP_HITTARGETACCRESSCODEGEN)(
        BT_DEVICE *pBtDevice,
        uint64_t HitTarget,
        unsigned long *pAccessCode
        );

typedef int
(*BT_BASE_FP_GETPAYLOADTYPEVAILDFLAG)(
        BT_DEVICE *pBtDevice,
        BT_TEST_MODE TestMode,
        BT_PKT_TYPE PKT_TYPE,
        unsigned int *ValidFlag
        );

//------------------------------------------------------------------------------------------------------------------
//-->Control Flow
typedef int(*BT_FP_SET_CONTINUETX_BEGIN)(BT_DEVICE *pBtDevice,BT_PARAMETER *pParam,BT_DEVICE_REPORT *pBtReport);
typedef int(*BT_FP_SET_CONTINUETX_STOP)(BT_DEVICE *pBtDevice,BT_PARAMETER *pParam,BT_DEVICE_REPORT *pBtReport);
typedef int(*BT_FP_SET_CONTINUETX_UPDATE)(BT_DEVICE *pBtDevice,BT_PARAMETER *pParam,BT_DEVICE_REPORT *pBtReport);
//-->PKT TX Flow
typedef int(*BT_FP_SET_PKTTX_BEGIN)(BT_DEVICE *pBtDevice,BT_PARAMETER *pParam,BT_DEVICE_REPORT *pBtReport);
typedef int(*BT_FP_SET_PKTTX_STOP)(BT_DEVICE *pBtDevice,BT_PARAMETER *pParam,BT_DEVICE_REPORT *pBtReport);
typedef int(*BT_FP_SET_PKTTX_UPDATE)(BT_DEVICE *pBtDevice,BT_PARAMETER *pParam,BT_DEVICE_REPORT *pBtReport);
//-->PKT RX Flow
typedef int(*BT_FP_SET_PKTRX_BEGIN)(BT_DEVICE *pBtDevice,BT_PARAMETER *pParam,BT_DEVICE_REPORT *pBtReport);
typedef int(*BT_FP_SET_PKTRX_STOP)(BT_DEVICE *pBtDevice,BT_PARAMETER *pParam,BT_DEVICE_REPORT *pBtReport);
typedef int(*BT_FP_SET_PKTRX_UPDATE)(BT_DEVICE *pBtDevice,BT_PARAMETER *pParam,BT_DEVICE_REPORT *pBtReport);

typedef int(*BT_FP_GET_CHIPID)(BT_DEVICE *pBtDevice);
typedef int(*BT_FP_GET_ECOVERSION)(BT_DEVICE *pBtDevice);
typedef int(*BT_FP_GET_CHIPVERSIONINFO)(BT_DEVICE *pBtDevice);
typedef int(*BT_FP_BT_DL_FW)(BT_DEVICE *pBtDevice, uint8_t *pPatchcode, int patchLength);
typedef int(*BT_FP_BT_DL_MERGER_FW)(BT_DEVICE *pBtDevice, uint8_t *pPatchcode, int patchLength);
// PG efuse
typedef int(*BT_FP_SF_PGEFUSE_RAWDATA)(BT_DEVICE *pBtDevice, int MapType, uint8_t *PGData, uint32_t PGDataLength);
// LE
typedef int(*BT_FP_LE_TEST)(BT_DEVICE *pBtDevice, BT_PARAMETER *pParam, BT_DEVICE_REPORT *pBtReport);

typedef struct BT_TRX_TIME_TAG BT_TRX_TIME;

typedef enum {
        TRX_TIME_STOP =0,
        TX_TIME_RUNING ,
        RX_TIME_RUNING,
        //////////////
        NUMOFTRXTIME_TAG
} TRXTIME_TAG;

struct BT_TRX_TIME_TAG {
    unsigned long beginTimeClockCnt;
    unsigned long UseTimeClockCnt;
    unsigned long endTimeClockCnt;
};

struct BT_DEVICE_TAG
{
    //-->Table is base function
    uint8_t TXGainTable[MAX_TXGAIN_TABLE_SIZE];
    uint8_t TXDACTable[MAX_TXDAC_TABLE_SIZE];
    BT_FP_SET_TXGAINTABLE       SetTxGainTable;
    BT_FP_SET_TXDACTABLE        SetTxDACTable;
    BT_FP_GET_PAYLOADLENTABLE   GetPayloadLenTable;

    //-->Register Read/Write
    BT_FP_SET_MD_REG_MASK_BITS  SetMdRegMaskBits;
    BT_FP_GET_MD_REG_MASK_BITS  GetMdRegMaskBits;

    BT_FP_SET_RF_REG_MASK_BITS  SetRfRegMaskBits;
    BT_FP_GET_RF_REG_MASK_BITS  GetRfRegMaskBits;

    //-->HCI Command & Event
    BT_FP_SEND_HCICOMMANDWITHEVENT  SendHciCommandWithEvent;
    BT_FP_RECV_ANYEVENT RecvAnyHciEvent;

    //-->HCI command raw data
    BT_FB_SEND_HCI_CMD      SendHciCmd;
    BT_FB_RECV_HCI_EVENT    RecvHciEvent;

    //Device member
    //-->Register Control
    BT_FP_SET_TXCHANNEL     SetTxChannel;
    BT_FP_SET_TXCHANNEL     SetLETxChannel;
    BT_FP_SET_RXCHANNEL     SetRxChannel;
    BT_FP_SET_POWERGAININDEX    SetPowerGainIndex;
    BT_FP_SET_POWERGAIN         SetPowerGain;
    BT_FP_SET_POWERDAC          SetPowerDac;
    BT_FP_SET_PAYLOADTYPE       SetPayloadType;
    BT_FP_SET_WHITENINGCOFF     SetWhiteningCoeff;
    BT_FP_SET_PACKETTYPE        SetPacketType;
    BT_FP_SET_HITTARGET         SetHitTarget;
    BT_FP_SET_TESTMODE          SetTestMode;
    BT_FP_SET_MUTIRXENABLE      SetMutiRxEnable;
    BT_FP_SET_SETRESETMDCOUNT   SetRestMDCount;
    BT_FP_SET_SETPACKHEADER     SetPackHeader;
    BT_FP_SET_PESUDOOUTERSETUP  SetPesudoOuterSetup;
    BT_FP_SET_TEST_MODE_ENABLE TestModeEnable;

    BT_FP_SET_RTL8761_XTAL SetRtl8761Xtal;
    BT_FP_GET_RTL8761_XTAL GetRtl8761Xtal;
    BT_FP_READ_THERMAL  ReadThermal;

    BT_FP_GET_STAGE                 GetStage;
    BT_FP_SET_SYS_REG_MASK_BITS SetSysRegMaskBits;
    BT_FP_GET_SYS_REG_MASK_BITS GetSysRegMaskBits;
    BT_FP_SET_BB_REG_MASK_BITS      SetBBRegMaskBits;
    BT_FP_GET_BB_REG_MASK_BITS      GetBBRegMaskBits;

    //-->Vendor HCI Command Control
    BT_FP_SET_FWPOWERTRACKENABLE    SetFWPowerTrackEnable;
    BT_FP_SET_HOPPINGMODE   SetHoppingMode;
    BT_FP_SET_HCIRESET  SetHciReset;
    BT_FP_GET_BT_CLOCK_TIME GetBTClockTime;

    unsigned long TxTriggerPktCnt;

    //0:TRX STOP  1 : is TX 2: IS rx
    //Con-TX
    BT_TRX_TIME                     TRxTime[NUMOFTRXTIME_TAG];
    BT_FP_SET_CONTINUETX_BEGIN      SetContinueTxBegin;
    BT_FP_SET_CONTINUETX_STOP       SetContinueTxStop;
    BT_FP_SET_CONTINUETX_UPDATE     SetContinueTxUpdate;
    // LE
    BT_FP_LE_TEST LeTxTestCmd;
    BT_FP_LE_TEST LeRxTestCmd;
    BT_FP_LE_TEST LeTestEndCmd;
    //PKT-TX
    BT_FP_SET_PKTTX_BEGIN           SetPktTxBegin;
    BT_FP_SET_PKTTX_STOP            SetPktTxStop;
    BT_FP_SET_PKTTX_UPDATE          SetPktTxUpdate;
    //PKT-RX
    BT_FP_SET_PKTRX_BEGIN           SetPktRxBegin;
    BT_FP_SET_PKTRX_STOP            SetPktRxStop;
    BT_FP_SET_PKTRX_UPDATE          SetPktRxUpdate;
    //Base Function
    BT_BASE_FP_GETPAYLOADTYPEVAILDFLAG  GetPayLoadTypeValidFlag;
    BT_BASE_FP_HITTARGETACCRESSCODEGEN  HitTargetAccessCodeGen;

    //interface
    void *pExtra;
    uint8_t InterfaceType;

    BASE_INTERFACE_MODULE *pBaseInterface;

    BT_CHIPINFO *pBTInfo;
    BT_CHIPINFO BaseBTInfoMemory;
    BT_FP_GET_CHIPID        GetChipId;
    BT_FP_GET_ECOVERSION    GetECOVersion;
    BT_FP_GET_CHIPVERSIONINFO GetChipVersionInfo;
    BT_FP_BT_DL_FW BTDlFW;
    BT_FP_BT_DL_MERGER_FW BTDlMERGERFW;
    // Efuse settting
    BT_FP_SF_PGEFUSE_RAWDATA    BT_SP_PGEfuseRawData;
};

//-----------------------------------------------------------------------------------------------------------------
//  Module
//-----------------------------------------------------------------------------------------------------------------
typedef enum _BT_REPORT_TAG {
    NO_THING,
    REPORT_PKT_TX,
    REPORT_RX,
    REPORT_CHIP,
    REPORT_ALL,
    REPORT_TX_GAIN_TABLE,
    REPORT_TX_DAC_TABLE,
    REPORT_RTL8761_XTAL,
    REPORT_THERMAL,
    REPORT_BT_STAGE,
    REPORT_CONT_TX
} BT_REPORT_TAG;

typedef struct  BT_MODULE_TAG BT_MODULE;

//-->module
typedef int
(*BT_MODULE_FP_ACTION_REPORT)(
        BT_MODULE *pBtModule,
        int ActiceItem,
        BT_DEVICE_REPORT *pReport
        );

typedef int
(*BT_MODULE_FP_UPDATA_PARAMETER) (
    BT_MODULE *pBtModule,
    BT_PARAMETER *pParam
    );

typedef int
(*BT_MODULE_FP_ACTION_CONTROLEXCUTE) (
        BT_MODULE *pBtModule
        );

typedef int
(*BT_MODULE_FP_ACTION_DLFW)(
        BT_MODULE *pBtModule,
        uint8_t *pPatchcode,
        int patchLength,
        int Mode
        );

//-->Register Read/Write
typedef int
(*BT_MODULE_FP_SET_MD_REG_MASK_BITS)(
        BT_MODULE *pBtModule,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        const uint16_t UserValue
        );

typedef int
(*BT_MODULE_FP_GET_MD_REG_MASK_BITS)(
        BT_MODULE *pBtModule,
        uint8_t RegStartAddr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pReadingValue
        );

typedef int
(*BT_MODULE_FP_SET_RF_REG_MASK_BITS)(
        BT_MODULE *pBtModule,
        uint8_t RegStartAddr,
        uint8_t Msb,
        uint8_t Lsb,
        const uint16_t WritingValue
        );

typedef int
(*BT_MODULE_FP_GET_RF_REG_MASK_BITS)(
        BT_MODULE *pBtModule,
        uint8_t RegStartAddr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pReadingValue
        );

//-->HCI Command & Event
typedef int
(*BT_MODULE_FP_SEND_HCICOMMANDWITHEVENT)(
        BT_MODULE *pBtModule,
        uint16_t OpCode,
        uint8_t PayLoadLength,
        uint8_t *pPayLoad,
        uint8_t EventType,
        uint8_t *pEvent,
        uint32_t *pEventLen
        );

typedef int
(*BT_MODULE_FP_RECV_ANYEVENT)(
        BT_MODULE *pBtModule,
        uint8_t *pEvent
        );

typedef int
(*BT_MODULE_FP_SET_SYS_REG_MASK_BITS)(
        BT_MODULE *pBtModule,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint32_t UserValue
        );

typedef int
(*BT_MODULE_FP_GET_SYS_REG_MASK_BITS)(
        BT_MODULE *pBtModule,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint32_t *pUserValue
        );

typedef int
(*BT_MODULE_FP_SET_BB_REG_MASK_BITS)(
        BT_MODULE *pBtModule,
        uint8_t Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        const uint32_t UserValue
        );

typedef int
(*BT_MODULE_FP_GET_BB_REG_MASK_BITS)(
        BT_MODULE *pBtModule,
        uint8_t Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint32_t *pUserValue
        );

typedef int
(*BT_MODULE_FP_SET_REG_MASK_BITS)(
        BT_MODULE *pBtModule,
        uint8_t Type,
        uint8_t Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        const uint32_t UserValue
        );

typedef int
(*BT_MODULE_FP_GET_REG_MASK_BITS)(
        BT_MODULE *pBtModule,
        uint8_t Type,
        uint8_t Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint32_t *pUserValue
        );

struct BT_MODULE_TAG {
    BT_PARAMETER        *pBtParam;
    BT_DEVICE           *pBtDevice;
    BT_DEVICE_REPORT    *pModuleBtReport;

    BT_PARAMETER        BaseBtParamMemory;
    BT_DEVICE           BaseBtDeviceMemory;
    BT_DEVICE_REPORT    BaseModuleBtReportMemory;

    //Module Function
    BT_MODULE_FP_UPDATA_PARAMETER           UpDataParameter;
    BT_MODULE_FP_ACTION_CONTROLEXCUTE       ActionControlExcute;
    BT_MODULE_FP_ACTION_REPORT              ActionReport;

    BT_MODULE_FP_ACTION_DLFW                DownloadPatchCode;

    //register read/write and hci command
    BT_MODULE_FP_SET_MD_REG_MASK_BITS       SetMdRegMaskBits;
    BT_MODULE_FP_GET_MD_REG_MASK_BITS       GetMdRegMaskBits;

    BT_MODULE_FP_SET_RF_REG_MASK_BITS       SetRfRegMaskBits;
    BT_MODULE_FP_GET_RF_REG_MASK_BITS       GetRfRegMaskBits;

    BT_MODULE_FP_SET_SYS_REG_MASK_BITS      SetSysRegMaskBits;
    BT_MODULE_FP_GET_SYS_REG_MASK_BITS      GetSysRegMaskBits;

    BT_MODULE_FP_SET_BB_REG_MASK_BITS       SetBBRegMaskBits;
    BT_MODULE_FP_GET_BB_REG_MASK_BITS       GetBBRegMaskBits;

    BT_MODULE_FP_SET_REG_MASK_BITS          SetRegMaskBits;
    BT_MODULE_FP_GET_REG_MASK_BITS          GetRegMaskBits;

    //-->HCI Command & Event
    BT_MODULE_FP_SEND_HCICOMMANDWITHEVENT   SendHciCommandWithEvent;
    BT_MODULE_FP_RECV_ANYEVENT              RecvAnyHciEvent;
};

//-----------------------------------------------------------------------------------------------------
//  Base Function
//-----------------------------------------------------------------------------------------------------

// Constants
#define INVALID_POINTER_VALUE       0
#define NO_USE                      0

#define LEN_1_BYTE                  1
#define LEN_2_BYTE                  2
#define LEN_3_BYTE                  3
#define LEN_4_BYTE                  4
#define LEN_5_BYTE                  5
#define LEN_6_BYTE                  6
#define LEN_7_BYTE                  7
#define LEN_11_BYTE                 11
#define LEN_14_BYTE                 14
#define LEN_16_BYTE                 16

#define LEN_1_BIT                   1

#define BYTE_MASK                   0xff
#define BYTE_SHIFT                  8
#define HEX_DIGIT_MASK              0xf
#define BYTE_BIT_NUM                8
#define LONG_BIT_NUM                32

#define BIT_0_MASK                  0x1
#define BIT_1_MASK                  0x2
#define BIT_2_MASK                  0x4
#define BIT_3_MASK                  0x8
#define BIT_4_MASK                  0x10
#define BIT_5_MASK                  0x20
#define BIT_6_MASK                  0x40
#define BIT_7_MASK                  0x80
#define BIT_8_MASK                  0x100

#define BIT_7_SHIFT                 7
#define BIT_8_SHIFT                 8

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

//  Define UART HCI Packet Indicator
#define IF_UART_CMD     0x01
#define IF_UART_ACL     0x02
#define IF_UART_SCO     0x03
#define IF_UART_EVT     0x04

//  Define HCI IO
#define HCIIO_EFUSE         0x00
#define HCIIO_BTRFREG       0x01
#define HCIIO_BTMODEMREG    0x02
#define HCIIO_BTCMD         0x03
#define HCIIO_BTEVT         0x04
#define HCIIO_BTACLIN       0x05
#define HCIIO_BTACLOUT      0x06
#define HCIIO_BTSCOIN       0x07
#define HCIIO_BTSCOOUT      0x08
#define HCIIO_PATCHCODE     0x09
#define HCIIO_UART_H5       0x0A

//  Patch code download size
#define PATCHCODE_DOWNLOAD_SIZE     0xFC

#define RESET_DEFAULTVALUE          0x00

//Define BTHCI Debug mode
#define BTHCI_DEBUG         1

enum BT_HCI_EVENT_FIELD {
    EVT_CODE = 0,
    EVT_PARA_LEN,
    EVT_HCI_CMD_NUM,
    EVT_CMD_OPCODE_0,
    EVT_CMD_OPCODE_1,
    EVT_STATUS,
    EVT_BYTE0,
    EVT_BYTE1,
    EVT_BYTE2,
    EVT_BYTE3,
};

//member function
int
bt_default_SetMDRegMaskBits(
        BT_DEVICE *pBtDevice,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        );

int
bt_default_GetMDRegMaskBits(
        BT_DEVICE *pBt,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        );

int
bt_default_GetRFRegMaskBits(
        BT_DEVICE *pBt,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        );

int
bt_default_SetRFRegMaskBits(
        BT_DEVICE *pBt,
        uint8_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        );

int
bt_default_GetBytes(
        BT_DEVICE *pBt,
        uint8_t Addr,
        uint32_t *pReadingValue,
        uint8_t *pEvtCode,
        uint32_t *pEvtCodeLen
        );

int
bt_default_SetBytes(
        BT_DEVICE *pBt,
        uint8_t Addr,
        uint32_t WritingValue,
        uint8_t *pEvtCode,
        uint32_t *pEvtCodeLen
        );

int
bt_default_RecvHCIEvent(
        BT_DEVICE *pBt,
        uint8_t PktType,
        uint8_t *pReadingBuf,
        uint32_t *pLen
        );

int
bt_default_SendHCICmd(
        BT_DEVICE *pBt,
        uint8_t PktType,
        uint8_t *pWritingBuf,
        uint32_t Len
        );

int
bt_uart_Recv(
        BT_DEVICE *pBt,
        uint8_t PktType,
        uint8_t *pReadingBuf,
        uint32_t *pLen
        );

int
bt_uart_Send(
        BT_DEVICE *pBt,
        uint8_t PktType,
        uint8_t *pWritingBuf,
        uint32_t Len
        );

int
bt_Recv(
        BT_DEVICE *pBt,
        uint8_t PktType,
        uint8_t *pReadingBuf,
        uint32_t *pLen
        );

int
bt_Send(
        BT_DEVICE *pBt,
        uint8_t PktType,
        uint8_t *pWritingBuf,
        uint32_t Len
        );

int
BT_SetSysRegMaskBits(
        BT_DEVICE *pBtDevice,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint32_t UserValue
        );

int
BT_GetSysRegMaskBits(
        BT_DEVICE *pBtDevice,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint32_t *pUserValue
        );

int
BT_SetBBRegMaskBits(
        BT_DEVICE *pBtDevice,
        int Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint32_t UserValue
        );

int
BT_GetBBRegMaskBits(
        BT_DEVICE *pBtDevice,
        int Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint32_t *pUserValue
        );

void
BTHCI_EvtReport(
        uint8_t *pEvtCode,
        uint32_t EvtCodeLen
        );

int
bt_default_GetChipId(
        BT_DEVICE *pBtDevice
        );

int
bt_default_GetECOVersion(
        BT_DEVICE *pBtDevice
        );

int
bt_default_GetBTChipVersionInfo(
        BT_DEVICE *pBtDevice
        );

int
bt_default_BTDlFW(
        BT_DEVICE *pBtDevice,
        uint8_t *pPatchcode,
        int patchLength
        );

int
bt_default_BTDlMergerFW(
        BT_DEVICE *pBtDevice,
        uint8_t *pPatchcode,
        int patchLength
        );

#endif
