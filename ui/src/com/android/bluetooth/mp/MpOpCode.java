package com.android.bluetooth.mp;

public final class MpOpCode {
    public static final int BT_MP_OP_CODE_HCI_SEND_CMD       = 0x0000;

    public static final int BT_MP_OP_CODE_GetParam       =   0x0010;
    public static final int BT_MP_OP_CODE_SetParam       =   0x0011;
    public static final int BT_MP_OP_CODE_SetParam1      =   0x0012;
    public static final int BT_MP_OP_CODE_SetParam2      =   0x0013;
    public static final int BT_MP_OP_CODE_SetConfig      =   0x0014;
    public static final int BT_MP_OP_CODE_Exec           =   0x0015;
    public static final int BT_MP_OP_CODE_Report         =   0x0016;
    public static final int BT_MP_OP_CODE_RegRW          =   0x0017;

    // All Exec subcodes
    public static final int BT_MP_OP_CODE_HciReset               =   0x0015; // subcode 0
    public static final int BT_MP_OP_CODE_TestMode               =   0x0115; // subcode 1
    public static final int BT_MP_OP_CODE_PGEfuse                =   0x0215; // subcode 2
    public static final int BT_MP_OP_CODE_SetTxGainTable         =   0x0315; // subcode 3
    public static final int BT_MP_OP_CODE_SetTxDACTable          =   0x0415; // subcode 4
    public static final int BT_MP_OP_CODE_SetDefaultTxGainTable  =   0x0515; // subcode 5
    public static final int BT_MP_OP_CODE_SetDefaultTxDACTable   =   0x0615; // subcode 6
    public static final int BT_MP_OP_CODE_SetPowerGainIndex      =   0x0715; // subcode 7
    public static final int BT_MP_OP_CODE_SetPowerGain           =   0x0815; // subcode 8
    public static final int BT_MP_OP_CODE_SetPowerDAC            =   0x0915; // subcode 9
    public static final int BT_MP_OP_CODE_SetXTAL                =   0x0a15; // subcode 10
    public static final int BT_MP_OP_CODE_ClearReport            =   0x0b15; // subcode 11
    public static final int BT_MP_OP_CODE_PktTxStart             =   0x0c15; // subcode 12
    public static final int BT_MP_OP_CODE_PktTxUpdate            =   0x0d15; // subcode 13
    public static final int BT_MP_OP_CODE_PktTxStop              =   0x0e15; // subcode 14
    public static final int BT_MP_OP_CODE_PktContTxStart         =   0x0f15; // subcode 15
    public static final int BT_MP_OP_CODE_PktContTxUpdate        =   0x1015; // subcode 16
    public static final int BT_MP_OP_CODE_PktContTxStop          =   0x1115; // subcode 17
    public static final int BT_MP_OP_CODE_PktRxStart             =   0x1215; // subcode 18
    public static final int BT_MP_OP_CODE_PktRxUpdate            =   0x1315; // subcode 19
    public static final int BT_MP_OP_CODE_PktRxStop              =   0x1415; // subcode 20
    public static final int BT_MP_OP_CODE_HoppingMode            =   0x1515; // subcode 21
    public static final int BT_MP_OP_CODE_LETxTest               =   0x1615; // subcode 22
    public static final int BT_MP_OP_CODE_LERxTest               =   0x1715; // subcode 23
    public static final int BT_MP_OP_CODE_LEEndTest              =   0x1815; // subcode 24

    public static final int BT_MP_OP_CODE_ReportAll              =   0x0016; // subcode 0
    public static final int BT_MP_OP_CODE_ReportTx               =   0x0116; // subcode 1
    public static final int BT_MP_OP_CODE_ReportContTx           =   0x0216; // subcode 2
    public static final int BT_MP_OP_CODE_ReportRx               =   0x0316; // subcode 3
    public static final int BT_MP_OP_CODE_ReportTxGainTable      =   0x0416; // subcode 4
    public static final int BT_MP_OP_CODE_ReportTxDACTable       =   0x0516; // subcode 5
    public static final int BT_MP_OP_CODE_ReportXTAL             =   0x0616; // subcode 6
    public static final int BT_MP_OP_CODE_ReportThermal          =   0x0716; // subcode 7
    public static final int BT_MP_OP_CODE_ReportStage            =   0x0816; // subcode 8
    public static final int BT_MP_OP_CODE_ReportChip             =   0x0916; // subcode 9

    public static final String BT_MP_OP_STR_GetParam        = "Get Param";
    public static final String BT_MP_OP_STR_SetParam        = "Set Param";
    public static final String BT_MP_OP_STR_SetParam1       = "Set Param1";
    public static final String BT_MP_OP_STR_SetParam2       = "Set Param2";
    public static final String BT_MP_OP_STR_TestMode        = "Enter Test Mode";
    public static final String BT_MP_OP_STR_SetTxGainTable  = "Set Tx Gain Table";
    public static final String BT_MP_OP_STR_SetTxDACTable   = "Set Tx DAC Table";
    public static final String BT_MP_OP_STR_SetDefaultTxGainTable  = "Set Default Tx Gain Table";
    public static final String BT_MP_OP_STR_SetDefaultTxDACTable   = "Set Default Tx DAC Table";
    public static final String BT_MP_OP_STR_SetPowerGainIndex      = "Set Power Gain Index";
    public static final String BT_MP_OP_STR_SetPowerGain    = "Set Power Gain";
    public static final String BT_MP_OP_STR_SetPowerDAC     = "Set Power DAC";
    public static final String BT_MP_OP_STR_SetXTAL         = "Set XTAL";
    public static final String BT_MP_OP_STR_ClearReport     = "Clear Report";
    public static final String BT_MP_OP_STR_PktTxStart      = "Pkt Tx Start";
    public static final String BT_MP_OP_STR_PktTxUpdate     = "Pkt Tx Update";
    public static final String BT_MP_OP_STR_PktTxStop       = "Pkt Tx Stop";
    public static final String BT_MP_OP_STR_PktContTxStart  = "Pkt Continue Tx Start";
    public static final String BT_MP_OP_STR_PktContTxUpdate = "Pkt Continue Tx Update";
    public static final String BT_MP_OP_STR_PktContTxStop   = "Pkt Continue Tx Stop";
    public static final String BT_MP_OP_STR_PktRxStart      = "Pkt Rx Start";
    public static final String BT_MP_OP_STR_PktRxUpdate     = "Pkt Rx Update";
    public static final String BT_MP_OP_STR_PktRxStop       = "Pkt Rx Stop";
    public static final String BT_MP_OP_STR_HoppingMode     = "Enable Hopping Mode";
    public static final String BT_MP_OP_STR_LETxTest        = "LE DUT Tx test";
    public static final String BT_MP_OP_STR_LERxTest        = "LE DUT Rx test";
    public static final String BT_MP_OP_STR_LEEndTest       = "LE DUT End test";
    public static final String BT_MP_OP_STR_RegRW           = "Reg R/W";
    public static final String BT_MP_OP_STR_ReportTx        = "Report Pkt Tx";
    public static final String BT_MP_OP_STR_ReportContTx    = "Report Continue Tx";
    public static final String BT_MP_OP_STR_ReportRx        = "Report Pkt Rx";
    public static final String BT_MP_OP_STR_ReportTxGainTable      = "Report Tx Gain Table";
    public static final String BT_MP_OP_STR_ReportTxDACTable       = "Report Tx DAC Table";
    public static final String BT_MP_OP_STR_ReportXTAL      = "Report XTAL";
    public static final String BT_MP_OP_STR_ReportThermal   = "Report Thermal";
    public static final String BT_MP_OP_STR_ReportStage     = "Report Stage";
    // Copied from bluetoothmp.h
    public static final String STR_BT_MP_ENABLE           =   "enable";
    public static final String STR_BT_MP_DISABLE          =   "disable";

    public static final String STR_BT_MP_HCI_CMD          =   "bt_mp_HciCmd";
    public static final String STR_BT_MP_GET_PARAM        =   "bt_mp_GetParam";
    public static final String STR_BT_MP_SET_PARAM        =   "bt_mp_SetParam";
    public static final String STR_BT_MP_SET_PARAM1       =   "bt_mp_SetParam1";
    public static final String STR_BT_MP_SET_PARAM2       =   "bt_mp_SetParam2";
    public static final String STR_BT_MP_SET_CONFIG       =   "bt_mp_SetConfig";
    public static final String STR_BT_MP_EXEC             =   "bt_mp_Exec";
    public static final String STR_BT_MP_REPORT           =   "bt_mp_Report";
    public static final String STR_BT_MP_REG_RW           =   "bt_mp_RegRW";

    public static final String STR_BT_MP_PARAM_DELIM   =   ",";
    public static final String STR_BT_MP_RESULT_DELIM  =   ",";
    public static final String STR_BT_MP_PAIR_DELIM    =   ";";
}
