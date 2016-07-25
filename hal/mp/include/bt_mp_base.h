#ifndef __BT_MP_BASE_H
#define __BT_MP_BASE_H

#include <stdio.h>
#include <string.h>

#include "foundation.h"

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

#define MAX_TXGAIN_TABLE_SIZE   7
#define MAX_TXDAC_TABLE_SIZE    5

#define EVT_HCI_VERSION         6
#define EVT_HCI_SUBVERSION      7
#define EVT_HCI_LMPVERSION      9
#define EVT_HCI_LMPSUBVERSION   12
#define EVT_CHIP_ECO_VERSION    6

typedef enum {
    MD_REG = 0,
    RF_REG,
    SYS_REG,
    BB_REG,
} BT_REG_TYPE;

typedef enum {
    HCI_RESET = 0,                  //0
    // test mode
    TEST_MODE_ENABLE,               //1
    // efuse setting
    WRITE_EFUSE_DATA,               //2
    // set table
    SET_TX_GAIN_TABLE,              //3
    SET_TX_DAC_TABLE,               //4
    SET_DEFAULT_TX_GAIN_TABLE,      //5
    SET_DEFAULT_TX_DAC_TABLE,       //6
    // set power
    SET_POWER_GAIN_INDEX,           //7
    SET_POWER_GAIN,                 //8
    SET_POWER_DAC,                  //9
    // set xtal
    SET_XTAL,                       //10
    // report clear
    REPORT_CLEAR,                   //11
    // pkt tx
    PACKET_TX_START,                //12
    PACKET_TX_UPDATE,               //13
    PACKET_TX_STOP,                 //14
    // continue tx
    CONTINUE_TX_START,              //15
    CONTINUE_TX_UPDATE,             //16
    CONTINUE_TX_STOP,               //17
    // pkt rx
    PACKET_RX_START,                //18
    PACKET_RX_UPDATE,               //19
    PACKET_RX_STOP,                 //20
    // hopping mode
    HOPPING_DWELL_TIME,             //21
    // LE
    LE_TX_DUT_TEST_CMD,             //22
    LE_RX_DUT_TEST_CMD,             //23
    LE_DUT_TEST_END_CMD,            //24

    READ_EFUSE_DATA,                //25

    SET_CONFIG_FILE_DATA,           //26
    CLEAR_CONFIG_FILE_DATA,         //27

    // LE Continue TX
    LE_CONTINUE_TX_START,           //28
    LE_CONTINUE_TX_STOP,            //29

    // FW pkt tx
    FW_PACKET_TX_START,             //30
    FW_PACKET_TX_STOP,              //31

    // FW pkt rx
    FW_PACKET_RX_START,             //32
    FW_PACKET_RX_STOP,              //33

    // FW continue tx
    FW_CONTINUE_TX_START,           //34
    FW_CONTINUE_TX_STOP,            //35

    //FW LE Continue TX
    FW_LE_CONTINUE_TX_START,        //36
    FW_LE_CONTINUE_TX_STOP,         //37

    FW_READ_TX_POWER_INFO,          //38

    BT_ACTION_NUM
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
    BT_LE_PAYLOAD_TYPE_PRBS9 = 0,
    BT_LE_PAYLOAD_TYPE_1111_0000 = 1,
    BT_LE_PAYLOAD_TYPE_1010 = 2,
    BT_LE_PAYLOAD_TYPE_PRBS15 = 3,
    BT_LE_PAYLOAD_TYPE_ALL1 = 4,
    BT_LE_PAYLOAD_TYPE_ALL0 = 5,
    BT_LE_PAYLOAD_TYPE_0000_1111 = 6,
    BT_LE_PAYLOAD_TYPE_0101 = 7,

} BT_LE_PAYLOAD_TYPE;

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

enum {
    SET_INDEX_BT_ADDR = 0,
    SET_INDEX_THERMAL,
    SET_INDEX_TX_POWER_DAC,
    SET_INDEX_XTAL,
    SET_INDEX_USB_VID_PID,
};

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
    uint32_t mPacketHeader;
    uint8_t mHoppingFixChannel;
    uint64_t mHitTarget;
    uint8_t TXGainTable[MAX_TXGAIN_TABLE_SIZE];
    uint8_t TXDACTable[MAX_TXDAC_TABLE_SIZE];
    uint16_t Rtl8761Xtal;
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

    uint8_t  CurrTXGainTable[MAX_TXGAIN_TABLE_SIZE];
    uint8_t  CurrTXDACTable[MAX_TXDAC_TABLE_SIZE];

    uint8_t  CurrThermalValue;
    uint8_t  CurrStage;
    uint16_t CurrRtl8761Xtal;

    BT_CHIPINFO *pBTInfo;
    BT_CHIPINFO BTInfoMemory;
    uint8_t ReportData[MAX_USERAWDATA_SIZE];
};

enum RTK_BT_CHIP_ID_GROUP {
    RTK_BT_CHIP_ID_UNKNOWCHIP=0xFF,
    RTK_BT_CHIP_ID_RTL8723A=0,
    RTK_BT_CHIP_ID_RTL8723B=1,
    RTK_BT_CHIP_ID_RTL8821A=2,
    RTK_BT_CHIP_ID_RTL8761A=3,
    RTK_BT_CHIP_ID_RTL8703A=4,
    RTK_BT_CHIP_ID_RTL8763A=5,
    RTK_BT_CHIP_ID_RTL8703B=6,
    RTK_BT_CHIP_ID_RTL8723C=7,
    RTK_BT_CHIP_ID_RTL8822B=8,
    RTK_BT_CHIP_ID_RTL8723D=9,
    RTK_BT_CHIP_ID_RTL8821C=10,

    NumOfRTKCHID
};

typedef struct BT_DEVICE_TAG BT_DEVICE;

// Table
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

// HCI Command & Event
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

// Vendor HCI Command Control
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
(*BT_FP_SET_HCIRESET)(
        BT_DEVICE *pBtDevice,
        int Delay_mSec
        );


typedef int
(*BT_FP_SET_TEST_MODE_ENABLE)(
        BT_DEVICE *pBtDevice
        );

typedef int
(*BT_FP_SET_RTL8761_XTAL)(
        BT_DEVICE *pBtDevice,
        uint16_t Value
        );

typedef int
(*BT_FP_GET_RTL8761_XTAL)(
        BT_DEVICE *pBtDevice,
        uint16_t *pValue
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
        uint16_t UserValue
        );

typedef int
(*BT_FP_GET_SYS_REG_MASK_BITS)(
        BT_DEVICE *pBtDevice,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        );

typedef int
(*BT_FP_SET_BB_REG_MASK_BITS)(
        BT_DEVICE *pBtDevice,
        int Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        );

typedef int
(*BT_FP_GET_BB_REG_MASK_BITS)(
        BT_DEVICE *pBtDevice,
        int Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        );

typedef int
(*BT_FP_SET_SETRESETMDCOUNT)(
        BT_DEVICE *pBtDevice
        );

typedef int
(*BT_FP_SET_POWERGAININDEX)(
        BT_DEVICE *pBtDevice,
        BT_PKT_TYPE PacketType,
        int Index
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

// Register Read/Write
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

// CON TX
typedef int
(*BT_FP_SET_CONTINUETX_BEGIN)(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

typedef int
(*BT_FP_SET_CONTINUETX_STOP)(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

typedef int
(*BT_FP_SET_CONTINUETX_UPDATE)(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

// PKT TX
typedef int
(*BT_FP_SET_PKTTX_BEGIN)(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

typedef int
(*BT_FP_SET_PKTTX_STOP)(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

typedef int
(*BT_FP_SET_PKTTX_UPDATE)(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

// PKT RX
typedef int
(*BT_FP_SET_PKTRX_BEGIN)(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

typedef int
(*BT_FP_SET_PKTRX_STOP)(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

typedef int
(*BT_FP_SET_PKTRX_UPDATE)(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

typedef int
(*BT_FP_GET_CHIPVERSIONINFO)(
    BT_DEVICE *pBtDevice
    );

typedef int
(*BT_FP_BT_DL_FW)(
    BT_DEVICE *pBtDevice,
    unsigned char *pPatchcode,
    int patchLength
    );

typedef int
(*BT_FP_BT_DL_MERGER_FW)(
    BT_DEVICE *pBtDevice,
    unsigned char *pPatchcode,
    int patchLength
    );

// PG efuse
typedef int
(*BT_FP_WRITE_EFUSE_DATA)(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam
    );

typedef int
(*BT_FP_READ_EFUSE_DATA)(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

// LE
typedef int
(*BT_FP_LE_TEST)(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

typedef int
(*BT_FP_8822B_LE_CONT_TX)(
    BT_DEVICE *pBtDevice,
    unsigned char enableLeContTx,
    unsigned char Channel,
    unsigned char TxPowerIndex
    );

typedef int
(*BT_FP_START)(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

typedef int
(*BT_FP_STOP)(
    BT_DEVICE *pBtDevice
    );

typedef int
(*BT_FP_UPDATE)(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    BT_DEVICE_REPORT *pBtReport
    );

typedef struct BT_TRX_TIME_TAG BT_TRX_TIME;

typedef enum {
    TRX_TIME_STOP =0,
    TX_TIME_RUNING ,
    RX_TIME_RUNING,

    NUMOFTRXTIME_TAG
} TRXTIME_TAG;

struct BT_TRX_TIME_TAG {
    unsigned long beginTimeClockCnt;
    unsigned long UseTimeClockCnt;
    unsigned long endTimeClockCnt;
};

#define MAX_EFUSE_PHY_LEN 512
#define MAX_EFUSE_LOG_LEN 1024
#define MAX_EFUSE_BANK_NUM 4

typedef struct {
    uint8_t NewValue;
    uint8_t OldValue;
} EFUSE_LOGIC;

typedef struct {
    BT_DEVICE *pBtDevice;
    uint8_t pEfusePhyMem[MAX_EFUSE_BANK_NUM*MAX_EFUSE_PHY_LEN];
    EFUSE_LOGIC pEfuseLogMem[MAX_EFUSE_LOG_LEN];
    uint32_t pEfusePhyDataLen[MAX_EFUSE_BANK_NUM];

    uint32_t EfusePhySize;
    uint32_t EfuseLogSize;

    uint8_t StartBank;
    uint8_t BankNum;
    uint8_t CurrBank;
} EFUSE_UNIT;

struct BT_DEVICE_TAG {
    // Table is base function
    uint8_t TXGainTable[MAX_TXGAIN_TABLE_SIZE];
    uint8_t TXDACTable[MAX_TXDAC_TABLE_SIZE];

    EFUSE_UNIT *pSysEfuse;
    EFUSE_UNIT SysEfuseMemory;

    EFUSE_UNIT *pBtEfuse;
    EFUSE_UNIT BtEfuseMemory;

    BT_FP_SET_TXGAINTABLE           SetTxGainTable;
    BT_FP_SET_TXDACTABLE            SetTxDACTable;

    // Register Read/Write
    BT_FP_SET_MD_REG_MASK_BITS      SetMdRegMaskBits;
    BT_FP_GET_MD_REG_MASK_BITS      GetMdRegMaskBits;

    BT_FP_SET_RF_REG_MASK_BITS      SetRfRegMaskBits;
    BT_FP_GET_RF_REG_MASK_BITS      GetRfRegMaskBits;

    // HCI Command & Event
    BT_FP_SEND_HCICOMMANDWITHEVENT  SendHciCommandWithEvent;
    BT_FP_RECV_ANYEVENT             RecvAnyHciEvent;

    // Register Control
    BT_FP_SET_POWERGAININDEX        SetPowerGainIndex;
    BT_FP_SET_POWERGAIN             SetPowerGain;
    BT_FP_SET_POWERDAC              SetPowerDac;
    BT_FP_SET_SETRESETMDCOUNT       SetRestMDCount;
    BT_FP_SET_TEST_MODE_ENABLE      TestModeEnable;

    BT_FP_SET_RTL8761_XTAL          SetRtl8761Xtal;
    BT_FP_GET_RTL8761_XTAL          GetRtl8761Xtal;
    BT_FP_READ_THERMAL              ReadThermal;

    BT_FP_GET_STAGE                 GetStage;
    BT_FP_SET_SYS_REG_MASK_BITS     SetSysRegMaskBits;
    BT_FP_GET_SYS_REG_MASK_BITS     GetSysRegMaskBits;
    BT_FP_SET_BB_REG_MASK_BITS      SetBBRegMaskBits;
    BT_FP_GET_BB_REG_MASK_BITS      GetBBRegMaskBits;

    // Vendor HCI Command Control
    BT_FP_SET_HOPPINGMODE           SetHoppingMode;
    BT_FP_SET_HCIRESET              SetHciReset;

    unsigned long TxTriggerPktCnt;

    //Con-TX
    BT_TRX_TIME                     TRxTime[NUMOFTRXTIME_TAG];
    BT_FP_SET_CONTINUETX_BEGIN      SetContinueTxBegin;
    BT_FP_SET_CONTINUETX_STOP       SetContinueTxStop;
    BT_FP_SET_CONTINUETX_UPDATE     SetContinueTxUpdate;
    // LE
    BT_FP_LE_TEST                   LeTxTestCmd;
    BT_FP_LE_TEST                   LeRxTestCmd;
    BT_FP_LE_TEST                   LeTestEndCmd;
    //PKT-TX
    BT_FP_SET_PKTTX_BEGIN           SetPktTxBegin;
    BT_FP_SET_PKTTX_STOP            SetPktTxStop;
    BT_FP_SET_PKTTX_UPDATE          SetPktTxUpdate;
    //PKT-RX
    BT_FP_SET_PKTRX_BEGIN           SetPktRxBegin;
    BT_FP_SET_PKTRX_STOP            SetPktRxStop;
    BT_FP_SET_PKTRX_UPDATE          SetPktRxUpdate;

    //interface
    uint8_t InterfaceType;

    BASE_INTERFACE_MODULE *pBaseInterface;

    BT_CHIPINFO *pBTInfo;
    BT_CHIPINFO BaseBTInfoMemory;

    BT_FP_GET_CHIPVERSIONINFO       GetChipVersionInfo;
    BT_FP_BT_DL_FW                  BTDlFW;
    BT_FP_BT_DL_MERGER_FW           BTDlMERGERFW;

    // Efuse settting
    BT_FP_WRITE_EFUSE_DATA             WriteEfuseLogicalData;
    BT_FP_READ_EFUSE_DATA           ReadEfuseLogicalData;

    BT_FP_8822B_LE_CONT_TX          LeContTxCmd_8822b;

    BT_FP_START                     FwPacketTxStart;
    BT_FP_STOP                      FwPacketTxStop;
    BT_FP_UPDATE                    FwPacketTxReport;

    BT_FP_START                     FwPacketRxStart;
    BT_FP_STOP                      FwPacketRxStop;
    BT_FP_UPDATE                    FwPacketRxReport;

    BT_FP_START                     FwContTxStart;
    BT_FP_STOP                      FwContTxStop;
    BT_FP_UPDATE                    FwContTxReport;

    BT_FP_UPDATE                    FwReadTxPowerInfo;
};

typedef enum {
    REPORT_ALL = 0,
    REPORT_PKT_TX,
    REPORT_CONT_TX,
    REPORT_PKT_RX,
    REPORT_TX_GAIN_TABLE,
    REPORT_TX_DAC_TABLE,
    REPORT_XTAL,
    REPORT_THERMAL,
    REPORT_BT_STAGE,
    REPORT_CHIP,
    REPORT_LOGICAL_EFUSE,
    REPORT_LE_RX,
    REPORT_LE_CONTINUE_TX,
    REPORT_FW_PACKET_TX,
    REPORT_FW_CONTINUE_TX,
    REPORT_FW_PACKET_RX,
    REPORT_FW_LE_CONTINUE_TX,
    REPORT_TX_POWER_INFO,
} BT_REPORT_TAG;

typedef struct  BT_MODULE_TAG BT_MODULE;

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
        uint16_t UserValue
        );

typedef int
(*BT_MODULE_FP_GET_SYS_REG_MASK_BITS)(
        BT_MODULE *pBtModule,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        );

typedef int
(*BT_MODULE_FP_SET_BB_REG_MASK_BITS)(
        BT_MODULE *pBtModule,
        uint8_t Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        );

typedef int
(*BT_MODULE_FP_GET_BB_REG_MASK_BITS)(
        BT_MODULE *pBtModule,
        uint8_t Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        );

typedef int
(*BT_MODULE_FP_SET_REG_MASK_BITS)(
        BT_MODULE *pBtModule,
        uint8_t Type,
        uint8_t Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        );

typedef int
(*BT_MODULE_FP_GET_REG_MASK_BITS)(
        BT_MODULE *pBtModule,
        uint8_t Type,
        uint8_t Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
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
bt_default_RecvAnyHciEvent(
        BT_DEVICE *pBtDevice,
        uint8_t *pEvent
        );

int
bt_default_SendHciCommandWithEvent(
        BT_DEVICE *pBtDevice,
        uint16_t  OpCode,
        uint8_t PayLoadLength,
        uint8_t *pPayLoad,
        uint8_t  EventType,
        uint8_t  *pEvent,
        uint32_t *pEventLen
        );

int
bt_default_SetSysRegMaskBits(
        BT_DEVICE *pBtDevice,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        );

int
bt_default_GetSysRegMaskBits(
        BT_DEVICE *pBtDevice,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        );

int
bt_default_SetBBRegMaskBits(
        BT_DEVICE *pBtDevice,
        int Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t UserValue
        );

int
bt_default_GetBBRegMaskBits(
        BT_DEVICE *pBtDevice,
        int Page,
        uint16_t Addr,
        uint8_t Msb,
        uint8_t Lsb,
        uint16_t *pUserValue
        );

int
bt_default_GetChipId(
        BT_DEVICE *pBtDevice
        );

int
bt_default_GetECOVersion(
        BT_DEVICE *pBtDevice
        );

#endif
