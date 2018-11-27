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
 *  Filename:      bt_hwcfg_uart.c
 *
 *  Description:   Contains controller-specific functions, like firmware patch
 *                 download, uart interface configure.
 *
 ******************************************************************************/

#define LOG_TAG "bt_hwcfg_uart"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#include "bt_syslog.h"
#include "bt_hwcfg_if.h"
#include "bt_vendor_lib.h"
#include "bt_hci_bdroid.h"
#include "bt_vendor_if.h"
#include "userial.h"

void UART_hw_config_cback(void *p_evt_buf);

static bt_hw_cfg_cb_t UART_hw_cfg_cb;

/** helper function converts line speed number into USERIAL baud symbol */
uint8_t line_speed_to_userial_baud(uint32_t line_speed)
{
    uint8_t baud;

    if (line_speed == 4000000)
        baud = USERIAL_BAUD_4M;
    else if (line_speed == 3000000)
        baud = USERIAL_BAUD_3M;
    else if (line_speed == 2000000)
        baud = USERIAL_BAUD_2M;
    else if (line_speed == 1500000)
        baud = USERIAL_BAUD_1_5M;
    else if (line_speed == 1000000)
        baud = USERIAL_BAUD_1M;
    else if (line_speed == 921600)
        baud = USERIAL_BAUD_921600;
    else if (line_speed == 460800)
        baud = USERIAL_BAUD_460800;
    else if (line_speed == 230400)
        baud = USERIAL_BAUD_230400;
    else if (line_speed == 115200)
        baud = USERIAL_BAUD_115200;
    else if (line_speed == 57600)
        baud = USERIAL_BAUD_57600;
    else if (line_speed == 19200)
        baud = USERIAL_BAUD_19200;
    else if (line_speed == 9600)
        baud = USERIAL_BAUD_9600;
    else if (line_speed == 1200)
        baud = USERIAL_BAUD_1200;
    else if (line_speed == 600)
        baud = USERIAL_BAUD_600;
    else {
        SYSLOGE("userial vendor: unsupported baud speed %d", line_speed);
        baud = USERIAL_BAUD_115200;
    }

    return baud;
}

typedef struct {
    uint32_t bt_speed;
    uint32_t uart_speed;
} baudrate_map;

baudrate_map baudrates[] = {
    {0x0252C014, 115200},
    {0x0252C00A, 230400},
    {0x05F75004, 921600},
    {0x00005004, 1000000},
    {0x04928002, 1500000},
    {0x00005002, 2000000},
    {0x0000B001, 2500000},
    {0x04928001, 3000000},
    {0x052A6001, 3500000},
    {0x00005001, 4000000},
};

/**
 * Change bluetooth uart speed to uart speed.
 * If there is no match in baudrates, uart speed will be set as #115200.
 */
static void bt_speed_to_uart_speed(uint32_t bt_speed, uint32_t *uart_speed)
{
    uint8_t i;

    *uart_speed = 115200;

    for (i = 0; i < sizeof(baudrates)/sizeof(baudrate_map); i++) {
        if (baudrates[i].bt_speed == bt_speed) {
            *uart_speed = baudrates[i].uart_speed;
            return;
        }
    }

    SYSLOGW("bt_speed_to_uart_speed: use default baudrate[115200]");

    return;
}

/** Set bluetooth controller's uart interface baudrate */
static uint8_t hw_config_set_controller_baudrate(HC_BT_HDR *p_buf, uint32_t baudrate)
{
    uint8_t ret = FALSE;
    uint8_t *p = (uint8_t *)(p_buf + 1);

    UINT16_TO_STREAM(p, HCI_VSC_UPDATE_BAUDRATE);
    *p++ = 4; /* parameter length */
    UINT32_TO_STREAM(p, baudrate);

    p_buf->len = HCI_CMD_PREAMBLE_SIZE + 4;

    ret = bt_vendor_cbacks->xmit_cb(HCI_VSC_UPDATE_BAUDRATE, p_buf,
                                 UART_hw_config_cback);
    return ret;
}

/** Callback function for controller configurationn */
void UART_hw_config_cback(void *p_mem)
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

#if (USE_CONTROLLER_BDADDR == TRUE)
    const uint8_t null_bdaddr[BD_ADDR_LEN] = {0,0,0,0,0,0};
#endif

    if (p_mem != NULL) {
        p_evt_buf = (HC_BT_HDR *)p_mem;
        status = *((uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_STATUS_RET_BYTE);
        p = (uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_OPCODE;
        STREAM_TO_UINT16(opcode, p);
    }

    /* Ask a new buffer big enough to hold any HCI cmd sent here */
    /* Vendor cmd 0xFC6D may have a status 1. */
    if (((status == 0) || (opcode == HCI_VSC_READ_ROM_VERSION)) &&
        bt_vendor_cbacks) {
        p_buf = (HC_BT_HDR *)bt_vendor_cbacks->alloc(BT_HC_HDR_SIZE + HCI_CMD_MAX_LEN);
    }

    if (p_buf != NULL) {
        p_buf->event = MSG_STACK_TO_HC_HCI_CMD;
        p_buf->offset = 0;
        p_buf->len = 0;
        p_buf->layer_specific = 0;

        p = (uint8_t *)(p_buf + 1);

        SYSLOGI("UART_hw_config_cback state: %d", UART_hw_cfg_cb.state);

        switch (UART_hw_cfg_cb.state) {
        case HW_CFG_H4_START:
        case HW_CFG_H5_SYNC:
            p = (uint8_t *)(p_buf + 1);
            /* read local version information, here we care LMP sub version. */
            UINT16_TO_STREAM(p, HCI_READ_LOCAL_VERSION_INFO);
            *p = 0; /* parameter length */
            p_buf->len = HCI_CMD_PREAMBLE_SIZE;

            UART_hw_cfg_cb.state = HW_CFG_READ_LMP_SUB_VER;

            is_proceeding = bt_vendor_cbacks->xmit_cb(HCI_READ_LOCAL_VERSION_INFO,
                                                    p_buf, UART_hw_config_cback);
            break;

        case HW_CFG_READ_LMP_SUB_VER:
            if (opcode != HCI_READ_LOCAL_VERSION_INFO) {
                SYSLOGE("ERROR! UART_hw_config_cback state: %d, received opcode 0x%04x",
                        UART_hw_cfg_cb.state, opcode);
                break;
            }

            p = (uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_LMP_SUB_VERSION;
            STREAM_TO_UINT16(UART_hw_cfg_cb.lmp_subver, p);

            p = (uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_HCI_SUB_VERSION;
            STREAM_TO_UINT16(UART_hw_cfg_cb.hci_subver, p);

            SYSLOGI("LMP sub version 0x%04x, HCI sub version 0x%04x",
                                   UART_hw_cfg_cb.lmp_subver, UART_hw_cfg_cb.hci_subver);
            if (UART_hw_cfg_cb.lmp_subver == ROM_LMP_8723a) {
                UART_hw_cfg_cb.state = HW_CFG_PARSE_FW_PATCH;
                goto CFG_PARSE_FW_PATCH;
            } else {
                p = (uint8_t *)(p_buf + 1);
                UINT16_TO_STREAM(p, HCI_VSC_READ_ROM_VERSION);
                *p = 0; /* parameter length */
                p_buf->len = HCI_CMD_PREAMBLE_SIZE;

                UART_hw_cfg_cb.state = HW_CFG_READ_ROM_VER;
                is_proceeding = bt_vendor_cbacks->xmit_cb(HCI_VSC_READ_ROM_VERSION,
                                                        p_buf, UART_hw_config_cback);
            }
            break;

        case HW_CFG_READ_ROM_VER:
            if (opcode != HCI_VSC_READ_ROM_VERSION) {
                SYSLOGE("ERROR! UART_hw_config_cback state: %d, received opcode 0x%04x",
                        UART_hw_cfg_cb.state, opcode);
                break;
            }

            p = (uint8_t *)(p_evt_buf + 1);

            if (status == 0) {
                UART_hw_cfg_cb.rom_ver = *(p + HCI_EVT_CMD_CMPL_ROM_VERSION);
            } else if (status == 1) {
                UART_hw_cfg_cb.rom_ver = 0; /* default a-cut */
            } else {
                is_proceeding = FALSE;
                SYSLOGE("read ROM version status error!");
                break;
            }

            UART_hw_cfg_cb.state = HW_CFG_PARSE_FW_PATCH;

            SYSLOGI("read ROM version status: %d, echo version: %d", status, UART_hw_cfg_cb.rom_ver);
            /* fall through intentionally */

CFG_PARSE_FW_PATCH:
        case HW_CFG_PARSE_FW_PATCH:
            SYSLOGI("UART_hw_config_cback state: %d", UART_hw_cfg_cb.state);

            /* load fw & config files according to patch item */
            entry = bt_hw_get_patch_item(UART_hw_cfg_cb.lmp_subver, UART_hw_cfg_cb.hci_subver);
            if (entry) {
                UART_hw_cfg_cb.config_len = bt_hw_load_file(&UART_hw_cfg_cb.config_buf,
                                                entry->config_name);
                if (UART_hw_cfg_cb.config_len < 0) {
                    UART_hw_cfg_cb.config_len = 0;
                } else {
                    bt_hw_parse_config(&UART_hw_cfg_cb, vnd_local_bd_addr);
                }

                UART_hw_cfg_cb.fw_len = bt_hw_load_file(&UART_hw_cfg_cb.fw_buf,
                                                entry->fw_name);
                if (UART_hw_cfg_cb.fw_len < 0) {
                    UART_hw_cfg_cb.fw_len = 0;
                    /* release config file buffer */
                    if (UART_hw_cfg_cb.config_len > 0) {
                        UART_hw_cfg_cb.config_len = 0;
                        free(UART_hw_cfg_cb.config_buf);
                    }
                } else {
                    bt_hw_extract_firmware(&UART_hw_cfg_cb);
                }
            }

            if (UART_hw_cfg_cb.total_len > 0 && UART_hw_cfg_cb.dl_fw_flag) {
                UART_hw_cfg_cb.patch_frag_cnt = UART_hw_cfg_cb.total_len / PATCH_FRAGMENT_MAX_SIZE;
                UART_hw_cfg_cb.patch_frag_tail = UART_hw_cfg_cb.total_len % PATCH_FRAGMENT_MAX_SIZE;
                if (UART_hw_cfg_cb.patch_frag_tail) {
                    UART_hw_cfg_cb.patch_frag_cnt += 1;
                } else {
                    UART_hw_cfg_cb.patch_frag_tail = PATCH_FRAGMENT_MAX_SIZE;
                }

                SYSLOGI("patch fragment count %d, tail len %d",
                        UART_hw_cfg_cb.patch_frag_cnt, UART_hw_cfg_cb.patch_frag_tail);
            } else {
                is_proceeding = FALSE;
                break;
            }

            UART_hw_cfg_cb.state = HW_CFG_SET_CNTRL_BAUDRATE;
            /* fall through intentionally */

        case HW_CFG_SET_CNTRL_BAUDRATE:
            SYSLOGI("UART_hw_config_cback state: %d", UART_hw_cfg_cb.state);

            if (UART_hw_cfg_cb.baudrate[1] == 0) {
                UART_hw_cfg_cb.baudrate[1] = 0x0252C014;
            }

            SYSLOGI("bt hw config: set controller uart baud 0x%08x", UART_hw_cfg_cb.baudrate[1]);
            is_proceeding = hw_config_set_controller_baudrate(p_buf, UART_hw_cfg_cb.baudrate[1]);
            UART_hw_cfg_cb.state = HW_CFG_SET_HOST_BAUDRATE;
            break;

        case HW_CFG_SET_HOST_BAUDRATE:
            SYSLOGI("UART_hw_config_cback state: %d", UART_hw_cfg_cb.state);

            /* update baudrate of host's UART port */
            bt_speed_to_uart_speed(UART_hw_cfg_cb.baudrate[1], &UART_hw_cfg_cb.baudrate[0]);
            SYSLOGI("bt hw config: set host uart baud %d", UART_hw_cfg_cb.baudrate[0]);
            userial_vendor_set_baud(line_speed_to_userial_baud(UART_hw_cfg_cb.baudrate[0]));

            if (UART_hw_cfg_cb.hw_flow_cntrl & 0x80) {
                SYSLOGI("bt hw config: change host hw flow control settings");
                if (UART_hw_cfg_cb.hw_flow_cntrl & 0x01) {
                    userial_vendor_set_hw_fctrl(1);
                } else {
                    userial_vendor_set_hw_fctrl(0);
                }
            }

            UART_hw_cfg_cb.state = HW_CFG_DL_FW_PATCH;
            ms_delay(100);
            /* fall through intentionally */

        case HW_CFG_DL_FW_PATCH:
            SYSLOGI("HW_CFG_DL_FW_PATCH status: %d, opcode 0x%04x", status, opcode);

            p = (uint8_t *)(p_evt_buf + 1);

            if (opcode == HCI_VSC_DOWNLOAD_FW_PATCH) {
                index = *(p + HCI_EVT_CMD_CMPL_DL_FW_PATCH_INDEX);
                SYSLOGI("HW_CFG_DL_FW_PATCH: index %d", index);
                if (index & 0x80) {
                    SYSLOGI("bt hw config completed");
                    btmp_log("bt hw config completed");
                    free(UART_hw_cfg_cb.total_buf);
                    bt_vendor_cbacks->dealloc(p_buf);
                    bt_vendor_cbacks->fwcfg_cb(BT_VND_OP_RESULT_SUCCESS);

                    UART_hw_cfg_cb.state = HW_CFG_UNINIT;
                    is_proceeding = TRUE;
                    break;
                }

                UART_hw_cfg_cb.patch_frag_idx++;
            }

            if (UART_hw_cfg_cb.patch_frag_idx < UART_hw_cfg_cb.patch_frag_cnt) {
                if (UART_hw_cfg_cb.patch_frag_idx == UART_hw_cfg_cb.patch_frag_cnt - 1) {
                    SYSLOGI("HW_CFG_DL_FW_PATCH: send last fw fragment");
					btmp_log("HW_CFG_DL_FW_PATCH: send last fw fragment");
                    UART_hw_cfg_cb.patch_frag_idx |= 0x80;
                    UART_hw_cfg_cb.patch_frag_len = UART_hw_cfg_cb.patch_frag_tail;
                } else {
                    UART_hw_cfg_cb.patch_frag_len = PATCH_FRAGMENT_MAX_SIZE;
                }
            }

            SYSLOGI("patch fragement index %d, len %d",
                    UART_hw_cfg_cb.patch_frag_idx, UART_hw_cfg_cb.patch_frag_len);

            /* download firmware patch in fragment unit */
            p = (uint8_t *)(p_buf + 1);
            UINT16_TO_STREAM(p, HCI_VSC_DOWNLOAD_FW_PATCH);
            *p++ = 1 +  UART_hw_cfg_cb.patch_frag_len; /* parameter length */
            *p++ = UART_hw_cfg_cb.patch_frag_idx;

            p_frag = UART_hw_cfg_cb.total_buf +
                (UART_hw_cfg_cb.patch_frag_idx & 0x7F) * PATCH_FRAGMENT_MAX_SIZE;
            memcpy(p, p_frag, UART_hw_cfg_cb.patch_frag_len);

            p_buf->len = HCI_CMD_PREAMBLE_SIZE + 1 + UART_hw_cfg_cb.patch_frag_len;

            is_proceeding = bt_vendor_cbacks->xmit_cb(HCI_VSC_DOWNLOAD_FW_PATCH,
                                                    p_buf, UART_hw_config_cback);
            break;

            default:
                break;
        } /* switch(UART_hw_cfg_cb.state) */
    } /* if (p_buf != NULL) */

    /* Free the RX event buffer */
    if (bt_vendor_cbacks && (p_mem != NULL))
        bt_vendor_cbacks->dealloc(p_evt_buf);

    if (is_proceeding == FALSE) {
        SYSLOGE("vendor lib fwcfg aborted!!!");
        if (bt_vendor_cbacks) {
            if (p_buf != NULL)
                bt_vendor_cbacks->dealloc(p_buf);

            bt_vendor_cbacks->fwcfg_cb(BT_VND_OP_RESULT_FAIL);
        }

        UART_hw_cfg_cb.state = 0;
    }
}

/** Kick off controller initialization process */
void UART_hw_config_start(bt_hci_if_t proto)
{
    HC_BT_HDR *p_buf = NULL;
    uint8_t *p;

    memset(&UART_hw_cfg_cb, 0, sizeof(bt_hw_cfg_cb_t));
    UART_hw_cfg_cb.state = HW_CFG_UNINIT;
    UART_hw_cfg_cb.dl_fw_flag = 1;

    SYSLOGI("UART_hw_config_start: proto %d[1/H4, 2/H5]", proto);

    if (proto == BT_HCI_IF_UART5) {
        /* H5: start from sending H5 SYNC */
        if (bt_vendor_cbacks) {
            p_buf = (HC_BT_HDR *)bt_vendor_cbacks->alloc(BT_HC_HDR_SIZE + 2);
        }

        if (p_buf) {
            p_buf->event = MSG_STACK_TO_HC_HCI_CMD;
            p_buf->offset = 0;
            p_buf->layer_specific = 0;
            p_buf->len = 2;

            p = (uint8_t *)(p_buf + 1);
            UINT16_TO_STREAM(p, HCI_VSC_H5_INIT);

            UART_hw_cfg_cb.state = HW_CFG_H5_SYNC;
            bt_vendor_cbacks->xmit_cb(HCI_VSC_H5_INIT, p_buf, UART_hw_config_cback);
        } else {
            if (bt_vendor_cbacks) {
                SYSLOGE("bt hw config aborted[no buffer]");
                bt_vendor_cbacks->fwcfg_cb(BT_VND_OP_RESULT_FAIL);
            }
        }
    } else if (proto == BT_HCI_IF_UART4) {
        /* H4: start from reading lmp version */
        UART_hw_cfg_cb.state = HW_CFG_H4_START;
        UART_hw_config_cback(NULL);
    }
}
