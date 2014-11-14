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
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include "bt_syslog.h"
#include "bt_hci_bdroid.h"
#include "bt_vendor_usb.h"
#include "bt_vendor_lib.h"

/******************************************************************************
**  Constants & Macros
******************************************************************************/
#define BT_CFG_VERSION "2.11"

#define BT_FIRMWARE_DIRECTORY   "/lib/firmware/%s"
#define PATCH_FRAGMENT_MAX_SIZE     252

#define RTK_VENDOR_CONFIG_MAGIC 0x8723ab55
struct rtk_bt_vendor_config_entry {
    uint16_t offset;
    uint8_t entry_len;
    uint8_t entry_data[0];
} __attribute__ ((packed));


struct rtk_bt_vendor_config {
    uint32_t signature;
    uint16_t data_len;
    struct rtk_bt_vendor_config_entry entry[0];
} __attribute__ ((packed));

#define HCI_CMD_MAX_LEN             258

#define HCI_RESET                       0x0C03

#define HCI_VSC_FIRMWARE_RESET          0xFC66
#define HCI_VSC_UPDATE_BAUDRATE         0xFC17
#define HCI_VSC_DOWNLOAD_FW_PATCH       0xFC20
#define HCI_VSC_READ_ROM_VERSION        0xFC6D
#define HCI_READ_LOCAL_VERSION_INFO     0x1001

#define ROM_LMP_NONE                0x0000
#define ROM_LMP_8723a               0x1200
#define ROM_LMP_8723b               0x8723
#define ROM_LMP_8821a               0X8821
#define ROM_LMP_8761a               0X8761

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

#define STREAM_TO_UINT16(u16, p) {u16 = ((uint16_t)(*(p)) + (((uint16_t)(*((p) + 1))) << 8)); (p) += 2;}
#define STREAM_TO_UINT32(u32, p) {u32 = (((uint32_t)(*(p))) + ((((uint32_t)(*((p) + 1)))) << 8) + ((((uint32_t)(*((p) + 2)))) << 16) + ((((uint32_t)(*((p) + 3)))) << 24)); (p) += 4;}
#define UINT16_TO_STREAM(p, u16) {*(p)++ = (uint8_t)(u16); *(p)++ = (uint8_t)((u16) >> 8);}
#define UINT32_TO_STREAM(p, u32) {*(p)++ = (uint8_t)(u32); *(p)++ = (uint8_t)((u32) >> 8); *(p)++ = (uint8_t)((u32) >> 16); *(p)++ = (uint8_t)((u32) >> 24);}

/* Hardware Configuration State */
enum {
    HW_CFG_UNINIT = 0,          /* usb & uart state */
    HW_CFG_FW_RESET,            /* usb specified state */
    HW_CFG_READ_LMP_SUB_VER,    /* usb & uart state */
    HW_CFG_READ_ROM_VER,        /* usb & uart state */
    HW_CFG_START,               /* usb & uart state */
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
} USB_bt_hw_cfg_cb_t;

/******************************************************************************
**  Externs
******************************************************************************/

void USB_hw_config_cback(void *p_evt_buf);

extern uint8_t vnd_local_bd_addr[BD_ADDR_LEN];

/******************************************************************************
**  Static variables
******************************************************************************/

static USB_bt_hw_cfg_cb_t USB_hw_cfg_cb;

/* patch signature: Realtech */
const uint8_t FW_PATCH_SIGNATURE[8] = {0x52, 0x65, 0x61, 0x6C, 0x74, 0x65, 0x63, 0x68};
/* extension section signature: 0x77FD0451 */
const uint8_t EXTENSION_SECTION_SIGNATURE[4] = {0x51, 0x04, 0xFD, 0x77};

uint16_t usb_project_id[] = {
    ROM_LMP_8723a,
    ROM_LMP_8723b,
    ROM_LMP_8821a,
    ROM_LMP_8761a,
    ROM_LMP_NONE
};

typedef struct {
    uint16_t lmp_subver;
    char     *fw_name;
    char     *config_name;
} patch_item;

static patch_item patch_table[] = {
    { ROM_LMP_8723a, "mp_rtl8723a_fw", "mp_rtl8723a_config" },    //RTL8723AU
    { ROM_LMP_8723b, "mp_rtl8723b_fw", "mp_rtl8723b_config" },    //RTL8723BU
    { ROM_LMP_8821a, "mp_rtl8821a_fw", "mp_rtl8821a_config" },    //RTL8821AU
    { ROM_LMP_8761a, "mp_rtl8761a_fw", "mp_rtl8761a_config" },    //RTL8761AU
    /* add entries here*/

    { ROM_LMP_NONE,  "mp_none_fw",     "mp_none_config" }
};

struct patch_entry {
    uint16_t chip_id;
    uint16_t patch_len;
    uint32_t patch_offset;
} __attribute__ ((packed));

struct patch_info {
    uint8_t  signature[8];
    uint32_t fw_ver;
    uint16_t patch_num;
    struct patch_entry entry[0];
} __attribute__ ((packed));

/** sleep unconditionally for timeout milliseconds */
static void ms_delay(uint32_t timeout)
{
    struct timespec delay;
    int err;

    if (timeout == 0)
        return;

    delay.tv_sec = timeout / 1000;
    delay.tv_nsec = 1000 * 1000 * (timeout%1000);

    /* [u]sleep can't be used because it may use SIGALRM */
    do {
        err = nanosleep(&delay, &delay);
    } while (err == -1 && errno == EINTR);
}

static void fw_reset_timeout(union sigval arg)
{
    SYSLOGI("fw_reset_timeout, timer id %d", *(int *)arg.sival_ptr);

    USB_hw_config_cback(NULL);
}

static int hw_cfg_set_timer(USB_bt_hw_cfg_cb_t *cfg_cb,
        void (*bt_hw_notify_func)(union sigval arg), uint32_t to_ms)
{
    int err;
    struct itimerspec ts;
    struct sigevent se;

    se.sigev_notify = SIGEV_THREAD;
    se.sigev_value.sival_ptr = &cfg_cb->timer_id;
    se.sigev_notify_function = bt_hw_notify_func;
    se.sigev_notify_attributes = NULL;

    err = timer_create(CLOCK_MONOTONIC, &se, &cfg_cb->timer_id);
    if (err == -1) {
        SYSLOGE("hw_cfg_set_timer: failed to create timer");
        return err;
    }

    ts.it_value.tv_sec = to_ms / 1000;
    ts.it_value.tv_nsec = 1000000 * (to_ms % 1000);
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;

    err = timer_settime(cfg_cb->timer_id, 0, &ts, 0);
    if (err == -1)
        SYSLOGE("hw_cfg_set_timer: failed to set timer");

    return err;
}

static void hw_cfg_clear_timer(timer_t timer_id)
{
    SYSLOGI("hw_cfg_clear_timer");
    timer_delete(timer_id);
}

static patch_item *get_patch_item(uint16_t lmp_subver)
{
    patch_item *item = patch_table;

    while (lmp_subver != item->lmp_subver) {
        if (item->lmp_subver == 0)
            break;

        item++;
    }

    return item;
}

static uint32_t bt_hw_parse_config(uint8_t *config_buf, size_t filelen, char bt_addr[6])
{
    struct rtk_bt_vendor_config* config = (struct rtk_bt_vendor_config*) config_buf;
    uint16_t config_len = config->data_len, temp = 0;
    struct rtk_bt_vendor_config_entry* entry = config->entry;
    unsigned int i = 0;
    uint32_t baudrate = 0;

    bt_addr[0] = 0; //reset bd addr byte 0 to zero

    if (config->signature != RTK_VENDOR_CONFIG_MAGIC) {
        SYSLOGE("config signature magic number(%x) is not set to RTK_VENDOR_CONFIG_MAGIC", config->signature);
        return 0;
    }

    if (config_len != filelen - sizeof(struct rtk_bt_vendor_config))
    {
        SYSLOGE("config len(%x) is not right(%x)", config_len, filelen-sizeof(struct rtk_bt_vendor_config));
        return 0;
    }

    for (i=0; i<config_len;) {
        temp = entry->entry_len + sizeof(struct rtk_bt_vendor_config_entry);
        i += temp;
        entry = (struct rtk_bt_vendor_config_entry*)((uint8_t*)entry + temp);
    }

    return baudrate;
}

static int bt_hw_load_file(uint8_t **file_buf, char *file_name)
{
    char file_path[PATH_MAX] = {0};
    struct stat st;
    size_t len;
    int fd;

    sprintf(file_path, BT_FIRMWARE_DIRECTORY, file_name);

    SYSLOGI("bt_hw_load_file[%s]", file_path);

    if (stat(file_path, &st) < 0) {
        SYSLOGE("can't access bt file[%s], errno %d", file_path, errno);
        return -1;
    }

    len = st.st_size;

    fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        SYSLOGE("can't open bt file[%s], errno %d", file_path, errno);
        return -1;
    }

    *file_buf = malloc(len);
    if (*file_buf == NULL) {
        SYSLOGE("failed to malloc buf for bt file[%s]", file_path);
        return -1;
    }

    if (read(fd, *file_buf, len) < (ssize_t)len) {
        SYSLOGE("can't read bt file[%s]", file_path);
        free(*file_buf);
        close(fd);
        return -1;
    }

    close(fd);
    return len;
}

static uint8_t bt_hw_parse_project_id(uint8_t *p_buf)
{
    uint8_t opcode;
    uint8_t len;
    uint8_t data;

    data = 0;
    /* The project id resides in extension section.
     * Its opcode is 0x00, and 1 byte len, 1 byte data.
     */
    do {
        opcode = *p_buf;
        len = *(p_buf - 1);
        if (opcode == 0x00) {
            if (len == 1) {
                data = *(p_buf - 2);
                SYSLOGI("bt_hw_parse_project_id: opcode %d, len %d, data %d",
                        opcode, len, data);
                break;
            } else {
                SYSLOGW("bt_hw_parse_project_id: invalid len %d", len);
            }
        }
        /* skip to next field */
        p_buf -= len + 2;
    } while (*p_buf != 0xFF);

    return data;
}

struct patch_entry *bt_hw_get_patch_entry(USB_bt_hw_cfg_cb_t *cfg_cb)
{
    uint16_t i;
    struct patch_info *patch;
    struct patch_entry *entry;
    uint8_t *p;
    uint16_t chip_id;

    patch = (struct patch_info *)cfg_cb->fw_buf;
    entry = (struct patch_entry *)malloc(sizeof(*entry));

    SYSLOGI("bt_hw_get_patch_entry: fw_ver 0x%08x, patch_num %d", patch->fw_ver, patch->patch_num);

    for (i = 0; i < patch->patch_num; i++) {
        p = cfg_cb->fw_buf + 14 + 2*i;
        STREAM_TO_UINT16(chip_id, p);
        if (chip_id == cfg_cb->rom_ver + 1) {
            entry->chip_id = chip_id;
            p = cfg_cb->fw_buf + 14 + 2*patch->patch_num + 2*i;
            STREAM_TO_UINT16(entry->patch_len, p);
            p = cfg_cb->fw_buf + 14 + 4*patch->patch_num + 4*i;
            STREAM_TO_UINT32(entry->patch_offset, p);
            SYSLOGI("bt_hw_get_patch_entry: chip_id %d, patch_len 0x%x, patch_offset 0x%x",
                    entry->chip_id, entry->patch_len, entry->patch_offset);
            break;
        }
    }

    if (i == patch->patch_num) {
        SYSLOGE("bt_hw_get_patch_entry: failed to get etnry");
        free(entry);
        entry = NULL;
    }

    return entry;
}

static void bt_hw_extract_firmware(USB_bt_hw_cfg_cb_t *cfg_cb)
{
    struct patch_entry *entry;
    uint8_t proj_id;
    struct patch_info *patch = (struct patch_info *)cfg_cb->fw_buf;

    /* malloc buf for firmware & config files */
    cfg_cb->total_buf = malloc(cfg_cb->fw_len + cfg_cb->config_len);
    if (!cfg_cb->total_buf) {
        SYSLOGE("bt_hw_extract_firmware: failed to allocate buf");
        cfg_cb->dl_fw_flag = 0;
        return;
    } else {
        cfg_cb->total_len = cfg_cb->fw_len + cfg_cb->config_len;
    }

    /* old style firmware patch, only for 8723a series */
    if (cfg_cb->lmp_subver == ROM_LMP_8723a) {
        if (!memcmp(cfg_cb->fw_buf, FW_PATCH_SIGNATURE, 8)) {
            SYSLOGE("bt_hw_extract_firmware: 8723au signature check error");
            cfg_cb->dl_fw_flag = 0;
            goto free;
        } else {
            /* copy fw & config files directly */
            memcpy(cfg_cb->total_buf, cfg_cb->fw_buf, cfg_cb->fw_len);
            memcpy(cfg_cb->total_buf + cfg_cb->fw_len, cfg_cb->config_buf, cfg_cb->config_len);
            cfg_cb->dl_fw_flag = 1;
            goto free;
        }
    }

    /* new style firmware patch, check patch signature first */
    if (memcmp(cfg_cb->fw_buf, FW_PATCH_SIGNATURE, 8)) {
        SYSLOGE("bt_hw_extract_firmware: signature check error");
        cfg_cb->dl_fw_flag = 0;
        goto free;
    }

    /* check the extension section signature */
    if (memcmp(cfg_cb->fw_buf + cfg_cb->fw_len - 4, EXTENSION_SECTION_SIGNATURE, 4)) {
        SYSLOGE("bt_hw_extract_firmware: extension section signature check error");
        cfg_cb->dl_fw_flag = 0;
        goto free;
    }

    /* get the project id, check with lmp subversion */
    proj_id = bt_hw_parse_project_id(cfg_cb->fw_buf + cfg_cb->fw_len - 5);
    if (cfg_cb->lmp_subver == usb_project_id[proj_id]) {
        SYSLOGI("lmp sub verson is 0x%04x, project_id is 0x%04x, match!",
                cfg_cb->lmp_subver, usb_project_id[proj_id]);
    } else {
        SYSLOGI("lmp sub verson is 0x%04x, project_id is 0x%04x, mismatch!",
                cfg_cb->lmp_subver, usb_project_id[proj_id]);
        cfg_cb->dl_fw_flag = 0;
        goto free;
    }

    /* get the patch entry according to chip id */
    entry = bt_hw_get_patch_entry(cfg_cb);
    if (entry) {
        cfg_cb->total_len = entry->patch_len + cfg_cb->config_len;
        memcpy(cfg_cb->total_buf, cfg_cb->fw_buf + entry->patch_offset, entry->patch_len);
        memcpy(cfg_cb->total_buf + entry->patch_len - 4, &patch->fw_ver, 4);
        memcpy(cfg_cb->total_buf + entry->patch_len, cfg_cb->config_buf, cfg_cb->config_len);
        free(entry);
    } else {
        cfg_cb->dl_fw_flag = 0;
        goto free;
    }

    /* everything works whell when arriving here */
    cfg_cb->dl_fw_flag = 1;

free:
    if (cfg_cb->fw_len > 0) {
        free(cfg_cb->fw_buf);
        cfg_cb->fw_len = 0;
    }
    if (cfg_cb->config_len > 0) {
        free(cfg_cb->config_buf);
        cfg_cb->config_len = 0;
    }
}

static int bt_hw_download_patch(USB_bt_hw_cfg_cb_t *cfg_cb, HC_BT_HDR *p_buf)
{
    uint8_t ret = FALSE;
    uint8_t *p = (uint8_t *)(p_buf + 1);
    uint8_t *p_frag = cfg_cb->total_buf;

    SYSLOGI("patch fragement index %d, len %d", cfg_cb->patch_frag_idx, cfg_cb->patch_frag_len);

    p_frag += (cfg_cb->patch_frag_idx & 0x7F) * PATCH_FRAGMENT_MAX_SIZE;

    UINT16_TO_STREAM(p, HCI_VSC_DOWNLOAD_FW_PATCH);
    *p++ = 1 + cfg_cb->patch_frag_len; /* parameter length */
    *p++ = cfg_cb->patch_frag_idx;
    memcpy(p, p_frag, cfg_cb->patch_frag_len);

    p_buf->len = HCI_CMD_PREAMBLE_SIZE + 1 + cfg_cb->patch_frag_len;

    cfg_cb->state = HW_CFG_DL_FW_PATCH;

    ret = USB_bt_vendor_cbacks->xmit_cb(HCI_VSC_DOWNLOAD_FW_PATCH, p_buf, USB_hw_config_cback);
    return ret;
}

/** Callback function for controller configurationn */
void USB_hw_config_cback(void *p_mem)
{
    HC_BT_HDR *p_evt_buf = NULL;
    HC_BT_HDR *p_buf = NULL;
    uint8_t   *p = NULL;
    uint8_t   status = 0;
    uint16_t  opcode = 0;
    uint8_t   is_proceeding = FALSE;
    patch_item *entry = NULL;
    uint8_t index = 0;

    if (p_mem != NULL) {
        p_evt_buf = (HC_BT_HDR *)p_mem;
        status = *((uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_STATUS_RET_BYTE);
        p = (uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_OPCODE;
        STREAM_TO_UINT16(opcode,p);
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

        SYSLOGI("USB_hw_cfg_cback state: %d", USB_hw_cfg_cb.state);

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

            is_proceeding = USB_bt_vendor_cbacks->xmit_cb(HCI_READ_LOCAL_VERSION_INFO, \
                                                    p_buf, USB_hw_config_cback);
            break;

        case HW_CFG_READ_LMP_SUB_VER:
            if (opcode != HCI_READ_LOCAL_VERSION_INFO) {
                SYSLOGE("ERROR! USB_hw_cfg_cback state: %d, received opcode 0x%04x",
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
                SYSLOGE("ERROR! USB_hw_cfg_cback state: %d, received opcode 0x%04x",
                        USB_hw_cfg_cb.state, opcode);
                break;
            }

            p = (uint8_t *)(p_evt_buf + 1);

            if (status == 0) {
                USB_hw_cfg_cb.rom_ver = *(p + HCI_EVT_CMD_CMPL_ROM_VERSION);
            } else if (status == 1) {
                USB_hw_cfg_cb.rom_ver = 0; //default a
            } else {
                is_proceeding = FALSE;
                SYSLOGE("read ROM version status error!");
                break;
            }

            USB_hw_cfg_cb.state = HW_CFG_START;

            SYSLOGI("read ROM version status: %d, rom echo version: %d",
                    status, USB_hw_cfg_cb.rom_ver);
            /* fall through intentionally */
CFG_START:
        case HW_CFG_START:
            SYSLOGI("USB_hw_config_cback state: %d", USB_hw_cfg_cb.state);

            /* load fw & config files according to patch item */
            entry = get_patch_item(USB_hw_cfg_cb.lmp_subver);
            if (entry) {
                USB_hw_cfg_cb.config_len = bt_hw_load_file(&USB_hw_cfg_cb.config_buf,
                                                entry->config_name);
                if (USB_hw_cfg_cb.config_len < 0) {
                    USB_hw_cfg_cb.config_len = 0;
                } else {
                    //bt_hw_parse_config(&USB_hw_cfg_cb);
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

            is_proceeding = bt_hw_download_patch(&USB_hw_cfg_cb, p_buf);
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

    memset(&USB_hw_cfg_cb, 0, sizeof(USB_bt_hw_cfg_cb_t));
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
