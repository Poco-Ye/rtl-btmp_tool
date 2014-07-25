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

#ifndef BLUETOOTH_MP_H
#define BLUETOOTH_MP_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <hardware/hardware.h>


__BEGIN_DECLS
/**
 * The Bluetooth Hardware Module ID
 */
#define BT_HARDWARE_MODULE_ID "bluetoothmp"
#define BT_STACK_MODULE_ID "bluetoothmp"
#define BT_STACK_TEST_MODULE_ID "bluetoothmp_test"

/** MP opcode */
enum BT_MP_OPCODE {

    BT_MP_OP_HCI_SEND_CMD = 0x00,
    BT_MP_OP_DUT_MODE_CONFIGURE = 0x01,

    BT_MP_OP_USER_DEF_GetParam = 0x10,
    BT_MP_OP_USER_DEF_SetParam = 0x11,
    BT_MP_OP_USER_DEF_SetParam1 = 0x12,
    BT_MP_OP_USER_DEF_SetParam2 = 0x13,
    BT_MP_OP_USER_DEF_SetConfig = 0x14,
    BT_MP_OP_USER_DEF_SetGainTable = 0x15,
    BT_MP_OP_USER_DEF_SetDacTable = 0x16,
    BT_MP_OP_USER_DEF_Exec = 0x17,
    BT_MP_OP_USER_DEF_ReportTx = 0x18,
    BT_MP_OP_USER_DEF_ReportContTx = 0x19,
    BT_MP_OP_USER_DEF_ReportRx = 0x1a,
    BT_MP_OP_USER_DEF_RegRW = 0x1b,

};

/** MP opcode string */
#define STR_BT_MP_ENABLE        "enable"
#define STR_BT_MP_DISABLE       "disable"
#define STR_BT_MP_DUT_MODE      "dut_mode_configure"

#define STR_BT_MP_HCI_CMD           "bt_mp_HciCmd"
#define STR_BT_MP_GET_PARAM         "bt_mp_GetParam"
#define STR_BT_MP_SET_PARAM         "bt_mp_SetParam"
#define STR_BT_MP_SET_PARAM1        "bt_mp_SetParam1"
#define STR_BT_MP_SET_PARAM2        "bt_mp_SetParam2"
#define STR_BT_MP_SET_CONFIG        "bt_mp_SetConfig"
#define STR_BT_MP_SET_GAIN_TABLE    "bt_mp_SetGainTable"
#define STR_BT_MP_SET_DAC_TABLE     "bt_mp_SetDacTable"
#define STR_BT_MP_EXEC              "bt_mp_Exec"
#define STR_BT_MP_REPORT_TX         "bt_mp_ReportTx"
#define STR_BT_MP_REPORT_CONT_TX    "bt_mp_ReportContTx"
#define STR_BT_MP_REPORT_RX         "bt_mp_ReportRx"
#define STR_BT_MP_REG_RW            "bt_mp_RegRW"

#define STR_BT_MP_HCI_RESET     "bt_mp_HciReset"
#define STR_BT_MP_CONTTXSTART   "bt_mp_ConTxStart"
#define STR_BT_MP_CONTTXSTOP    "bt_mp_ConTxStop"
#define STR_BT_MP_PKTTXSTART    "bt_mp_PktTxStart"
#define STR_BT_MP_PKTTXSTOP     "bt_mp_PktTxStop"
#define STR_BT_MP_PKTRXSTART    "bt_mp_PktRxStart"
#define STR_BT_MP_PKTRXSTOP     "bt_mp_PktRxStop"

#define STR_BT_MP_PARAM_DELIM        ","
#define STR_BT_MP_RESULT_DELIM       ","
#define STR_BT_MP_PAIR_DELIM         ";"

#define STR_BT_SUCCESS "Success"
#define STR_BT_FAILED "Failed"

#define STR_BT_NOT_ENABLED "BT not enabled"
#define STR_BT_HAS_ENABLED "BT has enabled"
#define STR_BT_HAS_DISABLED "BT has disabled"

typedef enum {
    BT_FUNCTION_SUCCESS = 0,
    FUNCTION_ERROR,

    FUNCTION_HCISEND_ERROR,
    FUNCTION_HCISEND_STAUTS_ERROR,
    FUNCTION_PARAMETER_ERROR,
    FUNCTION_PARAMETER_INVALID_CHANNEL,
    FUNCTION_NO_SUPPORT,
    FUNCTION_TRX_STATUS_ERROR,
    FUNCTION_RX_RUNNING,
    FUNCTION_TX_RUNNING,
    FUNCTION_RX_MAXCOUNT,
    FUNCTION_TX_FINISH,
    FUNCTION_RX_FINISH,
    FUNCTION_INTERFACE_ERROR,
    FUNCTION_PARSE_ERROR_ADB,

    NumOf_FUNCTION_RETURN_STATUS
} FUNCTION_RETURN_STATUS;

typedef enum {
    BT_HCI_IF_NONE = 0,
    BT_HCI_IF_UART,
    BT_HCI_IF_USB
} bt_hci_if_t;

/** Bluetooth Address */
typedef struct {
    uint8_t address[6];
} __attribute__((packed))bt_bdaddr_t;

/** Bluetooth Device Name */
typedef struct {
    uint8_t name[248];
} __attribute__((packed))bt_bdname_t;

/** Bluetooth Adapter Visibility Modes*/
typedef enum {
    BT_SCAN_MODE_NONE,
    BT_SCAN_MODE_CONNECTABLE,
    BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE
} bt_scan_mode_t;

/** Bluetooth Adapter State */
typedef enum {
    BT_STATE_OFF,
    BT_STATE_ON
} bt_state_t;

/** Bluetooth Error Status */
/** We need to build on this */

typedef enum {
    BT_STATUS_SUCCESS = 0,
    BT_STATUS_FAIL,
    BT_STATUS_NOT_READY,
    BT_STATUS_NOMEM,
    BT_STATUS_BUSY,
    BT_STATUS_DONE,        /* request already completed */
    BT_STATUS_UNSUPPORTED,
    BT_STATUS_PARM_INVALID,
    BT_STATUS_UNHANDLED,
    BT_STATUS_AUTH_FAILURE,
    BT_STATUS_RMT_DEV_DOWN

} bt_status_t;

/** Bluetooth PinKey Code */
typedef struct {
    uint8_t pin[16];
} __attribute__((packed))bt_pin_code_t;

/** Bluetooth Adapter Discovery state */
typedef enum {
    BT_DISCOVERY_STOPPED,
    BT_DISCOVERY_STARTED
} bt_discovery_state_t;

/** Bluetooth ACL connection state */
typedef enum {
    BT_ACL_STATE_CONNECTED,
    BT_ACL_STATE_DISCONNECTED
} bt_acl_state_t;

/** Bluetooth 128-bit UUID */
typedef struct {
   uint8_t uu[16];
} bt_uuid_t;

/** Bluetooth Interface callbacks */

/** Bluetooth Enable/Disable Callback. */
typedef void (*adapter_state_changed_callback)(bt_state_t state);

typedef enum {
    ASSOCIATE_JVM,
    DISASSOCIATE_JVM
} bt_cb_thread_evt;

/** Thread Associate/Disassociate JVM Callback */
/* Callback that is invoked by the callback thread to allow upper layer to attach/detach to/from
 * the JVM */
typedef void (*callback_thread_event)(bt_cb_thread_evt evt);

/** Bluetooth Test Mode Callback */
/* Receive any HCI event from controller. Must be in DUT Mode for this callback to be received */
typedef void (*dut_mode_recv_callback)(uint8_t opcode, char *buf);

/** TODO: Add callbacks for Link Up/Down and other generic
  *  notifications/callbacks */

/** Bluetooth DM callback structure. */
typedef struct {
    /** set to sizeof(bt_callbacks_t) */
    size_t size;
    adapter_state_changed_callback adapter_state_changed_cb;
    callback_thread_event thread_evt_cb;
    dut_mode_recv_callback dut_mode_recv_cb;
} bt_callbacks_t;


/** Represents the standard Bluetooth DM interface. */
typedef struct {
    /** set to sizeof(bt_interface_t) */
    size_t size;
    /**
     * Opens the interface and provides the callback routines
     * to the implemenation of this interface.
     */
    int (*init)(bt_callbacks_t* callbacks, bt_hci_if_t hci_if, const char *dev_node);

    /** Enable Bluetooth. */
    int (*enable)(void);

    /** Disable Bluetooth. */
    int (*disable)(void);

    /** Closes the interface. */
    void (*cleanup)(void);

    /** Bluetooth Test Mode APIs - Bluetooth must be enabled for these APIs */
    /* Configure DUT Mode - Use this mode to enter/exit DUT mode */
    int (*dut_mode_configure)(uint8_t enable);

    /** Send any test HCI (vendor-specific) command to the controller. Must */
    /* be in DUT Mode */
    int (*hal_mp_op_send)(uint16_t opcode, char *buf);
} bt_interface_t;


typedef struct {
    struct hw_device_t common;
    const bt_interface_t* (*get_bluetooth_interface)();
} bluetooth_device_t;

typedef bluetooth_device_t bluetooth_module_t;
__END_DECLS

#endif /* BLUETOOTH_MP_H */
