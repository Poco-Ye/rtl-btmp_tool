package com.android.bluetooth.mp;

import java.util.ArrayList;

import com.android.bluetooth.mp.MpOpcode;
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
    Spinner mspTestMode = null;
    Spinner mspRegID = null;
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
    EditText metxMultiRxEnable = null;
    EditText metxHoppingFixChannel = null;
    EditText metxHitTarget = null;
    EditText metxTxGainTable = null;
    EditText metxTxDacTable = null;
    EditText metxRTL8761Xtal = null;
    EditText metxGetThermal = null;
    EditText metxGetStage = null;
    EditText metxRxBits = null;
    EditText metxRxErrBits = null;
    EditText metxRxPktCounts = null;
    EditText metxTxBits = null;
    EditText metxTxPktCounts = null;

    // CheckBox
    //CheckBox cbGetThermal = null;
    //CheckBox cbLEConnect = null;
    //LargeText
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
    //ArrayAdapter<String> adpHitTarget = null;
    ArrayAdapter<String> adpLEConnect = null;

    // All variables we need to cache
    String mHciInterface = null;
    String mDevNode = null;

    // Internal Messages
    public static final int MSG_MP_STACK_STATUS = 0;
    public static final int MSG_MP_ACTION_START_RESULT = 1;
    public static final int MSG_HCI_SEND = 2;
    public static final int MSG_HCI_EVENT_BACK = 3;

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

        //Spinner
        mspHCIInterface = (Spinner) findViewById(R.id.spinner_hci_interface);
        mspActionItem = (Spinner) findViewById(R.id.spinner_action_item);
        mspRFChannel = (Spinner) findViewById(R.id.spinner_RF_Channel);
        mspPktType = (Spinner) findViewById(R.id.spinner_Pkt_Type);
        mspPayloadType = (Spinner) findViewById(R.id.spinner_Payload_Type);
        mspTxGainIndex = (Spinner) findViewById(R.id.spinner_Tx_Gain_Index);
        mspTestMode = (Spinner) findViewById(R.id.spinner_Test_Mode);
        mspRegID = (Spinner) findViewById(R.id.spinner_Register_ID);
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
        metxMultiRxEnable = (EditText) findViewById(R.id.editText_Multi_Rx_Enable);
        metxHoppingFixChannel = (EditText) findViewById(R.id.editText_Hopping_Fix_Channel);
        metxHitTarget = (EditText) findViewById(R.id.editText_Hit_Target);
        metxTxGainTable = (EditText) findViewById(R.id.editText_Tx_Gain_Table);
        metxTxDacTable = (EditText) findViewById(R.id.editText_Tx_Dac_Table);
        metxRTL8761Xtal = (EditText) findViewById(R.id.editText_RTL8761_Xtal);
        metxGetThermal = (EditText) findViewById(R.id.editText_Get_Thermal);
        metxGetStage = (EditText) findViewById(R.id.editText_Get_Stage);
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
                    if (msg.arg1 == 1) {
                        setButtonStateStarted();
                    } else {
                        new AlertDialog.Builder(MainActivity.this).
                            setMessage("MP action start failed!").
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

                case MSG_HCI_EVENT_BACK:
                    Log.v(TAG,"HCI Event has received");
                    String RxBuffer = (String)msg.obj;
                    metxLog.append("RX:");
                    metxLog.append(RxBuffer);

                    metxLog.append("\n");
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
    private void SetButtonStateStopped() {
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
        str = new String[] {"Packet Tx", "Packet Rx",
                            "Continue Tx", "Continue LE Tx"};
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
        int Tx_Gain_Index_MAX = 40;
        items = new ArrayList<String>();
        for (int i = 0; i <= Tx_Gain_Index_MAX; i++) {
            items.add(String.valueOf(i));
        }
        adpTxGainIndex = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, items);
        adpTxGainIndex.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mspTxGainIndex.setAdapter(adpTxGainIndex);

        // TestMode
        str = new String[]{"Nomal Test"};
        items = new ArrayList<String>();
        for (int i = 0; i < str.length; i++) {
            items.add(str[i]);
        }
        adpTestMode = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, items);
        adpTestMode.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mspTestMode.setAdapter(adpTestMode);

        // Reg ID: modem, RF, Sys, BB
        str = new String[]{"Modem", "RF", "Sys", "BB"};
        items = new ArrayList<String>();
        for (int i = 0; i < str.length; i++) {
            items.add(str[i]);
        }
        adpRegID = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, items);
        adpRegID.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mspRegID.setAdapter(adpRegID);


        //LE Connect
        str = new String[]{"Initiator"};
        items = new ArrayList<String>();
        for(int i = 0; i< str.length; i++) {
            items.add(str[i]);
        }
        adpLEConnect = new ArrayAdapter<String>(this,android.R.layout.simple_spinner_item,items);
        adpLEConnect.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        //mspLEConnect.setAdapter(adpLEConnect);
        //Hit Target
        //nothing

        //Set Listener
        mspHCIInterface.setOnItemSelectedListener(new SpinnerListener());
        mspActionItem.setOnItemSelectedListener(new SpinnerListener());
        mspRFChannel.setOnItemSelectedListener(new SpinnerListener());
        mspPktType.setOnItemSelectedListener(new SpinnerListener());
        mspTxGainIndex.setOnItemSelectedListener(new SpinnerListener());
        mspPayloadType.setOnItemSelectedListener(new SpinnerListener());
        mspTestMode.setOnItemSelectedListener(new SpinnerListener());
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
                mDevNode = new String("/dev/ttyS0");
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
                    Log.v(TAG, "button_Start clicked");
                    // Send action to stack

                    // buttons state update in handler
                    Message msg = new Message();
                    msg.what = MSG_MP_ACTION_START_RESULT;
                    msg.arg1 = 1;
                    updateUIHandler.sendMessage(msg);

                    break;

                case R.id.button_Stop:
                    Log.v(TAG,"button_Stop clicked");
                    SetButtonStateStopped();
                    metxLog.setText("");
                    break;

                //case R.id.button_Send:
                //    Log.v(TAG,"button_Send clicked");

                //    String mpBuffer = null;
                //    String opcodeBuffer = null;
                //    String opParaBuffer = null;
                //    int opcode = 0;
                //    Message msg;
                //    Bundle boudle;

                //    //BT_MP_OP_USER_DEF_GetPara
                //    opcode = MpOpcode.BT_MP_OP_USER_DEF_GetPara;
                //    opcodeBuffer = String.format("%x ", opcode);
                //    opParaBuffer = null;

                //    mService.hciSend(opcode, opParaBuffer);
                //    //Update UI
                //    msg = new Message();
                //    msg.what = MSG_HCI_SEND;
                //    boudle = new Bundle();
                //    mpBuffer = opcodeBuffer + opParaBuffer;
                //    boudle.putString("MP_SEND_BUF", mpBuffer);
                //    msg.setData(boudle);

                //    updateUIHandler.sendMessage(msg);
                //    /*
                //       public static final  int BT_MP_OP_USER_DEF_SetPara2 = 	0x82;
                //       public static final  int BT_MP_OP_USER_DEF_SetHit = 		0x83;
                //       public static final  int BT_MP_OP_USER_DEF_SetDacTable = 	0x84;
                //       public static final  int BT_MP_OP_USER_DEF_SetGainTable = 	0x85;
                //       public static final  int BT_MP_OP_USER_DEF_Exec = 		0x86;
                //       public static final  int BT_MP_OP_USER_DEF_ReportTx = 	0x87;
                //       public static final  int BT_MP_OP_USER_DEF_ReportRx = 	0x88;
                //     */
                //    //BT_MP_OP_USER_DEF_SetPara1
                //    opcode = MpOpcode.BT_MP_OP_USER_DEF_SetPara1;
                //    opcodeBuffer = String.format("%x ", opcode);
                //    opParaBuffer = "1,8,1,FF,A9,0";

                //    mService.hciSend(opcode, opParaBuffer);
                //    //Update UI
                //    msg = new Message();
                //    msg.what = MSG_HCI_SEND;
                //    boudle = new Bundle();
                //    mpBuffer = opcodeBuffer + opParaBuffer;
                //    boudle.putString("MP_SEND_BUF", mpBuffer);
                //    msg.setData(boudle);

                //    updateUIHandler.sendMessage(msg);

                //    break;

                //case R.id.button_Pause:
                //    //do pause here
                //    Log.v(TAG,"button_Pause clicked");

                //    //BT_MP_OP_USER_DEF_ReportTx
                //    opcode = MpOpcode.BT_MP_OP_USER_DEF_ReportTx;
                //    opcodeBuffer = String.format("%x ", opcode);
                //    opParaBuffer = null;

                //    mService.hciSend(opcode, opParaBuffer);
                //    //Update UI
                //    msg = new Message();
                //    msg.what = MSG_HCI_SEND;
                //    boudle = new Bundle();
                //    mpBuffer = opcodeBuffer + opParaBuffer;
                //    boudle.putString("MP_SEND_BUF", mpBuffer);
                //    msg.setData(boudle);

                //    updateUIHandler.sendMessage(msg);

                //    break;

                //case R.id.button_Clear:
                //    //do clear here
                //    Log.v(TAG,"button_Clear clicked");
                //    break;

                case R.id.button_Clear_Log:
                    // do clear log here
                    Log.v(TAG,"button_Clear_Log clicked");
                    break;

                case R.id.button_HCI_Reset:
                    //do hci resrt here
                    Log.v(TAG,"button_HCI_Reset clicked");
                    break;

                default:
                    break;
            }
        }
    }

    //Spinner listener implementation here
    class SpinnerListener implements OnItemSelectedListener {
        @Override
        public void onItemSelected(AdapterView<?> arg0, View v, int arg2,long arg3) {
            switch (arg0.getId()) {
                case R.id.spinner_hci_interface:
                    Log.v(TAG,"Spinner HCI Interface selected: " + mspHCIInterface.getSelectedItem().toString());
                    mHciInterface = mspHCIInterface.getSelectedItem().toString();
                    break;

                case R.id.spinner_action_item:
                    Log.v(TAG,"Spinner Action Item selected: " + mspActionItem.getSelectedItem().toString());
                    break;

                case R.id.spinner_RF_Channel:
                    Log.v(TAG,"Spinner RF Channel selected: " + mspRFChannel.getSelectedItem().toString());
                    break;

                case R.id.spinner_Pkt_Type:
                    Log.v(TAG,"Spinner Packet Type selected: " + mspPktType.getSelectedItem().toString());
                    break;

                case R.id.spinner_Payload_Type:
                    Log.v(TAG,"Spinner payload Type selected: " + mspPayloadType.getSelectedItem().toString());
                    break;

                case R.id.spinner_Tx_Gain_Index:
                    Log.v(TAG,"Spinner Tx Gain Index selected: " + mspTxGainIndex.getSelectedItem().toString());
                    break;

                case R.id.spinner_Test_Mode:
                    Log.v(TAG,"Spinner Test Mode selected: " + mspTestMode.getSelectedItem().toString());
                    break;

                case R.id.spinner_Register_ID:
                    Log.v(TAG,"Register ID selected: " + mspRegID.getSelectedItem().toString());
                    break;

                //case R.id.spinner_LE_Connect:
                //    Log.v(TAG,"Spinner LE Connect selected: " + mspLEConnect.getSelectedItem().toString());
                //    break;

                //case R.id.spinner_Hit_Target:
                //    Log.v(TAG,"Spinner Hit Target selected: " + mspHitTarget.getSelectedItem().toString());
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
