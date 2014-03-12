package com.android.bluetooth.btservice;


public final class MpOpcode {
	
	public static final  int BT_MP_OP_HCI_SEND_CMD = 		0x00;	
	public static final  int BT_MP_OP_DUT_MODE_CONFIGURE = 0x01;


	public static final  int BT_MP_OP_USER_DEF_GetPara = 	0x80;
	public static final  int BT_MP_OP_USER_DEF_SetPara1 = 	0x81;
	public static final  int BT_MP_OP_USER_DEF_SetPara2 = 	0x82;
	public static final  int BT_MP_OP_USER_DEF_SetHit = 		0x83;
	public static final  int BT_MP_OP_USER_DEF_SetDacTable = 	0x84;
	public static final  int BT_MP_OP_USER_DEF_SetGainTable = 	0x85;
	public static final  int BT_MP_OP_USER_DEF_Exec = 		0x86;
	public static final  int BT_MP_OP_USER_DEF_ReportTx = 	0x87;
	public static final  int BT_MP_OP_USER_DEF_ReportRx = 	0x88;

}
