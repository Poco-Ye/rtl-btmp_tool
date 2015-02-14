/******************************************************************************
 *
 *  Copyright (C) 2012-2014 Realtek Corporation
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

#define LOG_TAG "bt_vendor_uart"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "bt_syslog.h"
#include "bt_vendor_if.h"

void UART_hw_config_start(bt_hci_if_t proto);

/*****************************************************************************
**
**   BLUETOOTH VENDOR INTERFACE LIBRARY FUNCTIONS
**
*****************************************************************************/

static int UART_bt_vnd_init(const bt_vendor_callbacks_t* p_cb, unsigned char *local_bdaddr, const char *dev_node)
{
    SYSLOGI("%s: dev_node %s", __FUNCTION__, dev_node);

    if (p_cb == NULL) {
        SYSLOGE("init failed with no user callbacks!");
        return -1;
    }

    userial_vendor_init(dev_node);

    /* store reference to user callbacks */
    bt_vendor_cbacks = (bt_vendor_callbacks_t *)p_cb;

    /* This is handed over from the stack */
    memcpy(vnd_local_bd_addr, local_bdaddr, 6);

    return 0;
}

static int UART4_bt_vnd_op(bt_vendor_opcode_t opcode, void *param)
{
    int retval = 0;

    SYSLOGI("op for %d", opcode);

    switch (opcode) {
    case BT_VND_OP_POWER_CTRL:
        {
            int *state = (int *) param;
            if (*state == BT_VND_PWR_OFF) {
                bt_vendor_set_power(BT_POWER_OFF);
                usleep(200000);
                SYSLOGI("set power off and delay 200ms");
            } else if (*state == BT_VND_PWR_ON) {
                bt_vendor_set_power(BT_POWER_ON);
                usleep(500000);
                SYSLOGI("set power on and delay 500ms");
            }
        }
        break;

    case BT_VND_OP_FW_CFG:
        {
            UART_hw_config_start(BT_HCI_IF_UART4);
        }
        break;

    case BT_VND_OP_SCO_CFG:
        {
            retval = -1;
        }
        break;

    case BT_VND_OP_USERIAL_OPEN:
        {
            int (*fd_array)[] = (int (*)[]) param;
            int fd, idx;
            tUSERIAL_CFG uart4_cfg = {
                (USERIAL_DATABITS_8 | USERIAL_PARITY_NONE | USERIAL_STOPBITS_1),
                USERIAL_BAUD_115200,
                USERIAL_HW_FLOW_CTRL_OFF
            };
            fd = userial_vendor_open(&uart4_cfg);
            if (fd != -1) {
                for (idx=0; idx < CH_MAX; idx++)
                    (*fd_array)[idx] = fd;

                retval = 1;
            }
        }
        break;

    case BT_VND_OP_USERIAL_CLOSE:
        {
            userial_vendor_close();
        }
        break;

    default:
        break;
    }

    return retval;
}

static int UART5_bt_vnd_op(bt_vendor_opcode_t opcode, void *param)
{
    int retval = 0;

    SYSLOGI("op for %d", opcode);

    switch (opcode) {
    case BT_VND_OP_POWER_CTRL:
        {
            int *state = (int *) param;
            if (*state == BT_VND_PWR_OFF) {
                bt_vendor_set_power(BT_POWER_OFF);
                usleep(200000);
                SYSLOGI("set power off and delay 200ms");
            } else if (*state == BT_VND_PWR_ON) {
                bt_vendor_set_power(BT_POWER_ON);
                usleep(500000);
                SYSLOGI("set power on and delay 500ms");
            }
        }
        break;

    case BT_VND_OP_FW_CFG:
        {
            UART_hw_config_start(BT_HCI_IF_UART5);
        }
        break;

    case BT_VND_OP_SCO_CFG:
        {
            retval = -1;
        }
        break;

    case BT_VND_OP_USERIAL_OPEN:
        {
            int (*fd_array)[] = (int (*)[]) param;
            int fd, idx;
            tUSERIAL_CFG uart5_cfg = {
                (USERIAL_DATABITS_8 | USERIAL_PARITY_EVEN | USERIAL_STOPBITS_1),
                USERIAL_BAUD_115200,
                USERIAL_HW_FLOW_CTRL_OFF
            };
            fd = userial_vendor_open(&uart5_cfg);
            if (fd != -1) {
                for (idx=0; idx < CH_MAX; idx++)
                    (*fd_array)[idx] = fd;

                retval = 1;
            }
        }
        break;

    case BT_VND_OP_USERIAL_CLOSE:
        {
            userial_vendor_close();
        }
        break;

    default:
        break;
    }

    return retval;
}

/** Closes the interface */
static void UART_bt_vnd_cleanup( void )
{
    SYSLOGI("cleanup");
    bt_vendor_cbacks = NULL;
}

// Entry point of uart4 DLib
const bt_vendor_interface_t UART4_BLUETOOTH_VENDOR_LIB_INTERFACE = {
    sizeof(bt_vendor_interface_t),
    UART_bt_vnd_init,
    UART4_bt_vnd_op,
    UART_bt_vnd_cleanup
};

const bt_vendor_interface_t *UART4_bt_vnd_if = &UART4_BLUETOOTH_VENDOR_LIB_INTERFACE;

// Entry point of uart5 DLib
const bt_vendor_interface_t UART5_BLUETOOTH_VENDOR_LIB_INTERFACE = {
    sizeof(bt_vendor_interface_t),
    UART_bt_vnd_init,
    UART5_bt_vnd_op,
    UART_bt_vnd_cleanup
};

const bt_vendor_interface_t *UART5_bt_vnd_if = &UART5_BLUETOOTH_VENDOR_LIB_INTERFACE;
