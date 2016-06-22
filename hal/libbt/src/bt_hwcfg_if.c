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
 *  Filename:      bt_hwcfg_if.c
 *
 *  Description:   Contains controller-specific I/F functions for uart & usb HCI
 *                 common configure.
 *
 ******************************************************************************/

#define LOG_TAG "bt_hwcfg_if"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <linux/limits.h>

#include "bt_syslog.h"
#include "bt_hci_bdroid.h"
#include "bt_vendor_lib.h"
#include "bt_hwcfg_if.h"

/* patch signature: Realtech */
const uint8_t FW_PATCH_SIGNATURE[8] = {0x52, 0x65, 0x61, 0x6C, 0x74, 0x65, 0x63, 0x68};
/* extension section signature: 0x77FD0451 */
const uint8_t EXTENSION_SECTION_SIGNATURE[4] = {0x51, 0x04, 0xFD, 0x77};

uint16_t project_id[] = {
    ROM_LMP_8723a,
    ROM_LMP_8723b,
    ROM_LMP_8821a,
    ROM_LMP_8761a,
    ROM_LMP_8703a,
    ROM_LMP_8763a,
    ROM_LMP_8703b,
    ROM_LMP_8723c,
    ROM_LMP_8822b,
    ROM_LMP_8723d,
    ROM_LMP_8821c,
    ROM_LMP_NONE
};

static patch_item patch_table[] = {
    { ROM_LMP_8723a, ROM_HCI_8723a, "mp_rtl8723a_fw", "mp_rtl8723a_config" },    //RTL8723A
    { ROM_LMP_8723b, ROM_HCI_8723b, "mp_rtl8723b_fw", "mp_rtl8723b_config" },    //RTL8723B
    { ROM_LMP_8821a, ROM_HCI_8821a, "mp_rtl8821a_fw", "mp_rtl8821a_config" },    //RTL8821A
    { ROM_LMP_8761a, ROM_HCI_8761a, "mp_rtl8761a_fw", "mp_rtl8761a_config" },    //RTL8761A
    { ROM_LMP_8703a, ROM_HCI_8703a, "mp_rtl8703a_fw", "mp_rtl8703a_config" },    //RTL8703A
    { ROM_LMP_8763a, ROM_HCI_8763a, "mp_rtl8763a_fw", "mp_rtl8763a_config" },    //RTL8763A
    { ROM_LMP_8703b, ROM_HCI_8703b, "mp_rtl8703b_fw", "mp_rtl8703b_config" },    //RTL8703B
    { ROM_LMP_8723c, ROM_HCI_8723c, "mp_rtl8723c_fw", "mp_rtl8723c_config" },    //RTL8723C
    { ROM_LMP_8822b, ROM_HCI_8822b, "mp_rtl8822b_fw", "mp_rtl8822b_config" },    //RTL8822B
    { ROM_LMP_8723d, ROM_HCI_8723d, "mp_rtl8723d_fw", "mp_rtl8723d_config" },    //RTL8723D
    { ROM_LMP_8821c, ROM_HCI_8821c, "mp_rtl8821c_fw", "mp_rtl8821c_config" },    //RTL8821C
    /* add entries here*/

    { ROM_LMP_NONE,  ROM_HCI_NONE,  "mp_none_fw",     "mp_none_config" }
};

/** sleep unconditionally for timeout milliseconds */
void ms_delay(uint32_t timeout)
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

int hw_cfg_set_timer(bt_hw_cfg_cb_t *cfg_cb,
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

void hw_cfg_clear_timer(timer_t timer_id)
{
    SYSLOGI("hw_cfg_clear_timer");
    timer_delete(timer_id);
}

patch_item *bt_hw_get_patch_item(uint16_t lmp_subver, uint16_t hci_subver)
{
    patch_item *item = patch_table;

    while ((lmp_subver != item->lmp_subver) || (hci_subver != item->hci_subver)) {
        if ((item->lmp_subver == 0) && (item->hci_subver == 0))
            break;

        item++;
    }

    return item;
}

int bt_hw_load_file(uint8_t **file_buf, char *file_name)
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

int bt_hw_parse_config(bt_hw_cfg_cb_t *cfg_cb, uint8_t *bt_addr)
{
    struct bt_config_info *config;
    struct bt_config_entry *entry;
    uint16_t config_len;
    uint16_t offset;
    uint8_t *p;
    uint16_t i = 0;

    config = (struct bt_config_info *)cfg_cb->config_buf;
    if (config) {
        config_len = le16_to_cpu(config->data_len);
        entry = config->entry;
    } else {
        SYSLOGE("failed to get config file buf");
        return -1;
    }


    if (le32_to_cpu(config->signature) != BT_CONFIG_SIGNATURE) {
        SYSLOGE("failed to check config file signature 0x%08x", config->signature);
        return -1;
    }

    if (config_len != cfg_cb->config_len - sizeof(struct bt_config_info)) {
        SYSLOGE("failed to check config file length, stated[0x%04x], calculated[0x%04x]",
                config_len, cfg_cb->config_len - sizeof(struct bt_config_info));
        return -1;
    }

    for (i = 0; i < config_len; ) {

        switch (le16_to_cpu(entry->offset)) {
            /*
            case 0x3c:
            {
                int j=0;
                for (j=0; j<entry->entry_len; j++)
                    entry->entry_data[j] = bt_addr[entry->entry_len - 1 - j];
                break;
            }
            */
        case 0x0c:
            p = (uint8_t *)entry->entry_data;
            STREAM_TO_UINT32(cfg_cb->baudrate[1], p);
            if (entry->entry_len >= 12) {
                cfg_cb->hw_flow_cntrl |= 0x80; /* bit7 set hw flow control */
                if (entry->entry_data[12] & 0x04) /* offset 0x18, bit2 */
                    cfg_cb->hw_flow_cntrl |= 1; /* bit0 enable hw flow control */
            }
            SYSLOGI("bt_hw_parse_config: baudrate 0x%08x, hw flow control 0x%02x",
                    cfg_cb->baudrate[1], cfg_cb->hw_flow_cntrl);
            break;

        default:
                SYSLOGI("bt_hw_parse_config: config offset(0x%04x), length(0x%02x)",
                        entry->offset, entry->entry_len);
                break;
        }

        offset = entry->entry_len + sizeof(struct bt_config_entry);
        entry = (struct bt_config_entry *)((uint8_t *)entry + offset);
        i += offset;
    }

    return 0;
}

uint8_t bt_hw_parse_project_id(uint8_t *p_buf)
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

struct bt_patch_entry *bt_hw_get_patch_entry(bt_hw_cfg_cb_t *cfg_cb)
{
    uint16_t i;
    struct bt_patch_info *patch;
    struct bt_patch_entry *entry;
    uint32_t fw_ver;
    uint16_t patch_num;
    uint8_t *p;
    uint16_t chip_id;

    patch = (struct bt_patch_info *)cfg_cb->fw_buf;
    if (patch) {
        fw_ver = le32_to_cpu(patch->fw_ver);
        patch_num = le16_to_cpu(patch->patch_num);
        SYSLOGI("bt_hw_get_patch_entry: fw_ver 0x%08x, patch_num %d", fw_ver, patch_num);
    } else {
        SYSLOGE("bt_hw_get_patch_entry: No BT patch info found");
        return NULL;
    }

    entry = (struct bt_patch_entry *)malloc(sizeof(*entry));
    if (!entry) {
        SYSLOGE("bt_hw_get_patch_entry: failed to allocate mem for BT patch entry");
        return NULL;
    }

    for (i = 0; i < patch_num; i++) {
        p = cfg_cb->fw_buf + 14 + 2*i;
        STREAM_TO_UINT16(chip_id, p);
        if (chip_id == cfg_cb->rom_ver + 1) {
            entry->chip_id = chip_id;
            p = cfg_cb->fw_buf + 14 + 2*patch_num + 2*i;
            STREAM_TO_UINT16(entry->patch_len, p);
            p = cfg_cb->fw_buf + 14 + 4*patch_num + 4*i;
            STREAM_TO_UINT32(entry->patch_offset, p);
            SYSLOGI("bt_hw_get_patch_entry: chip_id %d, patch_len 0x%x, patch_offset 0x%x",
                    entry->chip_id, entry->patch_len, entry->patch_offset);
            break;
        }
    }

    if (i == patch_num) {
        SYSLOGE("bt_hw_get_patch_entry: failed to get etnry");
        free(entry);
        entry = NULL;
    }

    return entry;
}

void bt_hw_extract_firmware(bt_hw_cfg_cb_t *cfg_cb)
{
    struct bt_patch_entry *entry;
    uint8_t proj_id;
    struct bt_patch_info *patch = (struct bt_patch_info *)cfg_cb->fw_buf;

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
    if (cfg_cb->lmp_subver == project_id[proj_id]) {
        SYSLOGI("lmp sub verson is 0x%04x, project_id is 0x%04x, match!",
                cfg_cb->lmp_subver, project_id[proj_id]);
    } else {
        SYSLOGI("lmp sub verson is 0x%04x, project_id is 0x%04x, mismatch!",
                cfg_cb->lmp_subver, project_id[proj_id]);
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
