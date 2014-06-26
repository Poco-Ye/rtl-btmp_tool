package com.android.bluetooth.mp;

public final class MpOpCode {
    public static final int BT_MP_OP_CODE_HCI_SEND_CMD       = 0x0000;
    public static final int BT_MP_OP_CODE_DUT_MODE_CONFIGURE = 0x0001;

    public static final int BT_MP_OP_CODE_GetParam       =   0x0010;
    public static final int BT_MP_OP_CODE_SetParam       =   0x0011;
    public static final int BT_MP_OP_CODE_SetParam1      =   0x0012;
    public static final int BT_MP_OP_CODE_SetParam2      =   0x0013;
    public static final int BT_MP_OP_CODE_SetGainTable   =   0x0014;
    public static final int BT_MP_OP_CODE_SetDacTable    =   0x0015;
    public static final int BT_MP_OP_CODE_Exec           =   0x0016;
    public static final int BT_MP_OP_CODE_ReportTx       =   0x0017;
    public static final int BT_MP_OP_CODE_ReportRx       =   0x0018;
    public static final int BT_MP_OP_CODE_RegMD          =   0x0019;
    public static final int BT_MP_OP_CODE_RegRF          =   0x001a;
    public static final int BT_MP_OP_CODE_RegSYS         =   0x001b;
    public static final int BT_MP_OP_CODE_RegBB          =   0x001c;

    // All Exec subcodes
    public static final int BT_MP_OP_CODE_PktTxStart      =   0x1116; // subcode 17
    public static final int BT_MP_OP_CODE_PktTxUpdate     =   0x1316; // subcode 19
    public static final int BT_MP_OP_CODE_PktTxStop       =   0x1516; // subcode 21
    public static final int BT_MP_OP_CODE_PktRxStart      =   0x1616; // subcode 22
    public static final int BT_MP_OP_CODE_PktRxUpdate     =   0x1816; // subcode 24
    public static final int BT_MP_OP_CODE_PktRxStop       =   0x1916; // subcode 25
    public static final int BT_MP_OP_CODE_PktContTxStart  =   0x1a16; // subcode 26
    public static final int BT_MP_OP_CODE_PktContTxUpdate =   0x1b16; // subcode 27
    public static final int BT_MP_OP_CODE_PktContTxStop   =   0x1c16; // subcode 28

    public static final String BT_MP_OP_STR_GetParam        = "Get Param";
    public static final String BT_MP_OP_STR_SetParam        = "Set Param";
    public static final String BT_MP_OP_STR_SetParam1       = "Set Param1";
    public static final String BT_MP_OP_STR_SetParam2       = "Set Param2";
    public static final String BT_MP_OP_STR_SetGainTable    = "Set Gain Table";
    public static final String BT_MP_OP_STR_SetDacTable     = "Set Dac Table";
    public static final String BT_MP_OP_STR_Exec            = "Exec";
    public static final String BT_MP_OP_STR_ReportTx        = "Report Tx";
    public static final String BT_MP_OP_STR_ReportRx        = "Report Rx";
    public static final String BT_MP_OP_STR_RegMD           = "Reg Modem";
    public static final String BT_MP_OP_STR_RegRF           = "Reg RF";
    public static final String BT_MP_OP_STR_RegSYS          = "Reg SYS";
    public static final String BT_MP_OP_STR_RegBB           = "Reg BB";

    public static final String BT_MP_OP_STR_PktTxStart      = "Pkt Tx Start";
    public static final String BT_MP_OP_STR_PktTxUpdate     = "Pkt Tx Update";
    public static final String BT_MP_OP_STR_PktTxStop       = "Pkt Tx Stop";
    public static final String BT_MP_OP_STR_PktRxStart      = "Pkt Rx Start";
    public static final String BT_MP_OP_STR_PktRxUpdate     = "Pkt Rx Update";
    public static final String BT_MP_OP_STR_PktRxStop       = "Pkt Rx Stop";
    public static final String BT_MP_OP_STR_PktContTxStart  = "Pkt Continue Tx Start";
    public static final String BT_MP_OP_STR_PktContTxUpdate = "Pkt Continue Tx Update";
    public static final String BT_MP_OP_STR_PktContTxStop   = "Pkt Continue Tx Stop";

    // Copied from bluetoothmp.h
    public static final String STR_BT_MP_ENABLE           =   "enable";
    public static final String STR_BT_MP_DISABLE          =   "disable";
    public static final String STR_BT_MP_DUT_MODE         =   "dut_mode_configure";

    public static final String STR_BT_MP_GET_PARAM        =   "bt_mp_GetParam";
    public static final String STR_BT_MP_SET_PARAM        =   "bt_mp_SetParam";
    public static final String STR_BT_MP_SET_PARAM1       =   "bt_mp_SetParam1";
    public static final String STR_BT_MP_SET_PARAM2       =   "bt_mp_SetParam2";
    public static final String STR_BT_MP_SET_GAIN_TABLE   =   "bt_mp_SetGainTable";
    public static final String STR_BT_MP_SET_DAC_TABLE    =   "bt_mp_SetDacTable";
    public static final String STR_BT_MP_EXEC             =   "bt_mp_Exec";
    public static final String STR_BT_MP_REPORTTX         =   "bt_mp_ReportTx";
    public static final String STR_BT_MP_REPORTRX         =   "bt_mp_ReportRx";
    public static final String STR_BT_MP_REG_MD           =   "bt_mp_RegMd";
    public static final String STR_BT_MP_REG_RF           =   "bt_mp_RegRf";
    public static final String STR_BT_MP_REG_SYS          =   "bt_mp_RegSys";
    public static final String STR_BT_MP_REG_BB           =   "bt_mp_RegBb";
    public static final String STR_BT_MP_HCI_CMD          =   "hci_cmd";

    public static final String STR_BT_MP_HCI_RESET        =   "bt_mp_HciReset";
    public static final String STR_BT_MP_CONTTXSTART      =   "bt_mp_ConTxStart";
    public static final String STR_BT_MP_CONTTXSTOP       =   "bt_mp_ConTxStop";
    public static final String STR_BT_MP_PKTTXSTART       =   "bt_mp_PktTxStart";
    public static final String STR_BT_MP_PKTTXSTOP        =   "bt_mp_PktTxStop";
    public static final String STR_BT_MP_PKTRXSTART       =   "bt_mp_PktRxStart";
    public static final String STR_BT_MP_PKTRXSTOP        =   "bt_mp_PktRxStop";

    public static final String STR_BT_MP_PARAM_DELIM   =   ",";
    public static final String STR_BT_MP_RESULT_DELIM  =   ",";
    public static final String STR_BT_MP_PAIR_DELIM    =   ";";
}
