/******************************************************************************
 *
 *  Copyright (C) 2014 Realtek Corporation
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

/******************************************************************************
 *
 *  Filename:      bt_hwcfg_usb.c
 *
 *  Description:   Contains controller-specific functions, like firmware patch
 *                 download, usb interface configure.
 *
 ******************************************************************************/

#define LOG_TAG "bt_hwcfg_usb"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "bt_syslog.h"
#include "bt_hci_bdroid.h"
#include "bt_vendor_usb.h"
#include "bt_vendor_lib.h"
#include "bt_hwcfg_if.h"

extern uint8_t USB_vnd_local_bd_addr[BD_ADDR_LEN];

void USB_hw_config_cback(void *p_evt_buf);

static bt_hw_cfg_cb_t USB_hw_cfg_cb;

static void fw_reset_timeout(union sigval arg)
{
    SYSLOGI("fw_reset_timeout, timer id %d", *(int *)arg.sival_ptr);

    USB_hw_config_cback(NULL);
}

/** Callback function for controller configurationn */
void USB_hw_config_cback(void *p_mem)
{
    HC_BT_HDR *p_evt_buf = NULL;
    HC_BT_HDR *p_buf = NULL;
    uint8_t   *p = NULL;
    uint8_t   *p_frag = NULL;
    uint8_t   status = 0;
    uint16_t  opcode = 0;
    uint8_t   is_proceeding = FALSE;
    patch_item *entry = NULL;
    uint8_t index = 0;

    if (p_mem != NULL) {
        p_evt_buf = (HC_BT_HDR *)p_mem;
        status = *((uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_STATUS_RET_BYTE);
        p = (uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_OPCODE;
        STREAM_TO_UINT16(opcode, p);
    }

    /* Ask a new buffer big enough to hold any HCI cmd sent here */
    /* Vendor cmd 0xFC6D may have a status 1. */
    if (((status == 0) || (opcode == HCI_VSC_READ_ROM_VERSION)) &&
        USB_bt_vendor_cbacks) {
        p_buf = (HC_BT_HDR *)USB_bt_vendor_cbacks->alloc(BT_HC_HDR_SIZE + HCI_CMD_MAX_LEN);
    }

    if (p_buf != NULL) {
        p_buf->event = MSG_STACK_TO_HC_HCI_CMD;
        p_buf->offset = 0;
        p_buf->len = 0;
        p_buf->layer_specific = 0;

        SYSLOGI("USB_hw_config_cback state: %d", USB_hw_cfg_cb.state);

        switch (USB_hw_cfg_cb.state) {
        case HW_CFG_FW_RESET:
            /* clear the fw reset completion timer */
            hw_cfg_clear_timer(USB_hw_cfg_cb.timer_id);

            p = (uint8_t *)(p_buf + 1);
            /* read local version information, here we care LMP sub version. */
            UINT16_TO_STREAM(p, HCI_READ_LOCAL_VERSION_INFO);
            *p = 0; /* parameter length */
            p_buf->len = HCI_CMD_PREAMBLE_SIZE;

            USB_hw_cfg_cb.state = HW_CFG_READ_LMP_SUB_VER;
            is_proceeding = USB_bt_vendor_cbacks->xmit_cb(HCI_READ_LOCAL_VERSION_INFO,
                                                    p_buf, USB_hw_config_cback);
            break;

        case HW_CFG_READ_LMP_SUB_VER:
            if (opcode != HCI_READ_LOCAL_VERSION_INFO) {
                SYSLOGE("ERROR! USB_hw_config_cback state: %d, received opcode 0x%04x",
                        USB_hw_cfg_cb.state, opcode);
                break;
            }

            p = (uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_LMP_SUB_VERSION;
            STREAM_TO_UINT16(USB_hw_cfg_cb.lmp_subver, p);

            SYSLOGI("LMP sub version 0x%04x", USB_hw_cfg_cb.lmp_subver);
            if (USB_hw_cfg_cb.lmp_subver == ROM_LMP_8723a) {
                USB_hw_cfg_cb.state = HW_CFG_START;
                goto CFG_START;
            } else {
                p = (uint8_t *)(p_buf + 1);
                UINT16_TO_STREAM(p, HCI_VSC_READ_ROM_VERSION);
                *p = 0; /* parameter length */
                p_buf->len = HCI_CMD_PREAMBLE_SIZE;

                USB_hw_cfg_cb.state = HW_CFG_READ_ROM_VER;
                is_proceeding = USB_bt_vendor_cbacks->xmit_cb(HCI_VSC_READ_ROM_VERSION,
                                                        p_buf, USB_hw_config_cback);
            }
            break;

        case HW_CFG_READ_ROM_VER:
            if (opcode != HCI_VSC_READ_ROM_VERSION) {
                SYSLOGE("ERROR! USB_hw_config_cback state: %d, received opcode 0x%04x",
                        USB_hw_cfg_cb.state, opcode);
                break;
            }

            p = (uint8_t *)(p_evt_buf + 1);

            if (status == 0) {
                USB_hw_cfg_cb.rom_ver = *(p + HCI_EVT_CMD_CMPL_ROM_VERSION);
            } else if (status == 1) {
                USB_hw_cfg_cb.rom_ver = 0; /* default a-cut */
            } else {
                is_proceeding = FALSE;
                SYSLOGE("read ROM version status error!");
                break;
            }

            USB_hw_cfg_cb.state = HW_CFG_START;

            SYSLOGI("read ROM version status: %d, echo version: %d",
                    status, USB_hw_cfg_cb.rom_ver);
            /* fall through intentionally */

CFG_START:
        case HW_CFG_START:
            SYSLOGI("USB_hw_config_cback state: %d", USB_hw_cfg_cb.state);

            /* load fw & config files according to patch item */
            entry = bt_hw_get_patch_item(USB_hw_cfg_cb.lmp_subver);
            if (entry) {
                USB_hw_cfg_cb.config_len = bt_hw_load_file(&USB_hw_cfg_cb.config_buf,
                                                entry->config_name);
                if (USB_hw_cfg_cb.config_len < 0) {
                    USB_hw_cfg_cb.config_len = 0;
                } else {
                    bt_hw_parse_config(&USB_hw_cfg_cb, USB_vnd_local_bd_addr);
                }

                USB_hw_cfg_cb.fw_len = bt_hw_load_file(&USB_hw_cfg_cb.fw_buf,
                                                entry->fw_name);
                if (USB_hw_cfg_cb.fw_len < 0) {
                    USB_hw_cfg_cb.fw_len = 0;
                    /* release config file buffer */
                    if (USB_hw_cfg_cb.config_len > 0) {
                        USB_hw_cfg_cb.config_len = 0;
                        free(USB_hw_cfg_cb.config_buf);
                    }
                } else {
                    bt_hw_extract_firmware(&USB_hw_cfg_cb);
                }
            }

            if (USB_hw_cfg_cb.total_len > 0 && USB_hw_cfg_cb.dl_fw_flag) {
                USB_hw_cfg_cb.patch_frag_cnt = USB_hw_cfg_cb.total_len / PATCH_FRAGMENT_MAX_SIZE;
                USB_hw_cfg_cb.patch_frag_tail = USB_hw_cfg_cb.total_len % PATCH_FRAGMENT_MAX_SIZE;
                if (USB_hw_cfg_cb.patch_frag_tail) {
                    USB_hw_cfg_cb.patch_frag_cnt += 1;
                } else {
                    USB_hw_cfg_cb.patch_frag_tail = PATCH_FRAGMENT_MAX_SIZE;
                }

                SYSLOGI("patch fragment count %d, tail len %d",
                        USB_hw_cfg_cb.patch_frag_cnt, USB_hw_cfg_cb.patch_frag_tail);
            } else {
                is_proceeding = FALSE;
                break;
            }

            USB_hw_cfg_cb.state = HW_CFG_DL_FW_PATCH;
            /* fall through intentionally */

        case HW_CFG_DL_FW_PATCH:
            SYSLOGI("HW_CFG_DL_FW_PATCH status: %d, opcode 0x%04x", status, opcode);

            p = (uint8_t *)(p_evt_buf + 1);

            if (opcode == HCI_VSC_DOWNLOAD_FW_PATCH) {
                index = *(p + HCI_EVT_CMD_CMPL_DL_FW_PATCH_INDEX);
                SYSLOGI("HW_CFG_DL_FW_PATCH: index %d", index);
                if (index & 0x80) {
                    SYSLOGI("bt hw config completed");

                    free(USB_hw_cfg_cb.total_buf);
                    USB_bt_vendor_cbacks->dealloc(p_buf);
                    USB_bt_vendor_cbacks->fwcfg_cb(BT_VND_OP_RESULT_SUCCESS);

                    USB_hw_cfg_cb.state = HW_CFG_UNINIT;
                    is_proceeding = TRUE;
                    break;
                }

                USB_hw_cfg_cb.patch_frag_idx++;
            }

            if (USB_hw_cfg_cb.patch_frag_idx < USB_hw_cfg_cb.patch_frag_cnt) {
                if (USB_hw_cfg_cb.patch_frag_idx == USB_hw_cfg_cb.patch_frag_cnt - 1) {
                    SYSLOGI("HW_CFG_DL_FW_PATCH: send last fw fragment");
                    USB_hw_cfg_cb.patch_frag_idx |= 0x80;
                    USB_hw_cfg_cb.patch_frag_len = USB_hw_cfg_cb.patch_frag_tail;
                } else {
                    USB_hw_cfg_cb.patch_frag_len = PATCH_FRAGMENT_MAX_SIZE;
                }
            }

            SYSLOGI("patch fragement index %d, len %d",
                    USB_hw_cfg_cb.patch_frag_idx, USB_hw_cfg_cb.patch_frag_len);

            /* download firmware patch in fragment unit */
            p = (uint8_t *)(p_buf + 1);
            UINT16_TO_STREAM(p, HCI_VSC_DOWNLOAD_FW_PATCH);
            *p++ = 1 +  USB_hw_cfg_cb.patch_frag_len; /* parameter length */
            *p++ = USB_hw_cfg_cb.patch_frag_idx;

            p_frag = USB_hw_cfg_cb.total_buf +
                (USB_hw_cfg_cb.patch_frag_idx & 0x7F) * PATCH_FRAGMENT_MAX_SIZE;
            memcpy(p, p_frag, USB_hw_cfg_cb.patch_frag_len);

            p_buf->len = HCI_CMD_PREAMBLE_SIZE + 1 + USB_hw_cfg_cb.patch_frag_len;

            is_proceeding = USB_bt_vendor_cbacks->xmit_cb(HCI_VSC_DOWNLOAD_FW_PATCH,
                                                    p_buf, USB_hw_config_cback);
            break;

        default:
            break;
        } /* switch(USB_hw_cfg_cb.state) */
    } /* if (p_buf != NULL) */

    /* Free the RX event buffer */
    if (USB_bt_vendor_cbacks && p_evt_buf)
        USB_bt_vendor_cbacks->dealloc(p_evt_buf);

    if (is_proceeding == FALSE) {
        SYSLOGE("bt hw config aborted!!!");
        if (USB_bt_vendor_cbacks) {
            if (p_buf != NULL)
                USB_bt_vendor_cbacks->dealloc(p_buf);

            USB_bt_vendor_cbacks->fwcfg_cb(BT_VND_OP_RESULT_FAIL);
        }

        USB_hw_cfg_cb.state = HW_CFG_UNINIT;
    }
}

/** Kick off controller initialization process */
void USB_hw_config_start(void)
{
    HC_BT_HDR *p_buf = NULL;
    uint8_t *p;
    int res;

    memset(&USB_hw_cfg_cb, 0, sizeof(bt_hw_cfg_cb_t));
    USB_hw_cfg_cb.state = HW_CFG_UNINIT;
    USB_hw_cfg_cb.dl_fw_flag = 1;

    SYSLOGI("USB_hw_config_start: version %s", BT_CFG_VERSION);

    /* Start from clearing controller firmware */
    if (USB_bt_vendor_cbacks) {
        p_buf = (HC_BT_HDR *)USB_bt_vendor_cbacks->alloc(BT_HC_HDR_SIZE + HCI_CMD_PREAMBLE_SIZE);
    }

    if (p_buf) {
        p_buf->event = MSG_STACK_TO_HC_HCI_CMD;
        p_buf->offset = 0;
        p_buf->layer_specific = 0;
        p_buf->len = HCI_CMD_PREAMBLE_SIZE;

        p = (uint8_t *)(p_buf + 1);
        UINT16_TO_STREAM(p, HCI_VSC_FIRMWARE_RESET);
        *p = 0; /* parameter length */

        USB_hw_cfg_cb.state = HW_CFG_FW_RESET;

        USB_bt_vendor_cbacks->xmit_cb(HCI_VSC_FIRMWARE_RESET, p_buf, NULL);
        /* As firmware reset vendor cmd has not evt returned, we set a timer
         * waiting for 1 second to ensure that the cmd has been completed.
         * It is sick, you know.
         */
        res = hw_cfg_set_timer(&USB_hw_cfg_cb, fw_reset_timeout, 1000);
        if (res < 0) {
            if (USB_bt_vendor_cbacks) {
                SYSLOGE("bt hw config aborted[no timer]");
                USB_bt_vendor_cbacks->fwcfg_cb(BT_VND_OP_RESULT_FAIL);
            }
        }
    } else {
        if (USB_bt_vendor_cbacks) {
            SYSLOGE("bt hw config aborted[no buffer]");
            USB_bt_vendor_cbacks->fwcfg_cb(BT_VND_OP_RESULT_FAIL);
        }
    }
}
