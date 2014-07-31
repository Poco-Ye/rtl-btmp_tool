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
    Spinner mspRegID = null;
    Spinner mspRegRW = null;
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
    EditText metxXtal = null;
    EditText metxRegPage = null; // only for Reg BB
    EditText metxRegAddr = null;
    EditText metxRegMsb = null;
    EditText metxRegLsb = null;
    EditText metxRegData = null;
    EditText metxThermal = null;
    EditText metxStage = null;
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
    ArrayAdapter<String> adpRegID = null;
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
    public static final int MSG_MP_ACTION_START = 1;
    public static final int MSG_MP_ACTION_UPDATE = 2;
    public static final int MSG_MP_ACTION_STOP = 3;
    public static final int MSG_HCI_SEND = 4;
    public static final int MSG_MP_HCI_EVENT = 5;

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
        mspRegID = (Spinner) findViewById(R.id.spinner_Register_ID);
        mspRegRW = (Spinner) findViewById(R.id.spinner_Reg_RW);
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
        metxXtal = (EditText) findViewById(R.id.editText_Xtal);
        metxRegPage = (EditText) findViewById(R.id.editText_Reg_Page);
        metxRegAddr = (EditText) findViewById(R.id.editText_Reg_Addr);
        metxRegMsb = (EditText) findViewById(R.id.editText_Reg_Msb);
        metxRegLsb = (EditText) findViewById(R.id.editText_Reg_Lsb);
        metxRegData = (EditText) findViewById(R.id.editText_Reg_Data);
        metxThermal = (EditText) findViewById(R.id.editText_Thermal);
        metxStage = (EditText) findViewById(R.id.editText_Stage);
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
        //cbGetThermal = (CheckBox) findViewById(R.id.checkbox_Thermal);
        //cbLEConnect = (CheckBox)findViewById(R.id.checkbox_LE_Connect);
        //add listener to all CheckBoxs
        //cbGetThermal.setOnCheckedChangeListener(new CheckBoxListener());
        //cbLEConnect.setOnCheckedChangeListener(new CheckBoxListener());
    }

    static private int strParseInt(String string) {
        if (string.startsWith("0x") || string.startsWith("0X"))
            return Integer.parseInt(string.substring(2), 16);
        else if (string.startsWith("0"))
            return Integer.parseInt(string.substring(1), 8);
        else
            return Integer.parseInt(string, 10);
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

                case MSG_MP_ACTION_START:
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

                case MSG_MP_ACTION_STOP:
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
                            Log.v(TAG, "rf channel> " + bufArray[1]);
                            mspRFChannel.setSelection(strParseInt(bufArray[1]), true);
                            // Packet type
                            Log.v(TAG, "pkt type> " + bufArray[2]);
                            mspPktType.setSelection(strParseInt(bufArray[2]), true);
                            // Payload type
                            Log.v(TAG, "payload type> " + bufArray[3]);
                            mspPayloadType.setSelection(strParseInt(bufArray[3]), true);
                            // Tx Pkt count
                            Log.v(TAG, "tx pkt count> " + bufArray[4]);
                            metxPacketCount.setText("0x" + Integer.toHexString(strParseInt(bufArray[4])));
                            // Tx Gain value
                            Log.v(TAG, "tx gain value> " + bufArray[5]);
                            metxTxGainValue.setText("0x" + Integer.toHexString(strParseInt(bufArray[5])));
                            // Whitening value
                            Log.v(TAG, "whitening value> " + bufArray[6]);
                            metxWhiteningValue.setText("0x" + Integer.toHexString(strParseInt(bufArray[6])));
                            // Tx Gain index
                            Log.v(TAG, "tx gain index> " + bufArray[7]);
                            int tx_gain_index = strParseInt(bufArray[7]);
                            if (tx_gain_index < 1 || tx_gain_index > 7) {
                                Log.i(TAG, "Tx Gain index disabled, set to 0");
                                tx_gain_index = 0;
                            }
                            mspTxGainIndex.setSelection(tx_gain_index, true);
                            // Tx Dac
                            Log.v(TAG, "tx dac> " + bufArray[8]);
                            metxTxDac.setText("0x" + Integer.toHexString(strParseInt(bufArray[8])));
                            // Pkt header
                            Log.v(TAG, "pkt hdr> " + bufArray[9]);
                            metxPacketHeader.setText("0x" + Integer.toHexString(strParseInt(bufArray[9])));
                            // Hopping fix channel
                            Log.v(TAG, "hopping fix channel> " + bufArray[10]);
                            metxHoppingFixChannel.setText(strParseInt(bufArray[10]) + "");
                            // Hit target, must be hex format
                            Log.v(TAG, "hit target> " + bufArray[11]);
                            metxHitTarget.setText(bufArray[11]);
                            break;

                        case MpOpCode.BT_MP_OP_CODE_Report:
                            if (!bufArray[0].equals(MpOpCode.STR_BT_MP_REPORT)) {
                                Log.e(TAG, "MP HCI Event Params error");
                                break;
                            }
                            switch (strParseInt(bufArray[1])) {
                                case MpOpCode.BT_MP_OP_CODE_ReportTx >> 8:
                                case MpOpCode.BT_MP_OP_CODE_ReportContTx >> 8:
                                    if (bufArray.length != 5 || strParseInt(bufArray[2]) != 0) {
                                        Log.e(TAG, "MP HCI Event Params error");
                                        break;
                                    }
                                    // Total tx bits
                                    Log.v(TAG, "total tx bits> " + bufArray[3]);
                                    metxTxBits.setText("0x" + Integer.toHexString(strParseInt(bufArray[3])));
                                    // Total tx counts
                                    Log.v(TAG, "total tx counts> " + bufArray[4]);
                                    metxTxBits.setText("0x" + Integer.toHexString(strParseInt(bufArray[4])));
                                    break;
                                case MpOpCode.BT_MP_OP_CODE_ReportRx >> 8:
                                    if (bufArray.length != 7 || strParseInt(bufArray[2]) != 0) {
                                        Log.e(TAG, "MP HCI Event Params error");
                                        break;
                                    }
                                    // Rx Rssi
                                    Log.v(TAG, "rx rssi> " + bufArray[3]);
                                    metxRxRssi.setText("" + strParseInt(bufArray[3]));
                                    // Total rx bits
                                    Log.v(TAG, "total rx bits> " + bufArray[4]);
                                    metxRxBits.setText("0x" + Integer.toHexString(strParseInt(bufArray[4])));
                                    // Total rx counts
                                    Log.v(TAG, "total rx counts> " + bufArray[5]);
                                    metxRxPktCounts.setText("0x" + Integer.toHexString(strParseInt(bufArray[5])));
                                    // Total rx error bits
                                    Log.v(TAG, "total rx error bits> " + bufArray[6]);
                                    metxRxErrBits.setText("0x" + Integer.toHexString(strParseInt(bufArray[6])));
                                    break;
                                case MpOpCode.BT_MP_OP_CODE_ReportTxGainTable >> 8:
                                    if (bufArray.length != 10 || strParseInt(bufArray[2]) != 0) {
                                        Log.e(TAG, "MP HCI Event Params error");
                                        break;
                                    }
                                    // gain table0
                                    Log.v(TAG, "gain table0> " + bufArray[3]);
                                    metxTxGainTable0.setText("0x" + Integer.toHexString(strParseInt(bufArray[3])));
                                    // gain table1
                                    Log.v(TAG, "gain table1> " + bufArray[4]);
                                    metxTxGainTable1.setText("0x" + Integer.toHexString(strParseInt(bufArray[4])));
                                    // gain table2
                                    Log.v(TAG, "gain table2> " + bufArray[5]);
                                    metxTxGainTable2.setText("0x" + Integer.toHexString(strParseInt(bufArray[5])));
                                    // gain table3
                                    Log.v(TAG, "gain table3> " + bufArray[6]);
                                    metxTxGainTable3.setText("0x" + Integer.toHexString(strParseInt(bufArray[6])));
                                    // gain table4
                                    Log.v(TAG, "gain table4> " + bufArray[7]);
                                    metxTxGainTable4.setText("0x" + Integer.toHexString(strParseInt(bufArray[7])));
                                    // gain table5
                                    Log.v(TAG, "gain table5> " + bufArray[8]);
                                    metxTxGainTable5.setText("0x" + Integer.toHexString(strParseInt(bufArray[8])));
                                    // gain table6
                                    Log.v(TAG, "gain table6> " + bufArray[9]);
                                    metxTxGainTable6.setText("0x" + Integer.toHexString(strParseInt(bufArray[9])));
                                    break;
                                case MpOpCode.BT_MP_OP_CODE_ReportTxDACTable >> 8:
                                    if (bufArray.length != 8 || strParseInt(bufArray[2]) != 0) {
                                        Log.e(TAG, "MP HCI Event Params error");
                                        break;
                                    }
                                    // dac table0
                                    Log.v(TAG, "dac table0> " + bufArray[3]);
                                    metxTxDacTable0.setText("0x" + Integer.toHexString(strParseInt(bufArray[3])));
                                    // dac table1
                                    Log.v(TAG, "dac table1> " + bufArray[4]);
                                    metxTxDacTable1.setText("0x" + Integer.toHexString(strParseInt(bufArray[4])));
                                    // dac table2
                                    Log.v(TAG, "dac table2> " + bufArray[5]);
                                    metxTxDacTable2.setText("0x" + Integer.toHexString(strParseInt(bufArray[5])));
                                    // dac table3
                                    Log.v(TAG, "dac table3> " + bufArray[6]);
                                    metxTxDacTable3.setText("0x" + Integer.toHexString(strParseInt(bufArray[6])));
                                    // dac table4
                                    Log.v(TAG, "dac table4> " + bufArray[7]);
                                    metxTxDacTable4.setText("0x" + Integer.toHexString(strParseInt(bufArray[7])));
                                    break;
                                case MpOpCode.BT_MP_OP_CODE_ReportXTAL >> 8:
                                    if (bufArray.length != 4 || strParseInt(bufArray[2]) != 0) {
                                        Log.e(TAG, "MP HCI Event Params error");
                                        break;
                                    }
                                    // xtal
                                    Log.v(TAG, "xtal> " + bufArray[3]);
                                    metxXtal.setText("0x" + Integer.toHexString(strParseInt(bufArray[3])));
                                    break;
                                case MpOpCode.BT_MP_OP_CODE_ReportThermal >> 8:
                                    if (bufArray.length != 4 || strParseInt(bufArray[2]) != 0) {
                                        Log.e(TAG, "MP HCI Event Params error");
                                        break;
                                    }
                                    // thermal
                                    Log.v(TAG, "thermal> " + bufArray[3]);
                                    metxThermal.setText("0x" + Integer.toHexString(strParseInt(bufArray[3])));
                                    break;
                                case MpOpCode.BT_MP_OP_CODE_ReportStage >> 8:
                                    if (bufArray.length != 4 || strParseInt(bufArray[2]) != 0) {
                                        Log.e(TAG, "MP HCI Event Params error");
                                        break;
                                    }
                                    // stage
                                    Log.v(TAG, "stage> " + bufArray[3]);
                                    metxStage.setText("0x" + Integer.toHexString(strParseInt(bufArray[3])));
                                    break;
                                default:
                                    break;
                            }
                            break;

                        case MpOpCode.BT_MP_OP_CODE_RegRW:
                            if (bufArray.length != 2 && bufArray.length != 3){
                                Log.e(TAG, "MP HCI Event Params count error");
                                break;
                            }

                            if (!bufArray[0].equals(MpOpCode.STR_BT_MP_REG_RW)) {
                                Log.e(TAG, "MP HCI Event Opcode error");
                                break;
                            }

                            if ((bufArray.length == 3) &&
                                (mspRegRW.getSelectedItemPosition() == 0)) {
                                // It's read operation, update the value
                                Log.v(TAG, "reg data> " + bufArray[2]);
                                metxRegData.setText("0x" + Integer.toHexString(strParseInt(bufArray[2])));
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
        str = new String[] { MpOpCode.BT_MP_OP_STR_GetParam, MpOpCode.BT_MP_OP_STR_SetParam,
                             MpOpCode.BT_MP_OP_STR_TestMode,
                             MpOpCode.BT_MP_OP_STR_SetTxGainTable, MpOpCode.BT_MP_OP_STR_SetTxDACTable,
                             MpOpCode.BT_MP_OP_STR_SetDefaultTxGainTable, MpOpCode.BT_MP_OP_STR_SetDefaultTxDACTable,
                             MpOpCode.BT_MP_OP_STR_SetPowerGainIndex, MpOpCode.BT_MP_OP_STR_SetPowerGain,
                             MpOpCode.BT_MP_OP_STR_SetPowerDAC, MpOpCode.BT_MP_OP_STR_SetXTAL,
                             MpOpCode.BT_MP_OP_STR_ClearReport,
                             MpOpCode.BT_MP_OP_STR_PktTxStart, MpOpCode.BT_MP_OP_STR_PktTxUpdate,
                             MpOpCode.BT_MP_OP_STR_PktContTxStart, MpOpCode.BT_MP_OP_STR_PktContTxUpdate,
                             MpOpCode.BT_MP_OP_STR_PktRxStart, MpOpCode.BT_MP_OP_STR_PktRxUpdate,
                             MpOpCode.BT_MP_OP_STR_HoppingMode,
                             MpOpCode.BT_MP_OP_STR_LETxTest, MpOpCode.BT_MP_OP_STR_LERxTest, MpOpCode.BT_MP_OP_STR_LEEndTest,
                             MpOpCode.BT_MP_OP_STR_RegRW,
                             MpOpCode.BT_MP_OP_STR_ReportTx, MpOpCode.BT_MP_OP_STR_ReportContTx, MpOpCode.BT_MP_OP_STR_ReportRx,
                             MpOpCode.BT_MP_OP_STR_ReportTxGainTable, MpOpCode.BT_MP_OP_STR_ReportTxDACTable,
                             MpOpCode.BT_MP_OP_STR_ReportXTAL, MpOpCode.BT_MP_OP_STR_ReportThermal,
                             MpOpCode.BT_MP_OP_STR_ReportStage };
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

        // Reg ID: modem, RF, Sys, BB
        str = new String[]{"Modem", "RF", "Sys", "BB"};
        items = new ArrayList<String>();
        for (int i = 0; i < str.length; i++) {
            items.add(str[i]);
        }
        adpRegID = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, items);
        adpRegID.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mspRegID.setAdapter(adpRegID);

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
        mspRegID.setOnItemSelectedListener(new SpinnerListener());
        //mspLEConnect.setOnItemSelectedListener(new SpinnerListener());
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
                        case MpOpCode.BT_MP_OP_CODE_SetParam:
                            Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_SetParam);
                            mActionParam = null;
                            // RF channel(index: 1)
                            mActionParam = "1," + mspRFChannel.getSelectedItem().toString() + ";";
                            Log.v(TAG, "rf channel> " + mActionParam);
                            // Pkt type(index: 2)
                            mActionParam = mActionParam.concat("2," + mspPktType.getSelectedItemPosition() + ";");
                            Log.v(TAG, "+pkt type> " + mActionParam);
                            // Payload type(index: 3)
                            mActionParam = mActionParam.concat("3," + mspPayloadType.getSelectedItemPosition() + ";");
                            Log.v(TAG, "+payload type> " + mActionParam);
                            // Tx pkt count(index: 4)
                            mActionParam = mActionParam.concat("4," + metxPacketCount.getText().toString() + ";");
                            Log.v(TAG, "+tx pkt count> " + mActionParam);
                            // Tx gain value(index: 5)
                            mActionParam = mActionParam.concat("5," + metxTxGainValue.getText().toString() + ";");
                            Log.v(TAG, "+tx gain value> " + mActionParam);
                            // Whitening value(index: 6)
                            mActionParam = mActionParam.concat("6," + metxWhiteningValue.getText().toString() + ";");
                            Log.v(TAG, "+whitening value> " + mActionParam);
                            // Tx gain index(index: 7)
                            mActionParam = mActionParam.concat("7," + mspTxGainIndex.getSelectedItem().toString() + ";");
                            Log.v(TAG, "tx gain index> " + mActionParam);
                            // Tx dac(index: 8)
                            mActionParam = mActionParam.concat("8," + metxTxDac.getText().toString() + ";");
                            Log.v(TAG, "+tx dac> " + mActionParam);
                            // Pkt header(index: 9)
                            mActionParam = mActionParam.concat("9," + metxPacketHeader.getText().toString() + ";");
                            Log.v(TAG, "+pkt header> " + mActionParam);
                            // Hopping fix channel(index: 10)
                            mActionParam = mActionParam.concat("10," + metxHoppingFixChannel.getText().toString() + ";");
                            Log.v(TAG, "+hopping fix channel > " + mActionParam);
                            // Hit target(index: 11), 16 radix
                            mActionParam = mActionParam.concat("11," + metxHitTarget.getText().toString() + ";");
                            Log.v(TAG, "+hit target> " + mActionParam);
                            break;
                        case MpOpCode.BT_MP_OP_CODE_Exec:
                            Log.v(TAG, "Start action: " + mActionCode + ", subAction: " + mSubActionCode);
                            switch (mSubActionCode) {
                                case MpOpCode.BT_MP_OP_CODE_SetTxGainTable:
                                    // index12, Table0
                                    mActionParam = "12," + metxTxGainTable0.getText().toString() + ",";
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
                                    // update Tx gain table param first
                                    mService.hciSend(MpOpCode.BT_MP_OP_CODE_SetParam, mActionParam, MSG_MP_ACTION_UPDATE);

                                    mActionParam = Integer.toString((mSubActionCode >> 8) & 0xFF);
                                    break;
                                case MpOpCode.BT_MP_OP_CODE_SetTxDACTable:
                                    // index13, Table0
                                    mActionParam = "13," + metxTxDacTable0.getText().toString() + ",";
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
                                    // update Tx dac table param first
                                    mService.hciSend(MpOpCode.BT_MP_OP_CODE_SetParam, mActionParam, MSG_MP_ACTION_UPDATE);

                                    mActionParam = Integer.toString((mSubActionCode >> 8) & 0xFF);
                                    break;
                                case MpOpCode.BT_MP_OP_CODE_SetXTAL:
                                    // index14, xtal
                                    mActionParam = "14," + metxTxDacTable0.getText().toString() + ",";
                                    Log.v(TAG, "xtal> " + mActionParam);
                                    // update xtal param first
                                    mService.hciSend(MpOpCode.BT_MP_OP_CODE_SetParam, mActionParam, MSG_MP_ACTION_UPDATE);

                                    mActionParam = Integer.toString((mSubActionCode >> 8) & 0xFF);
                                    break;
                                default:
                                    mActionParam = Integer.toString((mSubActionCode >> 8) & 0xFF);
                                    break;
                            }
                            break;
                        case MpOpCode.BT_MP_OP_CODE_Report:
                            Log.v(TAG, "Start action: " + mActionCode + ", subAction: " + mSubActionCode);
                            mActionParam = Integer.toString((mSubActionCode >> 8) & 0xFF);
                            break;
                        case MpOpCode.BT_MP_OP_CODE_RegRW:
                            Log.v(TAG, "Start action: " + MpOpCode.BT_MP_OP_STR_RegRW);
                            mActionParam = null;
                            // id
                            mActionParam = mspRegID.getSelectedItemPosition() + ",";
                            Log.v(TAG, "id> " + mActionParam);
                            // RW
                            mActionParam = mActionParam.concat(mspRegRW.getSelectedItemPosition() + ",");
                            Log.v(TAG, "+RW> " + mActionParam);
                            // Page, valid for BB reg
                            if (mspRegID.getSelectedItemPosition() == 3) {
                                mActionParam = mActionParam.concat(metxRegPage.getText().toString() + ",");
                                Log.v(TAG, "+page> " + mActionParam);
                            }
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
                    mService.hciSend(mActionCode & 0xFF, mActionParam, MSG_MP_ACTION_START);
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
                        mService.hciSend(mActionCode & 0xFF, mActionParam, MSG_MP_ACTION_STOP);
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
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_SetParam)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_SetParam;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_TestMode)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_TestMode;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_SetTxGainTable)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_SetTxGainTable;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_SetTxDACTable)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_SetTxDACTable;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_SetDefaultTxGainTable)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_SetDefaultTxGainTable;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_SetDefaultTxDACTable)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_SetDefaultTxDACTable;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_SetPowerGainIndex)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_SetPowerGainIndex;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_SetPowerGain)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_SetPowerGain;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_SetPowerDAC)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_SetPowerDAC;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_SetXTAL)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_SetXTAL;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_ClearReport)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_ClearReport;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_PktTxStart)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_PktTxStart;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_PktTxUpdate)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_PktTxUpdate;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_PktContTxStart)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_PktContTxStart;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_PktContTxUpdate)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_PktContTxUpdate;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_PktRxStart)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_PktRxStart;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_PktRxUpdate)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_PktRxUpdate;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_HoppingMode)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_HoppingMode;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_LETxTest)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_LETxTest;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_LERxTest)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_LERxTest;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_LEEndTest)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Exec;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_LEEndTest;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_RegRW)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_RegRW;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_ReportTx)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Report;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_ReportTx;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_ReportContTx)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Report;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_ReportContTx;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_ReportRx)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Report;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_ReportRx;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_ReportTxGainTable)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Report;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_ReportTxGainTable;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_ReportTxDACTable)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Report;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_ReportTxDACTable;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_ReportXTAL)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Report;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_ReportXTAL;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_ReportThermal)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Report;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_ReportThermal;
                    } else if (mActionItem.equals(MpOpCode.BT_MP_OP_STR_ReportStage)) {
                        mActionCode = MpOpCode.BT_MP_OP_CODE_Report;
                        mSubActionCode = MpOpCode.BT_MP_OP_CODE_ReportStage;
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

                case R.id.spinner_Register_ID:
                    Log.v(TAG, "Register ID selected: " + mspRegID.getSelectedItem().toString());
                    break;

                //case R.id.spinner_LE_Connect:
                //    Log.v(TAG, "Spinner LE Connect selected: " + mspLEConnect.getSelectedItem().toString());
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
                //case R.id.checkbox_Thermal:
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
