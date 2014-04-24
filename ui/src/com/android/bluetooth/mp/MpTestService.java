/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @hide
 */

package com.android.bluetooth.mp;

import android.app.Application;
import android.app.Service;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.ParcelFileDescriptor;
import android.os.ParcelUuid;
import android.os.Process;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.provider.Settings;
import android.util.Log;
import android.util.Pair;

import java.io.FileDescriptor;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Set;
import java.util.Map;
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.List;
import android.content.pm.PackageManager;
//import android.os.ServiceManager;


public class MpTestService extends Service {
    private static final String TAG = "BluetoothMpTestService";
    //private static final boolean DBG = false;
    private static final boolean DBG = true;
    private Handler mHandler = null;
    // Binder given to clients
    private final IBinder mBinder = new MpTestServiceBinder();

    /**
     * Class used for the client Binder.  Because we know this service always
     * runs in the same process as its clients, we don't need to deal with IPC.
     */
    public class MpTestServiceBinder extends Binder {
        MpTestService getService() {
            // Return this instance of LocalService so clients can call public methods
            return MpTestService.this;
        }
    }

    //get the handler from MainActivity
    public void RegisterHandler(Handler handler) {
        mHandler = handler;
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");

        super.onCreate();
        //boolean ret = true;
        classInitNative();
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind");

        return mBinder;
    }

    public boolean onUnbind(Intent intent) {
        Log.d(TAG, "onUnbind");

        return super.onUnbind(intent);
    }

    public void onDestroy() {
        Log.d(TAG, "onDestroy");

        disableNative();
        cleanupNative();
    }

    //1--start sucessfully 0--start failed
    void stateChangeCallback(int status) {
        Log.d(TAG, "stateChangeCallback, status" + status);
        //send the start result to MainActivity
        Message msg = new Message();
        msg.what = MainActivity.MSG_START_RESULT;
        msg.arg1 = status;
        mHandler.sendMessage(msg);
    }

    //HCI event call back
    void dut_mode_recv(byte opcode, String data) {
        Log.d(TAG, "dut_mode_recv, opcode:" + opcode + "data:");

        Message msg = new Message();
        msg.what = MainActivity.MSG_HCI_EVENT_BACK;
        msg.obj = data;
        mHandler.sendMessage(msg);
    }

    public boolean enableMpTestMode() {
        Log.d(TAG, "enableMpTestMode");
        boolean ret = true;
        initNative();
        ret = enableNative();

        return ret;

    }

    public boolean disableMpTestMode() {
        Log.d(TAG, "disableMpTestMode");
        boolean ret = true;

        ret = disableNative();
        cleanupNative();
        return ret;
    }

    public int hciSend(int opcode, String data) {
        Log.d(TAG,"service send hci command");
        return hciSendNative(opcode, data);
    }

    native void classInitNative();
    native boolean initNative();
    native void cleanupNative();
    native boolean enableNative();
    native boolean disableNative();
    native int hciSendNative(int opcode, String data);
    native boolean dutModeConfigureNative(int configure);
}

