/******************************************************************************
 *
 *  Copyright (C) 2009-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/************************************************************************************
 *
 *  Filename:      bluetooth.c
 *
 *  Description:   Bluetooth HAL implementation
 *
 ***********************************************************************************/

#define LOG_TAG "bluedroid"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "bluetoothmp.h"
#include "bt_syslog.h"

#include "btif_api.h"
#include "foundation.h"
#include "bt_mp_base.h"
#include "bt_mp_api.h"
#include "gki.h"
#include "user_config.h"


#define DEV_NODE_NAME_MAXLEN 256

bt_callbacks_t *bt_hal_cbacks = NULL;
bt_hci_if_t bt_hci_if = BT_HCI_IF_NONE;
char bt_dev_node[DEV_NODE_NAME_MAXLEN] = {0};


/************************************************************************************
**  Static functions
************************************************************************************/

/************************************************************************************
**  Externs
************************************************************************************/
BT_MODULE               BtModuleMemory;
BASE_INTERFACE_MODULE   BaseInterfaceModuleMemory;

/************************************************************************************
**  Functions
************************************************************************************/

uint8_t hal_interface_ready(void)
{
    /* add checks here that would prevent API calls other than init to be executed */
    if (bt_hal_cbacks == NULL)
        return FALSE;

    return TRUE;
}


/*****************************************************************************
**
**   BLUETOOTH HAL INTERFACE FUNCTIONS
**
*****************************************************************************/

int hal_init(bt_callbacks_t *callbacks, bt_hci_if_t hci_if, const char *dev_node)
{
    SYSLOGI("init");

    /* sanity check */
    if (hal_interface_ready() == TRUE)
        return BT_STATUS_DONE;

    /* store reference to user callbacks */
    bt_hal_cbacks = callbacks;

    /* store bt hci if type and device driver node */
    bt_hci_if = hci_if;
    snprintf(bt_dev_node, DEV_NODE_NAME_MAXLEN, "%s", dev_node);

    /* init mp module */
    bt_mp_module_init(&BaseInterfaceModuleMemory, &BtModuleMemory);

    /* init btif */
    btif_init_bluetooth();

    return BT_STATUS_SUCCESS;
}

int hal_enable(void)
{
    SYSLOGI("enable");

    /* sanity check */
    if (hal_interface_ready() == FALSE)
        return BT_STATUS_NOT_READY;

    return btif_enable_bluetooth(bt_hci_if, bt_dev_node);
}

int hal_disable(void)
{
    /* sanity check */
    if (hal_interface_ready() == FALSE)
        return BT_STATUS_NOT_READY;

    return btif_disable_bluetooth();
}

void hal_cleanup(void)
{
    /* sanity check */
    if (hal_interface_ready() == FALSE)
        return;

    btif_shutdown_bluetooth();

    bt_hal_cbacks = NULL;
    /* hal callbacks reset upon shutdown complete callback */

    return;
}

extern void btu_hcif_mp_notify_event(BT_HDR *p_msg);

int hal_op_send(uint16_t opcode, char *buf)
{
    BT_HDR *p_buf = NULL;
    char *p = NULL;
    uint16_t buf_len = 0;
    char buf_cb[1024] = {0};
    int ret = 0;

    p_buf = (BT_HDR *)GKI_getbuf(sizeof(BT_HDR) + 1024);
    p_buf->offset = 0;
    p = (char *)(p_buf + 1);
    memset(p, 0, 1024);

    SYSLOGI("hal_op_send: opcode[0x%02x], buf[%s]", opcode, buf);

    /* sanity check */
    if (hal_interface_ready() == FALSE) {
        GKI_freebuf(p_buf);
        return BT_STATUS_NOT_READY;
    }

    switch (opcode) {
    case BT_MP_OP_HCI_SEND_CMD:
        ret = BT_SendHciCmd(&BtModuleMemory, buf, buf_cb);
        break;

    case BT_MP_OP_USER_DEF_GetParam:
        ret = BT_GetParam(&BtModuleMemory, buf, buf_cb);
        break;

    case BT_MP_OP_USER_DEF_SetParam:
        ret = BT_SetParam(&BtModuleMemory, buf, buf_cb);
        break;

    case BT_MP_OP_USER_DEF_SetConfig:
        ret = BT_SetConfig(&BtModuleMemory, buf, buf_cb);
        break;

    case BT_MP_OP_USER_DEF_Exec:
        ret = BT_Exec(&BtModuleMemory, buf, buf_cb);
        break;

    case BT_MP_OP_USER_DEF_Report:
        ret = BT_Report(&BtModuleMemory, buf, buf_cb);
        break;

    case BT_MP_OP_USER_DEF_RegRW:
        ret = BT_RegRW(&BtModuleMemory, buf, buf_cb);
        break;
        
#if (MP_TOOL_COMMAND_SEARCH_EXIST_PERMISSION == 1)
    case BT_MP_OP_USER_DEF_search:
        ret = BT_search(&BtModuleMemory, buf, buf_cb);
        break;
#endif

#if (MP_TOOL_COMMAND_READ_PERMISSION == 1)
    case BT_MP_OP_USER_DEF_read:
        ret = BT_read(&BtModuleMemory, buf, buf_cb);
        break;
#endif

    default:
        SYSLOGW("hal_op_send: undefined opcode[0x%02x]", opcode);
        break;
    }

    buf_len = strlen(buf_cb);

    UINT8_TO_STREAM(p, opcode);
    UINT16_TO_STREAM(p, buf_len);

    SYSLOGI("buf_cb: %s, buf_len %d", buf_cb, buf_len);

    memcpy(p, buf_cb, buf_len);

    btu_hcif_mp_notify_event(p_buf);

    GKI_freebuf(p_buf);

    return ret;
}

static const bt_interface_t bluetoothInterface = {
    sizeof(bt_interface_t),
    hal_init,
    hal_enable,
    hal_disable,
    hal_cleanup,
    hal_op_send
};


const bt_interface_t *bluetooth_get_bluetooth_interface(void)
{
    return &bluetoothInterface;
}

static int close_bluetooth_stack()
{
    hal_cleanup();
    return 0;
}

int open_bluetooth_stack(char const *name, bluetooth_device_t **bt_device)
{
    bluetooth_device_t *stack = malloc(sizeof(bluetooth_device_t));
    memset(stack, 0, sizeof(bluetooth_device_t));

    strncpy(stack->name, name, 35); // one byte reserved for null
    stack->close = close_bluetooth_stack;
    stack->get_bluetooth_interface = bluetooth_get_bluetooth_interface;
    *bt_device = stack;

    return 0;
}
