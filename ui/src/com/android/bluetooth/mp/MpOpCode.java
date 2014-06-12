package com.android.bluetooth.mp;

public final class MpOpCode {
    public static final int BT_MP_OP_CODE_HCI_SEND_CMD       = 0x0000;
    public static final int BT_MP_OP_CODE_DUT_MODE_CONFIGURE = 0x0001;

    public static final int BT_MP_OP_CODE_GetPara        =   0x0080;
    public static final int BT_MP_OP_CODE_SetPara1       =   0x0081;
    public static final int BT_MP_OP_CODE_SetPara2       =   0x0082;
    public static final int BT_MP_OP_CODE_SetHoppingMode =   0x0083;
    public static final int BT_MP_OP_CODE_SetHitTarget   =   0x0084;
    public static final int BT_MP_OP_CODE_SetGainTable   =   0x0085;
    public static final int BT_MP_OP_CODE_SetDacTable    =   0x0086;
    public static final int BT_MP_OP_CODE_Exec           =   0x0087;
    public static final int BT_MP_OP_CODE_ReportTx       =   0x0088;
    public static final int BT_MP_OP_CODE_ReportRx       =   0x0089;
    public static final int BT_MP_OP_CODE_RegMD          =   0x008a;
    public static final int BT_MP_OP_CODE_RegRF          =   0x008b;

    // All Exec subcodes
    public static final int BT_MP_OP_CODE_PktTxStart      =   0x1187; // subcode 17
    public static final int BT_MP_OP_CODE_PktTxUpdate     =   0x1387; // subcode 19
    public static final int BT_MP_OP_CODE_PktTxStop       =   0x1587; // subcode 21
    public static final int BT_MP_OP_CODE_PktRxStart      =   0x1687; // subcode 22
    public static final int BT_MP_OP_CODE_PktRxUpdate     =   0x1887; // subcode 24
    public static final int BT_MP_OP_CODE_PktRxStop       =   0x1987; // subcode 25
    public static final int BT_MP_OP_CODE_PktContTxStart  =   0x1a87; // subcode 26
    public static final int BT_MP_OP_CODE_PktContTxUpdate =   0x1b87; // subcode 27
    public static final int BT_MP_OP_CODE_PktContTxStop   =   0x1c87; // subcode 28

    public static final String BT_MP_OP_STR_GetPara         = "Get Param";
    public static final String BT_MP_OP_STR_SetPara1        = "Set Param1";
    public static final String BT_MP_OP_STR_SetPara2        = "Set Param2";
    public static final String BT_MP_OP_STR_SetHoppingMode  = "Set Hopping Mode";
    public static final String BT_MP_OP_STR_SetHitTarget    = "Set Hit Target";
    public static final String BT_MP_OP_STR_SetGainTable    = "Set Gain Table";
    public static final String BT_MP_OP_STR_SetDacTable     = "Set Dac Table";
    public static final String BT_MP_OP_STR_Exec            = "Exec";
    public static final String BT_MP_OP_STR_ReportTx        = "Report Tx";
    public static final String BT_MP_OP_STR_ReportRx        = "Report Rx";
    public static final String BT_MP_OP_STR_RegMD           = "Reg Modem";
    public static final String BT_MP_OP_STR_RegRF           = "Reg RF";

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

    public static final String STR_BT_MP_GET_PARA         =   "bt_mp_GetPara";
    public static final String STR_BT_MP_SET_PARA1        =   "bt_mp_SetPara1";
    public static final String STR_BT_MP_SET_PARA2        =   "bt_mp_SetPara2";
    public static final String STR_BT_MP_SET_HOPPING_MODE =   "bt_mp_SetHoppingMode";
    public static final String STR_BT_MP_SET_HIT          =   "bt_mp_SetHit";
    public static final String STR_BT_MP_SET_GAIN_TABLE   =   "bt_mp_SetGainTable";
    public static final String STR_BT_MP_SET_DAC_TABLE    =   "bt_mp_SetDacTable";
    public static final String STR_BT_MP_EXEC             =   "bt_mp_Exec";
    public static final String STR_BT_MP_REPORTTX         =   "bt_mp_ReportTx";
    public static final String STR_BT_MP_REPORTRX         =   "bt_mp_ReportRx";
    public static final String STR_BT_MP_REG_MD           =   "bt_mp_RegMd";
    public static final String STR_BT_MP_REG_RF           =   "bt_mp_RegRf";
    public static final String STR_BT_MP_HCI_CMD          =   "hci_cmd";

    public static final String STR_BT_MP_HCI_RESET        =   "bt_mp_HciReset";
    public static final String STR_BT_MP_CONTTXSTART      =   "bt_mp_ConTxStart";
    public static final String STR_BT_MP_CONTTXSTOP       =   "bt_mp_ConTxStop";
    public static final String STR_BT_MP_PKTTXSTART       =   "bt_mp_PktTxStart";
    public static final String STR_BT_MP_PKTTXSTOP        =   "bt_mp_PktTxStop";
    public static final String STR_BT_MP_PKTRXSTART       =   "bt_mp_PktRxStart";
    public static final String STR_BT_MP_PKTRXSTOP        =   "bt_mp_PktRxStop";

    public static final String STR_BT_MP_TX_PARA_DELIM    =   ",";
    public static final String STR_BT_MP_RX_RESULT_DELIM  =   ",";
}
