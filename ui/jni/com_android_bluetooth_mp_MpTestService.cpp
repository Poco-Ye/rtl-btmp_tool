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

#define LOG_TAG "BluetoothMpTestJni"
#include "JNIHelp.h"
#include "jni.h"

#include "bluetoothmp.h"
#include "hardware/hardware.h"

#include "utils/Log.h"
#include "utils/misc.h"
#include "cutils/properties.h"
#include "android_runtime/AndroidRuntime.h"
//#include "android_runtime/Log.h"

#include <string.h>
#include <pthread.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <cutils/properties.h>


namespace android {


static jmethodID method_stateChangeCallback;
static jmethodID method_dut_mode_recv;

static const bt_interface_t *sBluetoothInterface = NULL;

static JNIEnv *callbackEnv = NULL;

static jobject sJniCallbacksObj;

const bt_interface_t* getBluetoothInterface() {
    return sBluetoothInterface;
}

JNIEnv* getCallbackEnv() {
   return callbackEnv;
}

void checkAndClearExceptionFromCallback(JNIEnv* env,
                                               const char* methodName) {
    if (env->ExceptionCheck()) {
        ALOGE("An exception was thrown by callback '%s'.", methodName);
//        LOGE_EX(env);
        env->ExceptionClear();
    }
}

static bool checkCallbackThread() {
    JNIEnv* env = AndroidRuntime::getJNIEnv();
    if (callbackEnv != env || callbackEnv == NULL) {
        ALOGE("Callback env check fail: env: %p, callback: %p", env, callbackEnv);
        return false;
    }
    return true;
}

static void adapter_state_change_callback(bt_state_t status) {
    if (!checkCallbackThread()) {
       ALOGE("Callback: '%s' is not called on the correct thread", __FUNCTION__);
       return;
    }
    ALOGV("%s: Status is: %d", __FUNCTION__, status);

    callbackEnv->CallVoidMethod(sJniCallbacksObj, method_stateChangeCallback, (jint)status);

    checkAndClearExceptionFromCallback(callbackEnv, __FUNCTION__);
}

static void dut_mode_recv(uint8_t evtcode, char *buf)
{
    jstring dataBuffer = NULL;
    jclass strClass = NULL;
    jmethodID ctorID = NULL;
    jbyteArray byteBuffer = NULL;
    jstring encode = NULL;
    int strLen = 0;

    ALOGI("%s: opCode 0x%02x, buf[%s]", __FUNCTION__, evtcode, buf);

    if (!checkCallbackThread()) {
        ALOGE("Callback: '%s' is not called on the correct thread", __FUNCTION__);
        return;
    }

    strClass = callbackEnv->FindClass("java/lang/String");
    ctorID = callbackEnv->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    encode =callbackEnv->NewStringUTF("utf-8");
    if (encode == NULL) goto Fail;

    strLen = strlen(buf);

    byteBuffer = callbackEnv->NewByteArray(strLen);
    if (byteBuffer == NULL) goto Fail;

    callbackEnv->SetByteArrayRegion(byteBuffer, 0, strLen, (jbyte*)buf);
    dataBuffer = (jstring)callbackEnv->NewObject(strClass, ctorID, byteBuffer, encode);

    callbackEnv->CallVoidMethod(sJniCallbacksObj, method_dut_mode_recv, (jint)evtcode, dataBuffer);

    checkAndClearExceptionFromCallback(callbackEnv, __FUNCTION__);

Fail:
    if (byteBuffer) callbackEnv->DeleteLocalRef(byteBuffer);
    if (dataBuffer) callbackEnv->DeleteLocalRef(dataBuffer);
}

static void callback_thread_event(bt_cb_thread_evt event) {
    JavaVM* vm = AndroidRuntime::getJavaVM();
    if (event  == ASSOCIATE_JVM) {
        JavaVMAttachArgs args;
        char name[] = "BT MP test Service Callback Thread";
        //TODO(BT)
        args.version = JNI_VERSION_1_6;
        args.name = name;
        args.group = NULL;
        vm->AttachCurrentThread(&callbackEnv, &args);
        ALOGV("Callback thread attached: %p", callbackEnv);
    } else if (event == DISASSOCIATE_JVM) {
        if (!checkCallbackThread()) {
            ALOGE("Callback: '%s' is not called on the correct thread", __FUNCTION__);
            return;
        }
        vm->DetachCurrentThread();
    }
}

bt_callbacks_t sBluetoothCallbacks = {
    sizeof(sBluetoothCallbacks),
    adapter_state_change_callback, /* adapter_state_changed_cb */
    callback_thread_event, /* thread_evt_cb */
    dut_mode_recv, /* dut_mode_recv_cb */
};

static void classInitNative(JNIEnv* env, jclass clazz) {
    int err;
    hw_module_t* module;
    ALOGI("%s", __FUNCTION__);

    jclass jniCallbackClass =
        env->FindClass("com/android/bluetooth/mp/MpTestService");

    method_stateChangeCallback = env->GetMethodID(jniCallbackClass, "stateChangeCallback", "(I)V");
    method_dut_mode_recv = env->GetMethodID(jniCallbackClass, "dut_mode_recv", "(ILjava/lang/String;)V");

    err = hw_get_module(BT_STACK_MODULE_ID, (hw_module_t const**)&module);

    if (err == 0) {
        hw_device_t* abstraction;
        err = module->methods->open(module, BT_STACK_MODULE_ID, &abstraction);
        if (err == 0) {
            bluetooth_module_t* btStack = (bluetooth_module_t *)abstraction;
            sBluetoothInterface = btStack->get_bluetooth_interface();
        } else {
           ALOGE("Error while opening Bluetooth library");
        }
    } else {
        ALOGE("No Bluetooth Library found");
    }
}

static bool initNative(JNIEnv* env, jobject obj, jstring interface, jstring node) {
    int ret;
    bt_hci_if_t if_type = BT_HCI_IF_NONE;
    const char *if_str = NULL;
    const char *node_str = NULL;

    if (sJniCallbacksObj != NULL) {
         env->DeleteGlobalRef(sJniCallbacksObj);
         sJniCallbacksObj = NULL;
    }
    sJniCallbacksObj = env->NewGlobalRef(obj);

    if_str = env->GetStringUTFChars(interface, NULL);
    node_str = env->GetStringUTFChars(node, NULL);

    ALOGI("%s: interface %s, node %s", __FUNCTION__, if_str, node_str);

    if (!strcasecmp(if_str, "UART")) {
        if_type = BT_HCI_IF_UART;
    } else if (!strcasecmp(if_str, "USB")) {
        if_type = BT_HCI_IF_USB;
    }

    if (sBluetoothInterface) {
        ret = sBluetoothInterface->init(&sBluetoothCallbacks, if_type, node_str);
        if (ret != BT_STATUS_SUCCESS) {
            ALOGE("Error while setting the callbacks");
            sBluetoothInterface = NULL;
            return JNI_FALSE;
        }

        return JNI_TRUE;
    }
    return JNI_FALSE;
}

static bool cleanupNative(JNIEnv *env, jobject obj) {
    ALOGI("%s", __FUNCTION__);

    jboolean result = JNI_FALSE;
    if (!sBluetoothInterface) return result;

    sBluetoothInterface->cleanup();
    ALOGI("%s: return from cleanup", __FUNCTION__);

    if (sJniCallbacksObj != NULL) {
         env->DeleteGlobalRef(sJniCallbacksObj);
         sJniCallbacksObj = NULL;
    }

    return JNI_TRUE;
}

static jboolean enableNative(JNIEnv* env, jobject obj) {
    ALOGI("%s", __FUNCTION__);

    jboolean result = JNI_FALSE;
    if (!sBluetoothInterface) return result;

    int ret = sBluetoothInterface->enable();
    result = (ret == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;

    return result;
}

static jboolean disableNative(JNIEnv* env, jobject obj) {
    ALOGI("%s", __FUNCTION__);

    jboolean result = JNI_FALSE;
    if (!sBluetoothInterface) return result;

    int ret = sBluetoothInterface->disable();
    result = (ret == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;

    return result;
}


static jint hciSendNative(JNIEnv* env, jobject obj, jint opcode, jstring data)
{
    jint bytesSend = 0;
    const char *input_data = NULL;

    if (!sBluetoothInterface) return bytesSend;


    if (data != NULL) {
        input_data = env->GetStringUTFChars(data, NULL);
    }

    ALOGI("%s: opCode 0x%02x, buf[%s]", __FUNCTION__, opcode, input_data);

    bytesSend = sBluetoothInterface->hal_mp_op_send(opcode, (char*)input_data);

    if (input_data != NULL) {
        env->ReleaseStringUTFChars(data, input_data);
    }

    return bytesSend;
}

static jboolean dutModeConfigureNative(JNIEnv *env, jobject obj, jint type) {
    ALOGI("%s", __FUNCTION__);

    jboolean result = JNI_FALSE;
    if (!sBluetoothInterface) return result;

    int ret = sBluetoothInterface->dut_mode_configure( type);
    result = (ret == BT_STATUS_SUCCESS) ? JNI_TRUE : JNI_FALSE;

    return result;
}

static JNINativeMethod sMethods[] = {
     /* name, signature, funcPtr */
    {"classInitNative", "()V", (void *) classInitNative},
    {"initNative", "(Ljava/lang/String;Ljava/lang/String;)Z", (void *) initNative},
    {"cleanupNative", "()V", (void*) cleanupNative},
    {"enableNative", "()Z",  (void*) enableNative},
    {"disableNative", "()Z",  (void*) disableNative},
    {"hciSendNative", "(ILjava/lang/String;)I", (void*)hciSendNative},
    {"dutModeConfigureNative", "(I)Z", (void*) dutModeConfigureNative}

};

int register_com_android_bluetooth_mp_MpTestService(JNIEnv* env)
{
    return jniRegisterNativeMethods(env, "com/android/bluetooth/mp/MpTestService",
                                    sMethods, NELEM(sMethods));
}

} /* namespace android */


/*
 * JNI Initialization
 */
jint JNI_OnLoad(JavaVM *jvm, void *reserved)
{
   JNIEnv *e;
   int status;

   ALOGI("Bluetooth MpTest Service: loading JNI");

   // Check JNI version
   if(jvm->GetEnv((void **)&e, JNI_VERSION_1_6)) {
       ALOGE("JNI version mismatch error");
      return JNI_ERR;
   }

   if ((status = android::register_com_android_bluetooth_mp_MpTestService(e)) < 0) {
       ALOGE("jni mptest service registration failure, status: %d", status);
      return JNI_ERR;
   }

   return JNI_VERSION_1_6;
}
