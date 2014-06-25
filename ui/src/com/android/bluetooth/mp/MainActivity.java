package com.android.bluetooth.mp;

import java.util.ArrayList;

import com.android.bluetooth.mp.MpOpCode;
import com.android.bluetooth.mp.MpTestService;
import com.android.bluetooth.mp.MpTestService.MpTestServiceBinder;
import com.android.bluetooth.mp.R;
import com.android.bluetooth.mp.R.id;

import android.os.Bundle;
import android.app.Activity;
import android.app.AlertDialog;
import android.view.Menu;

import android.content.Context;
import android.content.ServiceConnection;
import android.content.Intent;

import android.content.ComponentName;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;

import android.widget.Adapter;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Switch;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import android.view.View;
import android.view.View.OnClickListener;
import android.text.method.ScrollingMovementMethod;

import android.util.Log;


public class MainActivity extends Activity {
    private static final String TAG = "BluetoothMpTestActivity";
    public MpTestService mService = null;
    boolean mBound = false;

    // Switch
    Switch mSwitchOnOff = null;

    // Button
    Button mbuttonStart = null;
    Button mbuttonStop = null;
    //Button mbuttonPause = null;
    //Button mbuttonClear = null;
    Button mbuttonClearLog = null;
    Button mbuttonHCIReset = null;

    // Spineer
    Spinner mspHCIInterface = null;
    Spinner mspActionItem = null;
    Spinner mspRFChannel = null;
    Spinner mspPktType = null;
    Spinner mspPayloadType = null;
    Spinner mspTxGainIndex = null;
    //Spinner mspTestMode = null;
    //Spinner mspRegID = null;
    Spinner mspRegRW = null;
    //Spinner mspHitTarget = null;
    //Spinner mspLEConnect = null;
    //EditText
    EditText metxDevNode = null;
    //EditText metxRxPacketsCount = null;
    //EditText metxErrorBits = null;
    //EditText metxBER = null;
    EditText metxPacketCount = null;
    EditText metxTxGainValue = null;
    EditText metxWhiteningValue = null;
    EditText metxTxDac = null;
    EditText metxPacketHeader = null;
    //EditText metxMultiRxEnable = null;
    EditText metxHoppingFixChannel = null;
    EditText metxHitTarget = null;
    EditText metxTxGainTable0 = null;
    EditText metxTxGainTable1 = null;
    EditText metxTxGainTable2 = null;
    EditText metxTxGainTable3 = null;
    EditText metxTxGainTable4 = null;
    EditText metxTxGainTable5 = null;
    EditText metxTxGainTable6 = null;
    EditText metxTxDacTable0 = null;
    EditText metxTxDacTable1 = null;
    EditText metxTxDacTable2 = null;
    EditText metxTxDacTable3 = null;
    EditText metxTxDacTable4 = null;
    EditText metxRTL8761Xtal = null;
    EditText metxRegPage = null; // only for Reg BB
    EditText metxRegAddr = null;
    EditText metxRegMsb = null;
    EditText metxRegLsb = null;
    EditText metxRegData = null;
    EditText metxGetThermal = null;
    EditText metxGetStage = null;
    EditText metxRxRssi = null;
    EditText metxRxBits = null;
    EditText metxRxErrBits = null;
    EditText metxRxPktCounts = null;
    EditText metxTxBits = null;
    EditText metxTxPktCounts = null;

    // CheckBox
    //CheckBox cbGetThermal = null;
    //CheckBox cbLEConnect = null;

    // LargeText
    TextView metxLog = null;
    //Spinner Adapter
    ArrayAdapter<String> adpHCIInterface = null;
    ArrayAdapter<String> adpActionItem = null;
    ArrayAdapter<String> adpRFChannel = null;
    ArrayAdapter<String> adpPktType = null;
    ArrayAdapter<String> adpTxGainIndex = null;
    ArrayAdapter<String> adpPayloadType = null;
    ArrayAdapter<String> adpTestMode = null;
    //ArrayAdapter<String> adpRegID = null;
    ArrayAdapter<String> adpRegRW = null;

    // All variables we need to cache
    String mHciInterface = null;
    String mDevNode = null;
    String mActionItem = null;
    int mActionCode = -1;
    int mSubActionCode = -1;
    int mStopSubActionCode = -1;
    String mActionParam = null;

    // Internal Messages
    public static final int MSG_MP_STACK_STATUS = 0;
    public static final int MSG_MP_ACTION_START_RESULT = 1;
    public static final int MSG_MP_ACTION_STOP_RESULT = 2;
    public static final int MSG_HCI_SEND = 3;
    public static final int MSG_MP_HCI_EVENT = 4;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG,"onCreate");

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Switch
        mSwitchOnOff = (Switch)findViewById(R.id.switchOnOff);
        if (mSwitchOnOff != null) {
            mSwitchOnOff.setOnCheckedChangeListener(new SwitchOnCheckedChange());
        }

        //Button
        mbuttonStart = (Button) findViewById(R.id.button_Start);
        mbuttonStop = (Button) findViewById(R.id.button_Stop);
        //mbuttonPause = (Button) findViewById(R.id.button_Pause);
        //mbuttonClear = (Button) findViewById(R.id.button_Clear);
        mbuttonClearLog = (Button) findViewById(R.id.button_Clear_Log);
        mbuttonHCIReset = (Button) findViewById(R.id.button_HCI_Reset);
        //Button Listener
        mbuttonStart.setOnClickListener(new ButtonClick());
        mbuttonStop.setOnClickListener(new ButtonClick());
        //mbuttonPause.setOnClickListener(new ButtonClick());
        //mbuttonClear.setOnClickListener(new ButtonClick());
        mbuttonClearLog.setOnClickListener(new ButtonClick());
        mbuttonHCIReset.setOnClickListener(new ButtonClick());

        ButtonStateInit();

        // Spinner
        mspHCIInterface = (Spinner) findViewById(R.id.spinner_hci_interface);
        mspActionItem = (Spinner) findViewById(R.id.spinner_action_item);
        mspRFChannel = (Spinner) findViewById(R.id.spinner_RF_Channel);
        mspPktType = (Spinner) findViewById(R.id.spinner_Pkt_Type);
        mspPayloadType = (Spinner) findViewById(R.id.spinner_Payload_Type);
        mspTxGainIndex = (Spinner) findViewById(R.id.spinner_Tx_Gain_Index);
        //mspTestMode = (Spinner) findViewById(R.id.spinner_Test_Mode);
        //mspRegID = (Spinner) findViewById(R.id.spinner_Register_ID);
        mspRegRW = (Spinner) findViewById(R.id.spinner_Reg_RW);
        //mspHitTarget = (Spinner) findViewById(R.id.spinner_Hit_Target);
        //mspLEConnect=  (Spinner) findViewById(R.id.spinner_LE_Connect);
        //add intems and listener to all spinners
        InitAllSpinners();

        //EditText
        metxDevNode = (EditText) findViewById(R.id.editText_Dev_Node);
        metxPacketCount = (EditText) findViewById(R.id.editText_Pkt_Count);
        metxTxGainValue = (EditText) findViewById(R.id.editText_Tx_Gain_Value);
        metxWhiteningValue = (EditText) findViewById(R.id.editText_Whitening_Value);
        metxTxDac = (EditText) findViewById(R.id.editText_Tx_Dac);
        //metxRxPacketsCount = (EditText) findViewById(R.id.editText_Rx_Packets);
        //metxErrorBits = (EditText) findViewById(R.id.editText_ErrorBits);
        //metxBER = (EditText) findViewById(R.id.editText_BER);
        metxPacketHeader = (EditText) findViewById(R.id.editText_Pkt_Header);
        //metxMultiRxEnable = (EditText) findViewById(R.id.editText_Multi_Rx_Enable);
        metxHoppingFixChannel = (EditText) findViewById(R.id.editText_Hopping_Fix_Channel);
        metxHitTarget = (EditText) findViewById(R.id.editText_Hit_Target);
        metxTxGainTable0 = (EditText) findViewById(R.id.editText_Tx_Gain_Table0);
        metxTxGainTable1 = (EditText) findViewById(R.id.editText_Tx_Gain_Table1);
        metxTxGainTable2 = (EditText) findViewById(R.id.editText_Tx_Gain_Table2);
        metxTxGainTable3 = (EditText) findViewById(R.id.editText_Tx_Gain_Table3);
        metxTxGainTable4 = (EditText) findViewById(R.id.editText_Tx_Gain_Table4);
        metxTxGainTable5 = (EditText) findViewById(R.id.editText_Tx_Gain_Table5);
        metxTxGainTable6 = (EditText) findViewById(R.id.editText_Tx_Gain_Table6);
        metxTxDacTable0 = (EditText) findViewById(R.id.editText_Tx_Dac_Table0);
        metxTxDacTable1 = (EditText) findViewById(R.id.editText_Tx_Dac_Table1);
        metxTxDacTable2 = (EditText) findViewById(R.id.editText_Tx_Dac_Table2);
        metxTxDacTable3 = (EditText) findViewById(R.id.editText_Tx_Dac_Table3);
        metxTxDacTable4 = (EditText) findViewById(R.id.editText_Tx_Dac_Table4);
        metxRTL8761Xtal = (EditText) findViewById(R.id.editText_RTL8761_Xtal);
        metxRegPage = (EditText) findViewById(R.id.editText_Reg_Page);
        metxRegAddr = (EditText) findViewById(R.id.editText_Reg_Addr);
        metxRegMsb = (EditText) findViewById(R.id.editText_Reg_Msb);
        metxRegLsb = (EditText) findViewById(R.id.editText_Reg_Lsb);
        metxRegData = (EditText) findViewById(R.id.editText_Reg_Data);
        metxGetThermal = (EditText) findViewById(R.id.editText_Get_Thermal);
        metxGetStage = (EditText) findViewById(R.id.editText_Get_Stage);
        metxRxRssi = (EditText) findViewById(R.id.editText_Rx_Rssi);
        metxRxBits = (EditText) findViewById(R.id.editText_Rx_Bits);
        metxRxErrBits = (EditText) findViewById(R.id.editText_Rx_Err_Bits);
        metxRxPktCounts = (EditText) findViewById(R.id.editText_Rx_Pkt_Counts);
        metxTxBits = (EditText) findViewById(R.id.editText_Tx_Bits);
        metxTxPktCounts = (EditText) findViewById(R.id.editText_Tx_Pkt_Counts);

        //Init EditText here
        metxDevNode.setTextColor(Color.rgb(255, 0, 0));
        //metxDevNode.setText("xxx");
        //metxDevNode.setKeyListener(null);
        //metxRxPacketsCount.setTextColor(Color.rgb(0, 0, 255));
        //metxRxPacketsCount.setText("xxx");
        //metxRxPacketsCount.setKeyListener(null); //read-only
        //metxErrorBits.setTextColor(Color.rgb(0, 0, 255));
        //metxErrorBits.setText("xxx");
        //metxErrorBits.setKeyListener(null);
        //metxBER.setTextColor(Color.rgb(255, 0, 0));
        //metxBER.setText("xxx");
        //metxBER.setKeyListener(null);
        //metxPacketCount.setText("xxxx");
        //metxPacketCount.setKeyListener(null);
        //metxTxGainValue.setKeyListener(null);
        //metxRxBits.setText("xxx");
        //metxPacketHeader.setText("xxx");
        //metxWhiteningCoeff.setText("xxx");

        // LargeText
        metxLog = (TextView) findViewById(R.id.textView_Log);
        metxLog.setMovementMethod(ScrollingMovementMethod.getInstance());

        // checkBox
        //cbGetThermal = (CheckBox) findViewById(R.id.checkbox_Get_Thermal);
        //cbLEConnect = (CheckBox)findViewById(R.id.checkbox_LE_Connect);
        //add listener to all CheckBoxs
        //cbGetThermal.setOnCheckedChangeListener(new CheckBoxListener());
        //cbLEConnect.setOnCheckedChangeListener(new CheckBoxListener());
    }

    // Update UI here
    Handler updateUIHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_MP_STACK_STATUS:
                    Log.v(TAG, "MP stack status received: " + msg.arg1);
                    metxLog.append("MP stack status: " + String.valueOf(msg.arg1) + "\n");
                    // MP stack enabled
                    if (msg.arg1 == 1) {
                        mbuttonStart.setEnabled(true);
                    } else {
                        mbuttonStart.setEnabled(false);
                        new AlertDialog.Builder(MainActivity.this).
                            setMessage("MP stack disabled!").
                            create().
                            show();
                    }
                    break;

                case MSG_MP_ACTION_START_RESULT:
                    Log.v(TAG, "MP action start result received: " + msg.arg1);
                    metxLog.append("Start result: " + String.valueOf(msg.arg1) + "\n");
                    // start successfully
                    if (msg.arg1 == 0) {
                        setButtonStateStarted();
                    } else {
                        new AlertDialog.Builder(MainActivity.this).
                            setMessage("MP action start failed!").
                            create().
                            show();
                        // Give the chance to select action spinner again
                        mspActionItem.setEnabled(true);
                    }
                    break;

                case MSG_MP_ACTION_STOP_RESULT:
                    Log.v(TAG, "MP action stop result received: " + msg.arg1);
                    metxLog.append("Stop result: " + String.valueOf(msg.arg1) + "\n");
                    // stop successfully
                    if (msg.arg1 == 0) {
                        setButtonStateStopped();
                        mspActionItem.setEnabled(true);
                    } else {
                        new AlertDialog.Builder(MainActivity.this).
                            setMessage("MP action stop failed!").
                            create().
                            show();
                    }
                    break;

                case MSG_HCI_SEND:
                    Log.v(TAG,"HCI Command has sent");
                    String msgBuffer = msg.getData().getString("MP_SEND_BUF");
                    if (msgBuffer != null) {
                        metxLog.append("TX:");
                        metxLog.append(msgBuffer);
                        metxLog.append("\n");
                    }
                    break;

                case MSG_MP_HCI_EVENT:
                    Log.v(TAG, "MP HCI Event received: 0x" + Integer.toHexString(msg.arg1));
                    String RxBuffer = (String)msg.obj;
                    metxLog.append("Rx <- opcode: " + String.valueOf(msg.arg1) + ", buf: " + RxBuffer + "\n");

                    // TODO: update UI according to opcode
                    String[] bufArray = RxBuffer.split(",");
                    switch (msg.arg1) {
                        case MpOpCode.BT_MP_OP_CODE_GetParam:
                            if (bufArray.length != 12 ||
                                !bufArray[0].equals(MpOpCode.STR_BT_MP_GET_PARAM)) {
                                Log.e(TAG, "MP HCI Event Params error");
                                break;
                            }
                            // RF channel
                            Log.v(TAG, "rf channel> 0x" + bufArray[1]);
                            mspRFChannel.setSelection(Integer.parseInt(bufArray[1], 16), true);
                            // Packet type
                            Log.v(TAG, "pkt type> 0x" + bufArray[2]);
                            mspPktType.setSelection(Integer.parseInt(bufArray[2], 16), true);
                            // Payload type
                            Log.v(TAG, "payload type> 0x" + bufArray[3]);
                            mspPayloadType.setSelection(Integer.parseInt(bufArray[3], 16), true);
                            // TxPkt count
                            Log.v(TAG, "tx pkt count> 0x" + bufArray[4]);
                            metxPacketCount.setText("0x" + bufArray[4]);
                            // Tx Gain value
                            Log.v(TAG, "tx gain value> 0x" + bufArray[5]);
                            metxTxGainValue.setText("0x" + bufArray[5]);
                            // Whitening value
                            Log.v(TAG, "whitening value> 0x" + bufArray[6]);
                            metxWhiteningValue.setText("0x" + bufArray[6]);
                            // Tx Gain index
                            Log.v(TAG, "tx gain index> 0x" + bufArray[7]);
                            int tx_gain_index = Integer.parseInt(bufArray[7], 16);
                            if (tx_gain_index < 1 || tx_gain_index > 7) {
                                Log.i(TAG, "Tx Gain index disabled, set to 0");
                                tx_gain_index = 0;
                            }
                            mspTxGainIndex.setSelection(tx_gain_index, true);
                            // Test mode
                            //Log.v(TAG, "tx gain index> " + bufArray[8]);
                            //mspTestMode.setSelection(Integer.parseInt(bufArray[8], 16), true);
                            // Tx Dac
                            Log.v(TAG, "tx dac> 0x" + bufArray[8]);
                            metxTxDac.setText("0x" + bufArray[8]);
                            // Pkt header
                            Log.v(TAG, "pkt hdr> 0x" + bufArray[9]);
                            metxPacketHeader.setText("0x"+bufArray[9]);
                            // Hopping fix channel
                            Log.v(TAG, "hopping fix channel> 0x" + bufArray[10]);
                            metxHoppingFixChannel.setText(Integer.parseInt(bufArray[10], 10) + "");
                            // Hit target
                            Log.v(TAG, "hit target> 0x" + bufArray[11]);
                            metxHitTarget.setText("0x" + bufArray[11]);
                            break;

                        case MpOpCode.BT_MP_OP_CODE_ReportTx:
                            if (bufArray.length != 3 ||
                                !bufArray[0].equals(MpOpCode.STR_BT_MP_REPORTTX)) {
                                Log.e(TAG, "MP HCI Event Params error");
                                break;
                            }
                            // Total tx bits
                            Log.v(TAG, "total tx bits> " + bufArray[1]);
                            metxTxBits.setText("0x" + bufArray[1]);
                            // Total tx counts
                            Log.v(TAG, "total tx counts> " + bufArray[2]);
                            metxTxPktCounts.setText("0x" + bufArray[2]);
                            break;

                        case MpOpCode.BT_MP_OP_CODE_ReportRx:
                            if (bufArray.length != 5 ||
                                !bufArray[0].equals(MpOpCode.STR_BT_MP_REPORTRX)) {
                                Log.e(TAG, "MP HCI Event Params error");
                                break;
                            }
                            // Rx Rssi
                            Log.v(TAG, "rx rssi> " + bufArray[1]);
                            metxRxRssi.setText("0x" + bufArray[1]);
                            // Total rx bits
                            Log.v(TAG, "total rx bits> " + bufArray[2]);
                            metxRxBits.setText("0x" + bufArray[2]);
                            // Total rx counts
                            Log.v(TAG, "total rx counts> " + bufArray[3]);
                            metxRxPktCounts.setText("0x" + bufArray[3]);
                            // Total rx error bits
                            Log.v(TAG, "total rx error bits> " + bufArray[4]);
                            metxRxErrBits.setText("0x" + bufArray[4]);
                            break;

                        case MpOpCode.BT_MP_OP_CODE_RegMD:
                        case MpOpCode.BT_MP_OP_CODE_RegRF:
                        case MpOpCode.BT_MP_OP_CODE_RegSYS:
                        case MpOpCode.BT_MP_OP_CODE_RegBB:
                            if (bufArray.length != 2 && bufArray.length != 3){
                                Log.e(TAG, "MP HCI Event Params count error");
                                break;
                            }

                            if (!bufArray[0].equals(MpOpCode.STR_BT_MP_REG_MD) &&
                                !bufArray[0].equals(MpOpCode.STR_BT_MP_REG_RF) &&
                                !bufArray[0].equals(MpOpCode.STR_BT_MP_REG_SYS) &&
                                !bufArray[0].equals(MpOpCode.STR_BT_MP_REG_BB)) {
                                Log.e(TAG, "MP HCI Event Opcode error");
                                break;
                            }

                            if ((bufArray.length == 3) &&
                                (mspRegRW.getSelectedItemPosition() == 0)) {
                                // It's read operation, update the value
                                Log.v(TAG, "reg data> " + bufArray[2]);
                                metxRegData.setText("0x" + bufArray[2]);
                            }
                            break;

                        default:
                            break;
                    }

                    break;
            }
            super.handleMessage(msg);
        }
    };

    // Before Switch is ON, All buttons are unavailable,
    // except the log clear button always enabled.
    private void ButtonStateInit() {
        mbuttonStart.setEnabled(false);
        mbuttonStop.setEnabled(false);
        mbuttonHCIReset.setEnabled(false);
        mbuttonClearLog.setEnabled(true);
    }

    // This method should be called when start operation
    // is completed in start button response.
    private void setButtonStateStarted() {
        mbuttonStart.setEnabled(false);
        mbuttonStop.setEnabled(true);
        // Make sure Reset is disable, or mess up
        mbuttonHCIReset.setEnabled(false);
    }

    // This method should be called when stop operation
    // is completed in stop button response.
    private void setButtonStateStopped() {
        mbuttonStart.setEnabled(true);
        mbuttonStop.setEnabled(false);
        //mbuttonClearLog.setEnabled(false);
        mbuttonHCIReset.setEnabled(true);
        metxLog.clearAnimation();
    }

    //add items and listeners to all spinners
    private void InitAllSpinners() {
        ArrayList<String> items;
        String[] str;

        // HCI interfaces
        items = new ArrayList<String>();
        str = new String[] {"UART", "USB"};
        for (int i = 0; i < str.length; i++) {
            items.add(str[i]);
        }
        adpHCIInterface = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, items);
        adpHCIInterface.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mspHCIInterface.setAdapter(adpHCIInterface);

        // Action items
        items = new ArrayList<String>();
        //str = new String[] {"Packet Tx", "Packet Rx",
        //                    "Continue Tx", "Continue LE Tx"};
        str = new String[] {MpOpCode.BT_MP_OP_STR_GetParam, MpOpCode.BT_MP_OP_STR_SetParam1,
                            MpOpCode.BT_MP_OP_STR_SetParam2, MpOpCode.BT_MP_OP_STR_SetHoppingMode,
                            MpOpCode.BT_MP_OP_STR_SetHitTarget, MpOpCode.BT_MP_OP_STR_SetGainTable,
                            MpOpCode.BT_MP_OP_STR_SetDacTable, MpOpCode.BT_MP_OP_STR_Exec,
                            MpOpCode.BT_MP_OP_STR_ReportTx, MpOpCode.BT_MP_OP_STR_ReportRx,
                            MpOpCode.BT_MP_OP_STR_RegMD, MpOpCode.BT_MP_OP_STR_RegRF,
                            MpOpCode.BT_MP_OP_STR_RegSYS, MpOpCode.BT_MP_OP_STR_RegBB,
                            MpOpCode.BT_MP_OP_STR_PktTxStart, MpOpCode.BT_MP_OP_STR_PktTxUpdate,
                            MpOpCode.BT_MP_OP_STR_PktRxStart, MpOpCode.BT_MP_OP_STR_PktRxUpdate,
                            MpOpCode.BT_MP_OP_STR_PktContTxStart, MpOpCode.BT_MP_OP_STR_PktContTxUpdate};
        for (int i = 0; i < str.length; i++) {
            items.add(str[i]);
        }
        adpActionItem = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, items);
        adpActionItem.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mspActionItem.setAdapter(adpActionItem);

        // RF channel
        int RF_CHANNEL_MAX = 79;
        items = new ArrayList<String>();
        for (int i = 0; i < RF_CHANNEL_MAX; i++) {
            items.add(String.valueOf(i));
        }
        adpRFChannel = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, items);
        adpRFChannel.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mspRFChannel.setAdapter(adpRFChannel);

        // Pkt Type
        items = new ArrayList<String>();
        str = new String[] {"1DH1", "1DH3", "1DH5", "2DH1", "2DH3", "2DH5", "3DH1", "3DH3", "3DH5",
                            "LE", "NULL", "RTL8723A"};
        for (int i = 0; i < str.length; i++) {
            items.add(str[i]);
        }
        adpPktType = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, items);
        adpPktType.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mspPktType.setAdapter(adpPktType);

        // Payload Type
        str = new String[] {"All_0","All_1", "0101", "1010", "0x0~0xF", "0000~1111", "1111~0000", "PRBS9"};
        items = new ArrayList<String>();
        for (int i = 0; i < str.length; i++) {
            items.add(str[i]);
        }
        adpPayloadType = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, items);
        adpPayloadType.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mspPayloadType.setAdapter(adpPayloadType);

        // Tx Gain Index
        int Tx_Gain_Index_MAX = 8;
        items = new ArrayList<String>();
        for (int i = 0; i < Tx_Gain_Index_MAX; i++) {
            items.add(String.valueOf(i));
        }
        adpTxGainIndex = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, items);
        adpTxGainIndex.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mspTxGainIndex.setAdapter(adpTxGainIndex);

        // TestMode
        //str = new String[]{"DUT Mode", "PSEUDO Mode"};
        //items = new ArrayList<String>();
        //for (int i = 0; i < str.length; i++) {
        //    items.add(str[i]);
        //}
        //adpTestMode = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, items);
        //adpTestMode.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        //mspTestMode.setAdapter(adpTestMode);

        // Reg ID: modem, RF, Sys, BB
        //str = new String[]{"Modem", "RF", "Sys", "BB"};
        //items = new ArrayList<String>();
        //for (int i = 0; i < str.length; i++) {
        //    items.add(str[i]);
        //}
        //adpRegID = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, items);
        //adpRegID.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        //mspRegID.setAdapter(adpRegID);

        // Reg read/write
        str = new String[] {"Read", "Write"};
        items = new ArrayList<String>();
        for (int i = 0; i < str.length; i++) {
            items.add(str[i]);
        }
        adpRegRW = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, items);
        adpRegRW.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mspRegRW.setAdapter(adpRegRW);


        //Set Listener
        mspHCIInterface.setOnItemSelectedListener(new SpinnerListener());
        mspActionItem.setOnItemSelectedListener(new SpinnerListener());
        mspRFChannel.setOnItemSelectedListener(new SpinnerListener());
        mspPktType.setOnItemSelectedListener(new SpinnerListener());
        mspTxGainIndex.setOnItemSelectedListener(new SpinnerListener());
        mspPayloadType.setOnItemSelectedListener(new SpinnerListener());
        //mspTestMode.setOnItemSelectedListener(new SpinnerListener());
        //mspLEConnect.setOnItemSelectedListener(new SpinnerListener());
        //mspHitTarget.setOnItemSelectedListener(new SpinnerListener());
    }

    // Switch Click response
    class SwitchOnCheckedChange implements OnCheckedChangeListener {
        @Override
        public void onCheckedChanged(CompoundButton bv, boolean isChecked) {
            Toast.makeText(getApplicationContext(), "Bluetooth MP tool is " + (isChecked ? "on" : "off"),
                    Toast.LENGTH_SHORT).show();

            mDevNode = metxDevNode.getText().toString();
            if (mDevNode == null || mDevNode.equals("")) {
                // FIXME use R.string.hint_Dev_Node
                //mDevNode = new String("/dev/ttyS0");
                mDevNode = new String("/dev/rtk_btusb");
            }

            Log.d(TAG, "HCI Interface: " + mHciInterface + "; Device Node: " + mDevNode);

            if (isChecked) {
                mService.enableMpTestMode(mHciInterface, mDevNode);
            } else {
                mService.disableMpTestMode();
            }
        }
    }

    // Button Click response
    class ButtonClick implements OnClickListener {
        public void onClick(View v) {
            switch (v.getId()) {
                case R.id.button_Start:
                    Log.v(TAG, "button_Start clicked, action: 0x" + Integer.toHexString(mActionCode));
                    boolean actSpinnerEnable = false;
                    switch (mActionCode) {
                        case MpOpCode.BT_MP_OP_CODE_GetParam:
                            Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_GetParam);
                            mActionParam = null;
                            break;
                        case MpOpCode.BT_MP_OP_CODE_SetParam1:
                            Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_SetParam1);
                            mActionParam = null;
                            // RF channel, 10 radix
                            mActionParam = mspRFChannel.getSelectedItem().toString() + ",";
                            Log.v(TAG, "rf channel> " + mActionParam);
                            // Pkt type, 10 radix
                            mActionParam = mActionParam.concat(mspPktType.getSelectedItemPosition() + ",");
                            Log.v(TAG, "+pkt type> " + mActionParam);
                            // Payload type, 10 radix
                            mActionParam = mActionParam.concat(mspPayloadType.getSelectedItemPosition() + ",");
                            Log.v(TAG, "+payload type> " + mActionParam);
                            // Tx pkt count, 10/16 radix
                            mActionParam = mActionParam.concat(metxPacketCount.getText().toString() + ",");
                            Log.v(TAG, "+tx pkt count> " + mActionParam);
                            // Tx gain value
                            mActionParam = mActionParam.concat(metxTxGainValue.getText().toString() + ",");
                            Log.v(TAG, "+tx gain value> " + mActionParam);
                            // Whitening value
                            mActionParam = mActionParam.concat(metxWhiteningValue.getText().toString() + ",");
                            Log.v(TAG, "+whitening value> " + mActionParam);
                            break;
                        case MpOpCode.BT_MP_OP_CODE_SetParam2:
                            Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_SetParam2);
                            mActionParam = null;
                            // Tx gain index
                            mActionParam = mspTxGainIndex.getSelectedItem().toString() + ",";
                            Log.v(TAG, "tx gain index> " + mActionParam);
                            // Test mode
                            //mActionParam = mActionParam.concat(mspTestMode.getSelectedItemPosition() + ",");
                            //Log.v(TAG, "+test mode> " + mActionParam);
                            // Tx dac
                            mActionParam = mActionParam.concat(metxPacketCount.getText().toString() + ",");
                            Log.v(TAG, "+tx dac> " + mActionParam);
                            // Pkt header
                            mActionParam = mActionParam.concat(mspPktType.getSelectedItemPosition() + ",");
                            Log.v(TAG, "+pkt header> " + mActionParam);
                            // Multi rx enable, 10/16 radix
                            //mActionParam = mActionParam.concat(metxMultiRxEnable.getText().toString() + ",");
                            //Log.v(TAG, "+multi rx enable> " + mActionParam);
                            break;
                        case MpOpCode.BT_MP_OP_CODE_SetHoppingMode:
                            Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_SetHoppingMode);
                            mActionParam = null;
                            // RF channel, 10 radix
                            mActionParam = mspRFChannel.getSelectedItem().toString() + ",";
                            Log.v(TAG, "rf channel> " + mActionParam);
                            // Pkt type, 10 radxi
                            mActionParam = mActionParam.concat(mspPktType.getSelectedItemPosition() + ",");
                            Log.v(TAG, "+pkt type> " + mActionParam);
                            // Hopping fix channel
                            mActionParam = mActionParam.concat(metxHoppingFixChannel.getText().toString() + ",");
                            Log.v(TAG, "+hopping fix channel > " + mActionParam);
                            // Whitening value
                            mActionParam = mActionParam.concat(metxWhiteningValue.getText().toString() + ",");
                            Log.v(TAG, "+whitening value> " + mActionParam);
                            break;
                        case MpOpCode.BT_MP_OP_CODE_SetHitTarget:
                            Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_SetHitTarget);
                            mActionParam = null;
                            // Hit target, 16 radix
                            mActionParam = metxHitTarget.getText().toString() + ",";
                            Log.v(TAG, "hit target> " + mActionParam);
                            break;
                        case MpOpCode.BT_MP_OP_CODE_SetGainTable:
                            Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_SetGainTable);
                            mActionParam = null;
                            // Table0
                            mActionParam = metxTxGainTable0.getText().toString() + ",";
                            Log.v(TAG, "table0> " + mActionParam);
                            // Table1
                            mActionParam = mActionParam.concat(metxTxGainTable1.getText().toString() + ",");
                            Log.v(TAG, "+table1> " + mActionParam);
                            // Table2
                            mActionParam = mActionParam.concat(metxTxGainTable2.getText().toString() + ",");
                            Log.v(TAG, "+table2> " + mActionParam);
                            // Table3
                            mActionParam = mActionParam.concat(metxTxGainTable3.getText().toString() + ",");
                            Log.v(TAG, "+table3> " + mActionParam);
                            // Table4
                            mActionParam = mActionParam.concat(metxTxGainTable4.getText().toString() + ",");
                            Log.v(TAG, "+table4> " + mActionParam);
                            // Table5
                            mActionParam = mActionParam.concat(metxTxGainTable5.getText().toString() + ",");
                            Log.v(TAG, "+table5> " + mActionParam);
                            // Table6
                            mActionParam = mActionParam.concat(metxTxGainTable6.getText().toString() + ",");
                            Log.v(TAG, "+table6> " + mActionParam);
                            break;
                        case MpOpCode.BT_MP_OP_CODE_SetDacTable:
                            Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_SetDacTable);
                            mActionParam = null;
                            // Table0
                            mActionParam = metxTxDacTable0.getText().toString() + ",";
                            Log.v(TAG, "table0> " + mActionParam);
                            // Table1
                            mActionParam = mActionParam.concat(metxTxDacTable1.getText().toString() + ",");
                            Log.v(TAG, "+table1> " + mActionParam);
                            // Table2
                            mActionParam = mActionParam.concat(metxTxDacTable2.getText().toString() + ",");
                            Log.v(TAG, "+table2> " + mActionParam);
                            // Table3
                            mActionParam = mActionParam.concat(metxTxDacTable3.getText().toString() + ",");
                            Log.v(TAG, "+table3> " + mActionParam);
                            // Table4
                            mActionParam = mActionParam.concat(metxTxDacTable4.getText().toString() + ",");
                            Log.v(TAG, "+table4> " + mActionParam);
                            break;
                        case MpOpCode.BT_MP_OP_CODE_Exec:
                            mActionParam = null;
                            switch (mSubActionCode) {
                                case MpOpCode.BT_MP_OP_CODE_PktTxStart:
                                    Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_PktTxStart);
                                    mActionParam = Integer.toString((mSubActionCode >> 8 ) & 0xFF);
                                    Log.v(TAG, "action index> " + mActionParam);
                                    mStopSubActionCode = MpOpCode.BT_MP_OP_CODE_PktTxStop;
                                    break;
                                case MpOpCode.BT_MP_OP_CODE_PktTxUpdate:
                                    Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_PktTxUpdate);
                                    mActionParam = Integer.toString((mSubActionCode >> 8 ) & 0xFF);
                                    Log.v(TAG, "action index> " + mActionParam);
                                    break;
                                case MpOpCode.BT_MP_OP_CODE_PktRxStart:
                                    Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_PktRxStart);
                                    mActionParam = Integer.toString((mSubActionCode >> 8 ) & 0xFF);
                                    Log.v(TAG, "action index> " + mActionParam);
                                    mStopSubActionCode = MpOpCode.BT_MP_OP_CODE_PktRxStop;
                                    break;
                                case MpOpCode.BT_MP_OP_CODE_PktRxUpdate:
                                    Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_PktRxUpdate);
                                    mActionParam = Integer.toString((mSubActionCode >> 8 ) & 0xFF);
                                    Log.v(TAG, "action index> " + mActionParam);
                                    break;
                                case MpOpCode.BT_MP_OP_CODE_PktContTxStart:
                                    Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_PktContTxStart);
                                    mActionParam = Integer.toString((mSubActionCode >> 8 ) & 0xFF);
                                    Log.v(TAG, "action index> " + mActionParam);
                                    mStopSubActionCode = MpOpCode.BT_MP_OP_CODE_PktContTxStop;
                                    break;
                                case MpOpCode.BT_MP_OP_CODE_PktContTxUpdate:
                                    Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_PktContTxUpdate);
                                    mActionParam = Integer.toString((mSubActionCode >> 8 ) & 0xFF);
                                    Log.v(TAG, "action index> " + mActionParam);
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case MpOpCode.BT_MP_OP_CODE_ReportTx:
                            Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_ReportTx);
                            mActionParam = null;
                            break;
                        case MpOpCode.BT_MP_OP_CODE_ReportRx:
                            Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_ReportRx);
                            mActionParam = null;
                            break;
                        case MpOpCode.BT_MP_OP_CODE_RegMD:
                            Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_RegMD);
                            mActionParam = null;
                            // RW
                            mActionParam = mspRegRW.getSelectedItemPosition() + ",";
                            Log.v(TAG, "RW> " + mActionParam);
                            // Address
                            mActionParam = mActionParam.concat(metxRegAddr.getText().toString() + ",");
                            Log.v(TAG, "+addr> " + mActionParam);
                            // Msb
                            mActionParam = mActionParam.concat(metxRegMsb.getText().toString() + ",");
                            Log.v(TAG, "+msb> " + mActionParam);
                            // Lsb
                            mActionParam = mActionParam.concat(metxRegLsb.getText().toString() + ",");
                            Log.v(TAG, "+lsb> " + mActionParam);
                            // data sent to stack if Write
                            if (mspRegRW.getSelectedItemPosition() == 1) {
                                mActionParam = mActionParam.concat(metxRegData.getText().toString() + ",");
                                Log.v(TAG, "+data> " + mActionParam);
                            }
                            break;
                        case MpOpCode.BT_MP_OP_CODE_RegRF:
                            Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_RegRF);
                            mActionParam = null;
                            // RW
                            mActionParam = mspRegRW.getSelectedItemPosition() + ",";
                            Log.v(TAG, "RW> " + mActionParam);
                            // Address
                            mActionParam = mActionParam.concat(metxRegAddr.getText().toString() + ",");
                            Log.v(TAG, "+addr> " + mActionParam);
                            // Msb
                            mActionParam = mActionParam.concat(metxRegMsb.getText().toString() + ",");
                            Log.v(TAG, "+msb> " + mActionParam);
                            // Lsb
                            mActionParam = mActionParam.concat(metxRegLsb.getText().toString() + ",");
                            Log.v(TAG, "+lsb> " + mActionParam);
                            // data sent to stack if Write
                            if (mspRegRW.getSelectedItemPosition() == 1) {
                                mActionParam = mActionParam.concat(metxRegData.getText().toString() + ",");
                                Log.v(TAG, "+data> " + mActionParam);
                            }
                            break;
                        case MpOpCode.BT_MP_OP_CODE_RegSYS:
                            Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_RegSYS);
                            mActionParam = null;
                            // RW
                            mActionParam = mspRegRW.getSelectedItemPosition() + ",";
                            Log.v(TAG, "RW> " + mActionParam);
                            // Address
                            mActionParam = mActionParam.concat(metxRegAddr.getText().toString() + ",");
                            Log.v(TAG, "+addr> " + mActionParam);
                            // Msb
                            mActionParam = mActionParam.concat(metxRegMsb.getText().toString() + ",");
                            Log.v(TAG, "+msb> " + mActionParam);
                            // Lsb
                            mActionParam = mActionParam.concat(metxRegLsb.getText().toString() + ",");
                            Log.v(TAG, "+lsb> " + mActionParam);
                            // data sent to stack if Write
                            if (mspRegRW.getSelectedItemPosition() == 1) {
                                mActionParam = mActionParam.concat(metxRegData.getText().toString() + ",");
                                Log.v(TAG, "+data> " + mActionParam);
                            }
                            break;
                        case MpOpCode.BT_MP_OP_CODE_RegBB:
                            Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_RegBB);
                            mActionParam = null;
                            // RW
                            mActionParam = mspRegRW.getSelectedItemPosition() + ",";
                            Log.v(TAG, "RW> " + mActionParam);
                            // Page
                            mActionParam = mActionParam.concat(metxRegPage.getText().toString() + ",");
                            Log.v(TAG, "+page> " + mActionParam);
                            // Address
                            mActionParam = mActionParam.concat(metxRegAddr.getText().toString() + ",");
                            Log.v(TAG, "+addr> " + mActionParam);
                            // Msb
                            mActionParam = mActionParam.concat(metxRegMsb.getText().toString() + ",");
                            Log.v(TAG, "+msb> " + mActionParam);
                            // Lsb
                            mActionParam = mActionParam.concat(metxRegLsb.getText().toString() + ",");
                            Log.v(TAG, "+lsb> " + mActionParam);
                            // data sent to stack if Write
                            if (mspRegRW.getSelectedItemPosition() == 1) {
                                mActionParam = mActionParam.concat(metxRegData.getText().toString() + ",");
                                Log.v(TAG, "+data> " + mActionParam);
                            }
                            break;
                        default:
                            Log.v(TAG, "Undefined start action: opcode 0x" +
                                    Integer.toHexString(mActionCode));
                            actSpinnerEnable = true;
                            break;
                    }

                    mspActionItem.setEnabled(actSpinnerEnable);
                    // Send action to stack; subaction code set in mActionParam
                    mService.hciSend(mActionCode & 0xFF, mActionParam, true);

                    break;

                case R.id.button_Stop:
                    Log.v(TAG,"button_Stop clicked");
                    // Just make start button available again,
                    // send stop action to stack when neccessary.
                    if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_PktTxStart) ||
                        mActionItem.equals(MpOpCode.BT_MP_OP_STR_PktRxStart) ||
                        mActionItem.equals(MpOpCode.BT_MP_OP_STR_PktContTxStart)) {
                        mActionParam = Integer.toString((mStopSubActionCode >> 8) & 0xFF);
                        Log.v(TAG, "stop action index> " + mActionParam);
                        mService.hciSend(mActionCode & 0xFF, mActionParam, false);
                    } else {
                        setButtonStateStopped();
                        // Action spinner is available after stop clicked.
                        mspActionItem.setEnabled(true);
                    }
                    break;

                case R.id.button_Clear_Log:
                    Log.v(TAG, "button_Clear_Log clicked");
                    metxLog.setText("");
                    break;

                case R.id.button_HCI_Reset:
                    Log.v(TAG, "button_HCI_Reset clicked");
                    break;

                default:
                    break;
            }
        }
    }

    // Spinner listener implementation here
    class SpinnerListener implements OnItemSelectedListener {
        @Override
        public void onItemSelected(AdapterView<?> arg0, View v, int arg2,long arg3) {
            switch (arg0.getId()) {
                case R.id.spinner_hci_interface:
                    Log.v(TAG, "Spinner HCI Interface selected: " + mspHCIInterface.getSelectedItem().toString());
                    mHciInterface = mspHCIInterface.getSelectedItem().toString();
                    break;

                case R.id.spinner_action_item:
                    Log.v(TAG, "Spinner Action Item selected: " + mspActionItem.getSelectedItem().toString());
                    mActionItem = mspActionItem.getSelectedItem().toString();

                    if (mActionItem == null || mActionItem.equals("")) {
                        Log.e(TAG, "Illegal Action Item selected");
                        break;
                    }

                    // TODO: use hashMap
                    if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_GetParam)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_GetParam;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_SetParam1)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_SetParam1;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_SetParam2)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_SetParam2;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_SetHoppingMode)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_SetHoppingMode;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_SetHitTarget)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_SetHitTarget;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_SetGainTable)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_SetGainTable;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_SetDacTable)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_SetDacTable;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_Exec)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_ReportTx)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_ReportTx;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_ReportRx)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_ReportRx;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_RegMD)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_RegMD;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_RegRF)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_RegRF;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_RegSYS)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_RegSYS;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_RegBB)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_RegBB;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_PktTxStart)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_PktTxStart;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_PktTxUpdate)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_PktTxUpdate;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_PktRxStart)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_PktRxStart;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_PktRxUpdate)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_PktRxUpdate;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_PktContTxStart)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_PktContTxStart;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_PktContTxUpdate)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_PktContTxUpdate;
                    }
                    break;

                case R.id.spinner_RF_Channel:
                    Log.v(TAG, "Spinner RF Channel selected: " + mspRFChannel.getSelectedItem().toString());
                    break;

                case R.id.spinner_Pkt_Type:
                    Log.v(TAG, "Spinner Packet Type selected: " + mspPktType.getSelectedItem().toString());
                    break;

                case R.id.spinner_Payload_Type:
                    Log.v(TAG, "Spinner payload Type selected: " + mspPayloadType.getSelectedItem().toString());
                    break;

                case R.id.spinner_Tx_Gain_Index:
                    Log.v(TAG, "Spinner Tx Gain Index selected: " + mspTxGainIndex.getSelectedItem().toString());
                    break;

                //case R.id.spinner_Test_Mode:
                //    Log.v(TAG, "Spinner Test Mode selected: " + mspTestMode.getSelectedItem().toString());
                //    break;

                //case R.id.spinner_Register_ID:
                //    Log.v(TAG, "Register ID selected: " + mspRegID.getSelectedItem().toString());
                //    break;

                //case R.id.spinner_LE_Connect:
                //    Log.v(TAG, "Spinner LE Connect selected: " + mspLEConnect.getSelectedItem().toString());
                //    break;

                //case R.id.spinner_Hit_Target:
                //    Log.v(TAG, "Spinner Hit Target selected: " + mspHitTarget.getSelectedItem().toString());
                //    break;

            }
        }
        @Override
        public void onNothingSelected(AdapterView<?> arg0) {
            // TODO Auto-generated method stub
            Log.v(TAG, "Spinner select nothing");
        }
    }

    // CheckBox Listener implementation here
    class CheckBoxListener implements OnCheckedChangeListener {
        @Override
        public void onCheckedChanged(CompoundButton bv, boolean isChecked) {
            switch (bv.getId()) {
                //case R.id.checkbox_Get_Thermal:
                //    if (isChecked) {
                //        Log.v(TAG, "UploadThremal Checkbox is selected!");
                //    } else {
                //        Log.v(TAG, "UploadThremal Checkbox is canceled!");
                //    }
                //    break;

                //case R.id.checkbox_LE_Connect:
                //    if (isChecked) {
                //        Log.v(TAG, "LE Connect Checkbox is selected!");
                //    } else {
                //        Log.v(TAG, "LE Connect Checkbox is canceled!");
                //    }
                //    break;
                default:
                    break;
            }
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        //Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.activity_main, menu);
        return true;
    }

    @Override
    protected void onStart() {
        Log.d(TAG,"onStart");

        super.onStart();
        // Bind to LocalService
        Intent intent = new Intent(this, MpTestService.class);
        bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG,"onDestroy");

        // TODO Auto-generated method stub
        super.onDestroy();
        // Unbind from the service
        if (mBound) {
            unbindService(mConnection);
            mBound = false;
        }
    }

    @Override
    protected void onStop() {
        Log.d(TAG,"onStop");

        super.onStop();
        // Unbind from the service
        if (mBound) {
            unbindService(mConnection);
            mBound = false;
        }
    }

    /** Defines callbacks for service binding, passed to bindService() */
    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName className,
                IBinder service) {
            Log.d(TAG,"onServiceConnected");

            // We've bound to LocalService, cast the IBinder and get LocalService instance
            MpTestServiceBinder binder = (MpTestServiceBinder) service;
            mService = binder.getService();
            //Service get the updateUIHandler
            mService.RegisterHandler(updateUIHandler);
            mBound = true;

            //
            //mbuttonStart.setEnabled(true);
            //mbuttonStop.setEnabled(true);
        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            Log.d(TAG,"onServiceDisconnected");

            mBound = false;
            //mbuttonStart.setEnabled(false);
            //mbuttonStop.setEnabled(false);
        }
    };
}
