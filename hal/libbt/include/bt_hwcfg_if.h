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
 *  Filename:      bt_hwcfg_if.h
 *
 *  Description:   Contains controller-specific I/F definitions for uart & usb HCI
 *                 common configure.
 *
 ******************************************************************************/

#ifndef BT_HWCFG_IF_H
#define BT_HWCFG_IF_H

#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include "bt_hci_bdroid.h"

#define BT_CFG_VERSION      "2.11"

#define BT_FIRMWARE_DIRECTORY       "/lib/firmware/%s"
#define HCI_CMD_MAX_LEN             258
#define PATCH_FRAGMENT_MAX_SIZE     252
#define BT_CONFIG_SIGNATURE         0x8723ab55

#define HCI_RESET                       0x0C03
#define HCI_VSC_FIRMWARE_RESET          0xFC66
#define HCI_VSC_UPDATE_BAUDRATE         0xFC17
#define HCI_VSC_DOWNLOAD_FW_PATCH       0xFC20
#define HCI_VSC_READ_ROM_VERSION        0xFC6D
#define HCI_READ_LOCAL_VERSION_INFO     0x1001

#define HCI_VSC_H5_INIT                 0xFCEE
#define H5_SYNC_REQ_SIZE                2
#define H5_SYNC_RESP_SIZE               2
#define H5_CONF_REQ_SIZE                3
#define H5_CONF_RESP_SIZE               2

#define HCI_EVT_CMD_CMPL_STATUS_RET_BYTE        5
#define HCI_EVT_CMD_CMPL_LOCAL_NAME_STRING      6
#define HCI_EVT_CMD_CMPL_LOCAL_BDADDR_ARRAY     6
#define HCI_EVT_CMD_CMPL_OPCODE                 3
#define HCI_EVT_CMD_CMPL_LMP_SUB_VERSION        12
#define HCI_EVT_CMD_CMPL_ROM_VERSION            6
#define HCI_EVT_CMD_CMPL_DL_FW_PATCH_INDEX      6
#define UPDATE_BAUDRATE_CMD_PARAM_SIZE          6
#define HCI_CMD_PREAMBLE_SIZE                   3
#define BD_ADDR_LEN                             6

#define ROM_LMP_NONE                0x0000
#define ROM_LMP_8723a               0x1200
#define ROM_LMP_8723b               0x8723
#define ROM_LMP_8821a               0X8821
#define ROM_LMP_8761a               0X8761

#define STREAM_TO_UINT16(u16, p) {u16 = ((uint16_t)(*(p)) + (((uint16_t)(*((p) + 1))) << 8)); (p) += 2;}
#define STREAM_TO_UINT32(u32, p) {u32 = (((uint32_t)(*(p))) + ((((uint32_t)(*((p) + 1)))) << 8) + ((((uint32_t)(*((p) + 2)))) << 16) + ((((uint32_t)(*((p) + 3)))) << 24)); (p) += 4;}
#define UINT16_TO_STREAM(p, u16) {*(p)++ = (uint8_t)(u16); *(p)++ = (uint8_t)((u16) >> 8);}
#define UINT32_TO_STREAM(p, u32) {*(p)++ = (uint8_t)(u32); *(p)++ = (uint8_t)((u32) >> 8); *(p)++ = (uint8_t)((u32) >> 16); *(p)++ = (uint8_t)((u32) >> 24);}

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_le16(d)  (d)
#define cpu_to_le32(d)  (d)
#define le16_to_cpu(d)  (d)
#define le32_to_cpu(d)  (d)
#else
#define cpu_to_le16(d)  bswap_16(d)
#define cpu_to_le32(d)  bswap_32(d)
#define le16_to_cpu(d)  bswap_16(d)
#define le32_to_cpu(d)  bswap_32(d)
#endif

struct bt_config_entry {
    uint16_t offset;
    uint8_t entry_len;
    uint8_t entry_data[0];
} __attribute__ ((packed));

struct bt_config_info {
    uint32_t signature;
    uint16_t data_len;
    struct bt_config_entry entry[0];
} __attribute__ ((packed));

typedef struct {
    uint16_t lmp_subver;
    char     *fw_name;
    char     *config_name;
} patch_item;

struct bt_patch_entry {
    uint16_t chip_id;
    uint16_t patch_len;
    uint32_t patch_offset;
} __attribute__ ((packed));

struct bt_patch_info {
    uint8_t  signature[8];
    uint32_t fw_ver;
    uint16_t patch_num;
    struct bt_patch_entry entry[0];
} __attribute__ ((packed));

/* Hardware Configuration State */
enum {
    HW_CFG_UNINIT = 0,          /* usb & uart state */
    HW_CFG_H5_SYNC,             /* uart(h5) specified state */
    HW_CFG_FW_RESET,            /* usb specified state */
    HW_CFG_READ_LMP_SUB_VER,    /* usb & uart state */
    HW_CFG_READ_ROM_VER,        /* usb & uart state */
    HW_CFG_START,               /* usb & uart state */
    HW_CFG_SET_CNTRL_BAUDRATE,  /* uart specified state */
    HW_CFG_SET_HOST_BAUDRATE,   /* uart specified state */
    HW_CFG_DL_FW_PATCH          /* usb & uart state */
};

/* h/w config control block */
typedef struct {
    uint8_t  state;           /* Hardware configuration state */
    uint8_t  rom_ver;         /* ROM echo version */
    uint16_t lmp_subver;      /* LMP sub version */
    timer_t  timer_id;        /* hw cfg specified timer */
    int      fw_len;          /* FW patch file len */
    int      config_len;      /* Config patch file len */
    int      total_len;       /* FW & config extracted buf len */
    uint8_t  *fw_buf;         /* FW patch file buf */
    uint8_t  *config_buf;     /* Config patch file buf */
    uint8_t  *total_buf;      /* FW & config extracted buf */
    uint8_t  dl_fw_flag;      /* Flag for download FW */
    uint8_t  patch_frag_cnt;  /* Patch fragment count download */
    uint8_t  patch_frag_idx;  /* Current patch fragment index */
    uint8_t  patch_frag_len;  /* Patch fragment length */
    uint8_t  patch_frag_tail; /* Last patch fragment length */
    uint32_t baudrate[2];     /* Host(0) & controller(1) buadrate */
    uint8_t  hw_flow_cntrl;   /* Uart flow control, bit7:set, bit0:enable */
} bt_hw_cfg_cb_t;

void ms_delay(uint32_t timeout);

int hw_cfg_set_timer(bt_hw_cfg_cb_t *cfg_cb,
        void (*bt_hw_notify_func)(union sigval arg), uint32_t to_ms);

void hw_cfg_clear_timer(timer_t timer_id);

patch_item *bt_hw_get_patch_item(uint16_t lmp_subver);

int bt_hw_load_file(uint8_t **file_buf, char *file_name);

int bt_hw_parse_config(bt_hw_cfg_cb_t *cfg_cb, uint8_t *bt_addr);

uint8_t bt_hw_parse_project_id(uint8_t *p_buf);

struct bt_patch_entry *bt_hw_get_patch_entry(bt_hw_cfg_cb_t *cfg_cb);

void bt_hw_extract_firmware(bt_hw_cfg_cb_t *cfg_cb);

#endif /* BT_HWCFG_IF_H */
