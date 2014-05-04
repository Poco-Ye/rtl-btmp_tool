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
    //Button
    Button mbuttonStart = null;
    Button mbuttonStop = null;
    //Button mbuttonSend = null;
    //Button mbuttonPause = null;
    //Button mbuttonClear = null;
    Button mbuttonClearLog = null;
    Button mbuttonHCIReset = null;
    Button mbuttonTestMode= null;
    Button mbuttonRegRw = null;
    Button mbuttonHostReInit = null;
    //Spineer
    Spinner mspHCIInterface = null;
    Spinner mspActionItem = null;
    Spinner mspDataRate = null;
    Spinner mspRFChannel = null;
    Spinner mspTxGainIndex = null;
    Spinner mspTestMode = null;
    Spinner mspHitTarget = null;
    Spinner mspPayloadType = null;
    Spinner mspTxDACCurrent = null;
    //Spinner mspLEConnect = null;
    //EditText
    EditText metxDevNode = null;
    //EditText metxRxPacketsCount = null;
    //EditText metxErrorBits = null;
    //EditText metxBER = null;
    EditText metxTxPacketsCount = null;
    EditText metxPayloadBits = null;
    EditText metxPacketHeader = null;
    //EditText metxWhiteningCoeff = null;
    //CheckBox
    //CheckBox cbUploadThermal = null;
    //CheckBox cbLEConnect = null;
    //CheckBox cbWhiteningCoeff = null;
    //LargeText
    TextView metxLog = null;
    //Spinner Adapter
    ArrayAdapter<String> adpHCIInterface = null;
    ArrayAdapter<String> adpActionItem = null;
    ArrayAdapter<String> adpDataRate = null;
    ArrayAdapter<String> adpRFChannel = null;
    ArrayAdapter<String> adpTxGainIndex = null;
    ArrayAdapter<String> adpPayloadType = null;
    ArrayAdapter<String> adpTestMode = null;
    ArrayAdapter<String> adpHitTarget = null;
    ArrayAdapter<String> adpLEConnect = null;
    ArrayAdapter<String> adpTxDACCurrent = null;

    //Message
    public static final int MSG_START_RESULT = 0;
    public static final int MSG_HCI_SEND = 1;
    public static final int MSG_HCI_EVENT_BACK = 2;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG,"onCreate");

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        //Button
        mbuttonStart = (Button) findViewById(R.id.button_Start);
        mbuttonStop = (Button) findViewById(R.id.button_Stop);
        //mbuttonSend = (Button) findViewById(R.id.button_Send);
        //mbuttonPause = (Button) findViewById(R.id.button_Pause);
        //mbuttonClear = (Button) findViewById(R.id.button_Clear);
        mbuttonClearLog = (Button) findViewById(R.id.button_Clear_Log);
        mbuttonHCIReset = (Button) findViewById(R.id.button_HCI_Reset);
        mbuttonTestMode = (Button) findViewById(R.id.button_Test_Mode);
        mbuttonRegRw = (Button) findViewById(R.id.button_Reg_RW);
        mbuttonHostReInit = (Button) findViewById(R.id.button_Host_ReInit);
        //Button Listener
        mbuttonStart.setOnClickListener(new ButtonClick());
        mbuttonStop.setOnClickListener(new ButtonClick());
        //mbuttonSend.setOnClickListener(new ButtonClick());
        //mbuttonPause.setOnClickListener(new ButtonClick());
        //mbuttonClear.setOnClickListener(new ButtonClick());
        mbuttonClearLog.setOnClickListener(new ButtonClick());
        mbuttonHCIReset.setOnClickListener(new ButtonClick());
        mbuttonTestMode.setOnClickListener(new ButtonClick());
        mbuttonRegRw.setOnClickListener(new ButtonClick());
        mbuttonHostReInit.setOnClickListener(new ButtonClick());
        //Button Enable
        SetButtonStateBeforStart();

        //Spinner
        mspHCIInterface = (Spinner) findViewById(R.id.spinner_hci_interface);
        mspActionItem =  (Spinner) findViewById(R.id.spinner_action_item);
        mspDataRate =  (Spinner) findViewById(R.id.spinner_Data_Rate);
        mspRFChannel =  (Spinner) findViewById(R.id.spinner_RF_Channel);
        mspTxGainIndex =  (Spinner) findViewById(R.id.spinner_Tx_Gain_Index);
        mspTestMode  =  (Spinner) findViewById(R.id.spinner_Test_Mode);
        mspHitTarget =  (Spinner) findViewById(R.id.spinner_Hit_Target);
        mspPayloadType =  (Spinner) findViewById(R.id.spineer_Payload_Type);
        mspTxDACCurrent =  (Spinner) findViewById(R.id.spinner_Tx_DAC_current);
        //mspLEConnect=  (Spinner) findViewById(R.id.spinner_LE_Connect);
        //add intems and listener to all spinners
        InitAllSpinners();

        //EditText
        metxDevNode = (EditText) findViewById(R.id.editText_Dev_Node);
        //metxRxPacketsCount = (EditText) findViewById(R.id.editText_Rx_Packets);
        //metxErrorBits = (EditText) findViewById(R.id.editText_ErrorBits);
        //metxBER = (EditText) findViewById(R.id.editText_BER);
        metxTxPacketsCount = (EditText) findViewById(R.id.editText_Tx_Packets_Count);
        metxPayloadBits = (EditText) findViewById(R.id.editText_Payload_Bits);
        metxPacketHeader = (EditText) findViewById(R.id.editText_Packet_Header);
        //metxWhiteningCoeff = (EditText) findViewById(R.id.editText_Whitening_Coeff);

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
        metxTxPacketsCount.setText("xxx");
        metxTxPacketsCount.setKeyListener(null);
        metxPayloadBits.setText("xxx");
        metxPacketHeader.setText("xxx");
        //metxWhiteningCoeff.setText("xxx");

        //LargeText
        metxLog = (TextView) findViewById (R.id.textView_Log);
        metxLog.setMovementMethod(ScrollingMovementMethod.getInstance());

        //checkBox
        //cbUploadThermal = (CheckBox)findViewById(R.id.checkbox_upload_thermal);
        //cbLEConnect = (CheckBox)findViewById(R.id.checkbox_LE_Connect);
        //cbWhiteningCoeff = (CheckBox)findViewById(R.id.checkbox_Whitening_Coeff);
        //add listener to all CheckBoxs
        //cbUploadThermal.setOnCheckedChangeListener(new CheckBoxListener());
        //cbLEConnect.setOnCheckedChangeListener(new CheckBoxListener());
        //cbWhiteningCoeff.setOnCheckedChangeListener(new CheckBoxListener());
    }

    //Update UI here
    Handler updateUIHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                //start result is sent here
                case MSG_START_RESULT:
                    Log.v(TAG,"Start result has been received!");
                    metxLog.append("Start result: " + String.valueOf(msg.arg1) + "\n");
                    //start successfully
                    if (msg.arg1 == 1) {
                        SetButtonStateAfterStart();
                    } else {
                        new AlertDialog.Builder(MainActivity.this).
                            setMessage("Strat failed!").
                            create().
                            show();
                    }
                    break;
                    //HCI command send
                case MSG_HCI_SEND:
                    Log.v(TAG,"HCI Command has sent");
                    String msgBuffer = msg.getData().getString("MP_SEND_BUF");
                    if (msgBuffer != null) {
                        metxLog.append("TX:");
                        metxLog.append(msgBuffer);
                        metxLog.append("\n");
                    }
                    break;
                    //HCI event received
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

    //before start,only start button is available
    //called in function onCreate()
    private void SetButtonStateBeforStart() {
        mbuttonStart.setEnabled(true);
        mbuttonStop.setEnabled(false);
        //mbuttonSend.setEnabled(false);
        //mbuttonPause.setEnabled(false);
        //mbuttonClear.setEnabled(false);
        mbuttonClearLog.setEnabled(true);
        mbuttonHCIReset.setEnabled(false);
        mbuttonTestMode.setEnabled(false);
        mbuttonRegRw.setEnabled(false);
        mbuttonHostReInit.setEnabled(false);
    }

    //after start,some buttons is available
    //called in start button response
    private void SetButtonStateAfterStart() {
        mbuttonStop.setEnabled(true);
        //mbuttonSend.setEnabled(true);
        //mbuttonPause.setEnabled(true);
        //mbuttonClear.setEnabled(true);
        mbuttonClearLog.setEnabled(true);
        mbuttonHCIReset.setEnabled(true);
        mbuttonTestMode.setEnabled(true);
        mbuttonRegRw.setEnabled(true);
        mbuttonHostReInit.setEnabled(true);
    }

    //call in stop response
    private void SetButtonStateStoped() {
        mbuttonStart.setEnabled(true);
        mbuttonStop.setEnabled(false);
        //mbuttonSend.setEnabled(false);
        //mbuttonPause.setEnabled(false);
        //mbuttonClear.setEnabled(false);
        mbuttonClearLog.setEnabled(false);
        mbuttonHCIReset.setEnabled(false);
        mbuttonTestMode.setEnabled(false);
        mbuttonRegRw.setEnabled(false);
        mbuttonHostReInit.setEnabled(false);
        metxLog.clearAnimation();
    }

    //add items and listeners to all spinners
    private void InitAllSpinners() {
        ArrayList<String> items;
        String[] str;

        // hci interfaces
        items = new ArrayList<String>();
        str = new String[] {"UART", "USB"};
        for (int i = 0; i < str.length; i++) {
            items.add(str[i]);
        }
        adpHCIInterface = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, items);
        adpHCIInterface.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mspHCIInterface.setAdapter(adpHCIInterface);

        //action items
        items = new ArrayList<String>();
        str = new String[] {"Packet Tx", "Packet Rx",
                            "Continue Tx", "Continue LE Tx"};
        for (int i = 0; i < str.length; i++) {
            items.add(str[i]);
        }
        adpActionItem = new ArrayAdapter<String>(this,android.R.layout.simple_spinner_item, items);
        adpActionItem.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mspActionItem.setAdapter(adpActionItem);

        //data rate
        str = new String[] {"1M","2M", "3M", "LE"};
        items = new ArrayList<String>();
        for(int i = 0; i<str.length; i++) {
            items.add(str[i]);
        }
        adpDataRate = new ArrayAdapter<String>(this,android.R.layout.simple_spinner_item,items);
        adpDataRate.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mspDataRate.setAdapter(adpDataRate);
        //Rf channel
        int RF_CHANNEL_MAX = 80;
        items = new ArrayList<String>();
        for(int i = 0; i<= RF_CHANNEL_MAX; i++) {
            items.add(String.valueOf(i));
        }
        adpRFChannel = new ArrayAdapter<String>(this,android.R.layout.simple_spinner_item,items);
        adpRFChannel.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mspRFChannel.setAdapter(adpRFChannel);
        //Tx Gain Index
        int Tx_Gain_Index_MAX = 40;
        items = new ArrayList<String>();
        for(int i = 0; i<= Tx_Gain_Index_MAX; i++) {
            items.add(String.valueOf(i));
        }
        adpTxGainIndex = new ArrayAdapter<String>(this,android.R.layout.simple_spinner_item,items);
        adpTxGainIndex.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mspTxGainIndex.setAdapter(adpTxGainIndex);
        //Payload Type
        str = new String[]{"All_0","All_1", "0101", "1010", "0x0~0xf"};
        items = new ArrayList<String>();
        for(int i = 0; i< str.length; i++) {
            items.add(str[i]);
        }
        adpPayloadType = new ArrayAdapter<String>(this,android.R.layout.simple_spinner_item,items);
        adpPayloadType.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mspPayloadType.setAdapter(adpPayloadType);
        //TestMode
        str = new String[]{"Nomal Test"};
        items = new ArrayList<String>();
        for(int i = 0; i< str.length; i++) {
            items.add(str[i]);
        }
        adpTestMode = new ArrayAdapter<String>(this,android.R.layout.simple_spinner_item,items);
        adpTestMode.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mspTestMode.setAdapter(adpTestMode);
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
        //Tx DAC Current
        int TX_MAX_CURRENT = 5;
        items = new ArrayList<String>();
        for(int i = 1; i<= TX_MAX_CURRENT; i++) {
            items.add(String.valueOf(i));
        }
        adpTxDACCurrent = new ArrayAdapter<String>(this,android.R.layout.simple_spinner_item,items);
        adpTxDACCurrent.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mspTxDACCurrent.setAdapter(adpTxDACCurrent);

        //Set Listener
        mspHCIInterface.setOnItemSelectedListener(new SpinnerListener());
        mspActionItem.setOnItemSelectedListener(new SpinnerListener());
        mspDataRate.setOnItemSelectedListener(new SpinnerListener());
        mspRFChannel.setOnItemSelectedListener(new SpinnerListener());
        mspTxGainIndex.setOnItemSelectedListener(new SpinnerListener());
        mspPayloadType.setOnItemSelectedListener(new SpinnerListener());
        mspTestMode.setOnItemSelectedListener(new SpinnerListener());
        //mspLEConnect.setOnItemSelectedListener(new SpinnerListener());
        mspHitTarget.setOnItemSelectedListener(new SpinnerListener());
        mspTxDACCurrent.setOnItemSelectedListener(new SpinnerListener());
    }

    //Button response implementation here
    class ButtonClick implements OnClickListener {
        public void onClick(View v) {
            switch (v.getId()) {
                case R.id.button_Start:
                    Log.v(TAG,"button_Start clicked");
                    mService.enableMpTestMode();
                    mbuttonStart.setEnabled(false);

                    break;

                case R.id.button_Stop:
                    SetButtonStateStoped();
                    mService.disableMpTestMode();
                    metxLog.setText("");
                    Log.v(TAG,"button_Stop clicked");
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
                    //do clear log here
                    Log.v(TAG,"button_Clear_Log clicked");
                    break;

                case R.id.button_HCI_Reset:
                    //do hci resrt here
                    Log.v(TAG,"button_HCI_Reset clicked");
                    break;

                case R.id.button_Test_Mode:
                    //test mode
                    Log.v(TAG,"button_Test_Mode clicked");
                    break;

                case R.id.button_Reg_RW:
                    //do read reg here
                    Log.v(TAG,"button_Reg_RW clicked");
                    break;

                case R.id.button_Host_ReInit:
                    //do reinit here
                    Log.v(TAG,"button_Host_ReInit clicked");
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
                    break;

                case R.id.spinner_action_item:
                    Log.v(TAG,"Spinner Action Item selected: " + mspActionItem.getSelectedItem().toString());
                    break;

                case R.id.spinner_Data_Rate:
                    Log.v(TAG,"Spinner Data Rate selected: " + mspDataRate.getSelectedItem().toString());
                    break;

                case R.id.spinner_RF_Channel:
                    Log.v(TAG,"Spinner RF Channel selected: " + mspRFChannel.getSelectedItem().toString());
                    break;

                case R.id.spinner_Tx_Gain_Index:
                    Log.v(TAG,"Spinner Tx Gain Index selected: " + mspTxGainIndex.getSelectedItem().toString());
                    break;

                case R.id.spineer_Payload_Type:
                    Log.v(TAG,"Spinner payload Type selected: " + mspPayloadType.getSelectedItem().toString());
                    break;

                case R.id.spinner_Test_Mode:
                    Log.v(TAG,"Spinner Test Mode selected: " + mspTestMode.getSelectedItem().toString());
                    break;

                //case R.id.spinner_LE_Connect:
                //    Log.v(TAG,"Spinner LE Connect selected: " + mspLEConnect.getSelectedItem().toString());
                //    break;

                case R.id.spinner_Hit_Target:
                    Log.v(TAG,"Spinner Hit Target selected: " + mspHitTarget.getSelectedItem().toString());
                    break;

                case R.id.spinner_Tx_DAC_current:
                    Log.v(TAG,"Spinner Tx DAC Current selected: " + mspTxDACCurrent.getSelectedItem().toString());
                    break;

            }
        }
        @Override
        public void onNothingSelected(AdapterView<?> arg0) {
            // TODO Auto-generated method stub
            Log.v(TAG, "Spinner select nothing");
        }
    }

    //CheckBox Listener implementation here
    class CheckBoxListener implements OnCheckedChangeListener {
        @Override
        public void onCheckedChanged(CompoundButton bv, boolean isChecked) {
            switch (bv.getId()) {
                //case R.id.checkbox_upload_thermal:
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

                //case R.id.checkbox_Whitening_Coeff:
                //    if(isChecked)
                //    {
                //        Log.v(TAG, "Whitening Coeff Checkbox is selected!");
                //    }
                //    else
                //    {
                //        Log.v(TAG, "Whitening Coeff Checkbox is canceled!");
                //    }
                //    break;
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
            //mbuttonSend.setEnabled(true);
        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            Log.d(TAG,"onServiceDisconnected");

            mBound = false;
            //mbuttonStart.setEnabled(false);
            //mbuttonStop.setEnabled(false);
            //mbuttonSend.setEnabled(false);
        }
    };
}
