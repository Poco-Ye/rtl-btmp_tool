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

#define LOG_TAG "bt_vendor_usb"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#include "bt_vendor_usb.h"

#ifndef BTVND_DBG
#define BTVND_DBG FALSE
#endif

#if (BTVND_DBG == TRUE)
#define BTVNDDBG(param, ...) {/*ALOGD(param, ## __VA_ARGS__);*/}
#else
#define BTVNDDBG(param, ...) {}
#endif

/******************************************************************************
**  Local type definitions
******************************************************************************/
#define VND_PORT_NAME_MAXLEN    256
/* vendor serial control block */
typedef struct
{
    int fd;                     /* fd to Bluetooth device */
    char port_name[VND_PORT_NAME_MAXLEN];
} vnd_usb_cb_t;

/******************************************************************************
**  Variables
******************************************************************************/

bt_vendor_callbacks_t *USB_bt_vendor_cbacks = NULL;
uint8_t USB_vnd_local_bd_addr[6]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static vnd_usb_cb_t vnd_usb;

static void usb_vendor_init(const char *dev_node)
{
    const char *port;

    vnd_usb.fd = -1;

    port = dev_node ?: BLUETOOTH_USB_DEVICE_PORT;
    snprintf(vnd_usb.port_name, VND_PORT_NAME_MAXLEN, "%s", port);
}

static int usb_vendor_open(void)
{
    //ALOGI("usb vendor open: opening %s", vnd_usb.port_name);

    if ((vnd_usb.fd = open(vnd_usb.port_name, O_RDWR)) == -1)
    {
        //ALOGE("usb vendor open: unable to open %s(uid %d, gid %d): %s",
        //        vnd_usb.port_name, getuid(), getgid(), strerror(errno));
        return -1;
    }

    //ALOGI("device fd = %d open", vnd_usb.fd);

    return vnd_usb.fd;
}

static void usb_vendor_close(void)
{
    int res;

    if (vnd_usb.fd == -1)
        return;

    //ALOGI("device fd = %d close", vnd_usb.fd);

    res = close(vnd_usb.fd);
    if (res  < 0)
        ;//ALOGE("Failed to close(fd %d): %s", vnd_usb.fd, strerror(res));

    vnd_usb.fd = -1;
}

/*****************************************************************************
**
**   BLUETOOTH VENDOR INTERFACE LIBRARY FUNCTIONS
**
*****************************************************************************/

static int USB_bt_vnd_init(const bt_vendor_callbacks_t* p_cb, unsigned char *local_bdaddr, const char *dev_node)
{
    //ALOGI("%s: dev_node %s", __FUNCTION__, dev_node);

    if (p_cb == NULL) {
        //ALOGE("init failed with no user callbacks!");
        return -1;
    }

    usb_vendor_init(dev_node);

    /* store reference to user callbacks */
    USB_bt_vendor_cbacks = (bt_vendor_callbacks_t *)p_cb;

    /* This is handed over from the stack */
    memcpy(USB_vnd_local_bd_addr, local_bdaddr, 6);

    return 0;
}


/** Requested operations */
static int USB_bt_vnd_op(bt_vendor_opcode_t opcode, void *param)
{
    int retval = 0;

    BTVNDDBG("op for %d", opcode);

    switch(opcode)
    {
        case BT_VND_OP_POWER_CTRL:
            {
                return 0;
            }
            break;

        case BT_VND_OP_FW_CFG:
            {
                USB_bt_vendor_cbacks->fwcfg_cb(BT_VND_OP_RESULT_SUCCESS);
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
                fd = usb_vendor_open();
                if (fd != -1)
                {
                    for (idx=0; idx < CH_MAX; idx++)
                        (*fd_array)[idx] = fd;

                    retval = 1;
                }
                /* retval contains numbers of open fd of HCI channels */
            }
            break;

        case BT_VND_OP_USERIAL_CLOSE:
            {
                usb_vendor_close();
            }
            break;

        case BT_VND_OP_GET_LPM_IDLE_TIMEOUT:
            {
                uint32_t *timeout_ms = (uint32_t *) param;
                *timeout_ms = 250;
            }
            break;

        case BT_VND_OP_LPM_SET_MODE:
            {
                if (USB_bt_vendor_cbacks)
                    USB_bt_vendor_cbacks->lpm_cb(BT_VND_OP_RESULT_SUCCESS);
            }
            break;

        case BT_VND_OP_LPM_WAKE_SET_STATE:
            break;
            default:
                break;
    }

    return retval;
}

/** Closes the interface */
static void USB_bt_vnd_cleanup( void )
{
    BTVNDDBG("cleanup");
    USB_bt_vendor_cbacks = NULL;
}

// Entry point of DLib
const bt_vendor_interface_t USB_BLUETOOTH_VENDOR_LIB_INTERFACE = {
    sizeof(bt_vendor_interface_t),
    USB_bt_vnd_init,
    USB_bt_vnd_op,
    USB_bt_vnd_cleanup
};

const bt_vendor_interface_t *USB_bt_vnd_if = &USB_BLUETOOTH_VENDOR_LIB_INTERFACE;
