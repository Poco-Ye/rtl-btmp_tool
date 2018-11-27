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
#include <dirent.h>

#include "bt_syslog.h"
#include "bt_hci_bdroid.h"
#include "bt_vendor_if.h"
#include "bt_vendor_lib.h"
#include "bt_hwcfg_if.h"

#define CONFIG_MAC_OFFSET_GEN_1_2       (0x3C)      //MAC's OFFSET in config/efuse for realtek generation 1~2 bluetooth chip
#define CONFIG_MAC_OFFSET_GEN_3PLUS     (0x44)      //MAC's OFFSET in config/efuse for rtk generation 3+ bluetooth chip
#define CONFIG_MAC_OFFSET_GEN_4PLUS     (0x30)      //MAC's OFFSET in config/efuse for rtk generation 4+ bluetooth chip

#define MAX_PATCH_SIZE_24K              (1024*24)   //24K
#define MAX_PATCH_SIZE_40K              (1024*40)   //40K


typedef struct {
    uint16_t    vid;
    uint16_t    pid;
    uint16_t    lmp_sub_default;
    uint16_t    lmp_sub;
    uint16_t    eversion;
    char        *mp_patch_name;
    char        *patch_name;
    char        *config_name;
    uint8_t     *fw_cache;
    int         fw_len;
    uint16_t    mac_offset;
    uint32_t    max_patch_size;
} usb_patch_info;

typedef struct {
    uint16_t    vid;
    uint16_t    pid;
} lsusb_info;

lsusb_info usb_dev_list[128] = {{0,0}};
static uint32_t usb_dev_number = 0;


static usb_patch_info usb_fw_patch_table[] = {
/* { vid, pid, lmp_sub_default, lmp_sub, everion, mp_fw_name, fw_name, config_name, fw_cache, fw_len, mac_offset } */
{ 0x0BDA, 0x1724, 0x1200, 0, 0, "mp_rtl8723a_fw", "rtl8723a_fw", "mp_rtl8723a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8723A */
{ 0x0BDA, 0x8723, 0x1200, 0, 0, "mp_rtl8723a_fw", "rtl8723a_fw", "mp_rtl8723a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* 8723AE */
{ 0x0BDA, 0xA723, 0x1200, 0, 0, "mp_rtl8723a_fw", "rtl8723a_fw", "mp_rtl8723a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* 8723AE for LI */
{ 0x0BDA, 0x0723, 0x1200, 0, 0, "mp_rtl8723a_fw", "rtl8723a_fw", "mp_rtl8723a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* 8723AE */
{ 0x13D3, 0x3394, 0x1200, 0, 0, "mp_rtl8723a_fw", "rtl8723a_fw", "mp_rtl8723a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* 8723AE for Azurewave*/

{ 0x0BDA, 0x0724, 0x1200, 0, 0, "mp_rtl8723a_fw", "rtl8723a_fw", "mp_rtl8723a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* 8723AU */
{ 0x0BDA, 0x8725, 0x1200, 0, 0, "mp_rtl8723a_fw", "rtl8723a_fw", "mp_rtl8723a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* 8723AU */
{ 0x0BDA, 0x872A, 0x1200, 0, 0, "mp_rtl8723a_fw", "rtl8723a_fw", "mp_rtl8723a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* 8723AU */
{ 0x0BDA, 0x872B, 0x1200, 0, 0, "mp_rtl8723a_fw", "rtl8723a_fw", "mp_rtl8723a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* 8723AU */

{ 0x0BDA, 0xb720, 0x8723, 0, 0, "mp_rtl8723b_fw", "rtl8723b_fw", "mp_rtl8723bu_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8723BU */
{ 0x0BDA, 0xb72A, 0x8723, 0, 0, "mp_rtl8723b_fw", "rtl8723b_fw", "mp_rtl8723bu_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8723BU */
{ 0x0BDA, 0xb728, 0x8723, 0, 0, "mp_rtl8723b_fw", "rtl8723b_fw", "mp_rtl8723b_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8723BE for LC */
{ 0x0BDA, 0xb723, 0x8723, 0, 0, "mp_rtl8723b_fw", "rtl8723b_fw", "mp_rtl8723b_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8723BE */
{ 0x0BDA, 0xb72B, 0x8723, 0, 0, "mp_rtl8723b_fw", "rtl8723b_fw", "mp_rtl8723b_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8723BE */
{ 0x0BDA, 0xb001, 0x8723, 0, 0, "mp_rtl8723b_fw", "rtl8723b_fw", "mp_rtl8723b_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8723BE for HP */
{ 0x0BDA, 0xb002, 0x8723, 0, 0, "mp_rtl8723b_fw", "rtl8723b_fw", "mp_rtl8723b_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8723BE */
{ 0x0BDA, 0xb003, 0x8723, 0, 0, "mp_rtl8723b_fw", "rtl8723b_fw", "mp_rtl8723b_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8723BE */
{ 0x0BDA, 0xb004, 0x8723, 0, 0, "mp_rtl8723b_fw", "rtl8723b_fw", "mp_rtl8723b_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8723BE */
{ 0x0BDA, 0xb005, 0x8723, 0, 0, "mp_rtl8723b_fw", "rtl8723b_fw", "mp_rtl8723b_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8723BE */

{ 0x13D3, 0x3410, 0x8723, 0, 0, "mp_rtl8723b_fw", "rtl8723b_fw", "mp_rtl8723b_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8723BE for Azurewave */
{ 0x13D3, 0x3416, 0x8723, 0, 0, "mp_rtl8723b_fw", "rtl8723b_fw", "mp_rtl8723b_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8723BE for Azurewave */
{ 0x13D3, 0x3459, 0x8723, 0, 0, "mp_rtl8723b_fw", "rtl8723b_fw", "mp_rtl8723b_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8723BE for Azurewave */
{ 0x0489, 0xE085, 0x8723, 0, 0, "mp_rtl8723b_fw", "rtl8723b_fw", "mp_rtl8723b_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8723BE for Foxconn */
{ 0x0489, 0xE08B, 0x8723, 0, 0, "mp_rtl8723b_fw", "rtl8723b_fw", "mp_rtl8723b_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8723BE for Foxconn */

{ 0x0BDA, 0x2850, 0x8761, 0, 0, "mp_rtl8761a_fw", "rtl8761au_fw", "mp_rtl8761a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8761AU */
{ 0x0BDA, 0xA761, 0x8761, 0, 0, "mp_rtl8761a_fw", "rtl8761au_fw", "mp_rtl8761a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8761AU only */
{ 0x0BDA, 0x818B, 0x8761, 0, 0, "mp_rtl8761a_fw", "rtl8761aw8192eu_fw", "mp_rtl8761aw8192eu_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8761AW + 8192EU */
{ 0x0BDA, 0x818C, 0x8761, 0, 0, "mp_rtl8761a_fw", "rtl8761aw8192eu_fw", "mp_rtl8761aw8192eu_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8761AW + 8192EU */
{ 0x0BDA, 0x8760, 0x8761, 0, 0, "mp_rtl8761a_fw", "rtl8761au8192ee_fw", "mp_rtl8761a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8761AU + 8192EE */
{ 0x0BDA, 0xB761, 0x8761, 0, 0, "mp_rtl8761a_fw", "rtl8761au_fw", "mp_rtl8761a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8761AUV only */
{ 0x0BDA, 0x8761, 0x8761, 0, 0, "mp_rtl8761a_fw", "rtl8761au8192ee_fw", "mp_rtl8761a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8761AU + 8192EE for LI */
{ 0x0BDA, 0x8A60, 0x8761, 0, 0, "mp_rtl8761a_fw", "rtl8761au8812ae_fw", "mp_rtl8761a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8761AU + 8812AE */
{ 0x0BDA, 0x8771, 0x8761, 0, 0, "mp_rtl8761b_fw", "rtl8761bu_fw", "mp_rtl8761b_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_40K}, /* RTL8761BU */

{ 0x0BDA, 0x8821, 0x8821, 0, 0, "mp_rtl8821a_fw", "rtl8821a_fw", "mp_rtl8821a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8821AE */
{ 0x0BDA, 0x0821, 0x8821, 0, 0, "mp_rtl8821a_fw", "rtl8821a_fw", "mp_rtl8821a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8821AE */
{ 0x0BDA, 0x0823, 0x8821, 0, 0, "mp_rtl8821a_fw", "rtl8821a_fw", "mp_rtl8821a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8821AU */
{ 0x13D3, 0x3414, 0x8821, 0, 0, "mp_rtl8821a_fw", "rtl8821a_fw", "mp_rtl8821a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8821AE */
{ 0x13D3, 0x3458, 0x8821, 0, 0, "mp_rtl8821a_fw", "rtl8821a_fw", "mp_rtl8821a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8821AE */
{ 0x13D3, 0x3461, 0x8821, 0, 0, "mp_rtl8821a_fw", "rtl8821a_fw", "mp_rtl8821a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8821AE */
{ 0x13D3, 0x3462, 0x8821, 0, 0, "mp_rtl8821a_fw", "rtl8821a_fw", "mp_rtl8821a_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_1_2, MAX_PATCH_SIZE_24K}, /* RTL8821AE */

{ 0x0BDA, 0xB822, 0x8822, 0, 0, "mp_rtl8822b_fw", "rtl8822b_fw", "mp_rtl8822b_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_3PLUS, MAX_PATCH_SIZE_24K}, /* RTL8822BE */
{ 0x0BDA, 0xB82C, 0x8822, 0, 0, "mp_rtl8822b_fw", "rtl8822b_fw", "mp_rtl8822b_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_3PLUS, MAX_PATCH_SIZE_24K}, /* RTL8822BU */
{ 0x0BDA, 0xB023, 0x8822, 0, 0, "mp_rtl8822b_fw", "rtl8822b_fw", "mp_rtl8822b_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_3PLUS, MAX_PATCH_SIZE_24K}, /* RTL8822BE */
{ 0x0BDA, 0xB703, 0x8703, 0, 0, "mp_rtl8723c_fw", "rtl8723c_fw", "mp_rtl8723c_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_3PLUS, MAX_PATCH_SIZE_24K}, /* RTL8723CU */
{ 0x0BDA, 0xC82C, 0x8822, 0, 0, "mp_rtl8822c_fw", "rtl8822c_fw", "mp_rtl8822c_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_4PLUS, MAX_PATCH_SIZE_40K}, /* RTL8822CU */

/* todo: RTL8703BU */

{ 0x0BDA, 0xD723, 0x8723, 0, 0, "mp_rtl8723d_fw", "rtl8723d_fw", "mp_rtl8723d_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_3PLUS, MAX_PATCH_SIZE_40K}, /* RTL8723DU */
{ 0x0BDA, 0xD720, 0x8723, 0, 0, "mp_rtl8723d_fw", "rtl8723d_fw", "mp_rtl8723d_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_3PLUS, MAX_PATCH_SIZE_40K}, /* RTL8723DE */
{ 0x0BDA, 0xB820, 0x8821, 0, 0, "mp_rtl8821c_fw", "rtl8821c_fw", "mp_rtl8821c_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_3PLUS, MAX_PATCH_SIZE_40K}, /* RTL8821CU */
{ 0x0BDA, 0xC820, 0x8821, 0, 0, "mp_rtl8821c_fw", "rtl8821c_fw", "mp_rtl8821c_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_3PLUS, MAX_PATCH_SIZE_40K}, /* RTL8821CU */
{ 0x0BDA, 0xC821, 0x8821, 0, 0, "mp_rtl8821c_fw", "rtl8821c_fw", "mp_rtl8821c_config", NULL, 0 ,CONFIG_MAC_OFFSET_GEN_3PLUS, MAX_PATCH_SIZE_40K}, /* RTL8821CE */
/* todo: RTL8703CU */

/* NOTE: must append patch entries above the null entry */
{ 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, 0, 0, 0 }
};

void USB_hw_config_cback(void *p_evt_buf);

static bt_hw_cfg_cb_t USB_hw_cfg_cb;

static void fw_reset_timeout(union sigval arg)
{
    SYSLOGI("fw_reset_timeout, timer id %d", *(int *)arg.sival_ptr);

    USB_hw_config_cback(NULL);
}

static int open_device(const char *device)
{
  FILE *file;
  int busnum = 0, devnum = 0, pid = 0, vid = 0;
  char toybuf[4096] = {0};
  sprintf(toybuf, "%s/uevent", device);
  file = fopen(toybuf, "r");
  if (file) {
    int count = 0;

    while (fgets(toybuf, sizeof(toybuf), file))
      if (sscanf(toybuf, "BUSNUM=%u\n", &busnum)
          || sscanf(toybuf, "DEVNUM=%u\n", &devnum)
          || sscanf(toybuf, "PRODUCT=%x/%x/", &vid, &pid)) count++;

    if (count == 3){
      SYSLOGI("Bus %03d Device %03d: ID %04x:%04x\n", busnum, devnum, vid, pid);
      usb_dev_list[usb_dev_number].pid = pid;
      usb_dev_list[usb_dev_number].vid = vid;
      ++usb_dev_number;
    }
    fclose(file);
  }
  return 0;
}

static int scan_dir(const char *dirname)
{
    char devname[PATH_MAX];
    char *filename;
    DIR *dir;
    struct dirent *de;
    dir = opendir(dirname);
    if(dir == NULL)
        return -1;
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while((de = readdir(dir))) {
        if(de->d_name[0] == '.' &&
           (de->d_name[1] == '\0' ||
            (de->d_name[1] == '.' && de->d_name[2] == '\0')))
            continue;
        strcpy(filename, de->d_name);
        //debug("scan_dir: %s\n",devname);
        open_device(devname);
    }
    closedir(dir);
    return 0;
}

static void get_usb_dev_list()
{
    usb_dev_number = 0;
    scan_dir("/sys/bus/usb/devices");
}

static usb_patch_info *USB_get_fw_table_entry()
{
    usb_patch_info *patch_entry = usb_fw_patch_table;

    uint32_t entry_size = sizeof(usb_fw_patch_table) / sizeof(usb_fw_patch_table[0]);
    uint32_t i,k;
    get_usb_dev_list();

    SYSLOGI("%s: usb_dev_number = %d", __func__,usb_dev_number);
    for(k = 0; k < usb_dev_number; k++){
        patch_entry = usb_fw_patch_table;
        for (i = 0; i < entry_size; i++, patch_entry++) {
            if ((usb_dev_list[k].vid == patch_entry->vid)&&(usb_dev_list[k].pid == patch_entry->pid))
                return patch_entry;
        }
    }

   SYSLOGI("%s: No fw table entry found", __func__);
    return NULL;
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
//    patch_item *entry = NULL;
    uint8_t index = 0;
    usb_patch_info* entry = NULL;

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
            is_proceeding = bt_vendor_cbacks->xmit_cb(HCI_READ_LOCAL_VERSION_INFO,
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

            p = (uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_HCI_SUB_VERSION;
            STREAM_TO_UINT16(USB_hw_cfg_cb.hci_subver, p);

            SYSLOGI("LMP sub version 0x%04x, HCI sub version 0x%04x",
                                   USB_hw_cfg_cb.lmp_subver, USB_hw_cfg_cb.hci_subver);
            if (USB_hw_cfg_cb.lmp_subver == ROM_LMP_8723a) {
                USB_hw_cfg_cb.state = HW_CFG_PARSE_FW_PATCH;
                goto CFG_PARSE_FW_PATCH;
            } else {
                p = (uint8_t *)(p_buf + 1);
                UINT16_TO_STREAM(p, HCI_VSC_READ_ROM_VERSION);
                *p = 0; /* parameter length */
                p_buf->len = HCI_CMD_PREAMBLE_SIZE;

                USB_hw_cfg_cb.state = HW_CFG_READ_ROM_VER;
                is_proceeding = bt_vendor_cbacks->xmit_cb(HCI_VSC_READ_ROM_VERSION,
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

            USB_hw_cfg_cb.state = HW_CFG_PARSE_FW_PATCH;

            SYSLOGI("read ROM version status: %d, echo version: %d",
                    status, USB_hw_cfg_cb.rom_ver);
            /* fall through intentionally */

CFG_PARSE_FW_PATCH:
        case HW_CFG_PARSE_FW_PATCH:
            SYSLOGI("USB_hw_config_cback state: %d", USB_hw_cfg_cb.state);

            /* load fw & config files according to patch item */
            entry = USB_get_fw_table_entry();
            //entry = bt_hw_get_patch_item(USB_hw_cfg_cb.lmp_subver, USB_hw_cfg_cb.hci_subver);
            if (entry) {
                USB_hw_cfg_cb.config_len = bt_hw_load_file(&USB_hw_cfg_cb.config_buf,
                                                entry->config_name);
                if (USB_hw_cfg_cb.config_len < 0) {
                    USB_hw_cfg_cb.config_len = 0;
                } else {
                    bt_hw_parse_config(&USB_hw_cfg_cb, vnd_local_bd_addr);
                }

                USB_hw_cfg_cb.fw_len = bt_hw_load_file(&USB_hw_cfg_cb.fw_buf,
                                                entry->mp_patch_name);
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
                    btmp_log("bt hw config completed");
                    free(USB_hw_cfg_cb.total_buf);
                    bt_vendor_cbacks->dealloc(p_buf);
                    bt_vendor_cbacks->fwcfg_cb(BT_VND_OP_RESULT_SUCCESS);

                    USB_hw_cfg_cb.state = HW_CFG_UNINIT;
                    is_proceeding = TRUE;
                    break;
                }

                USB_hw_cfg_cb.patch_frag_idx++;
            }

            if (USB_hw_cfg_cb.patch_frag_idx < USB_hw_cfg_cb.patch_frag_cnt) {
                if (USB_hw_cfg_cb.patch_frag_idx == USB_hw_cfg_cb.patch_frag_cnt - 1) {
                    SYSLOGI("HW_CFG_DL_FW_PATCH: send last fw fragment");
					btmp_log("HW_CFG_DL_FW_PATCH: send last fw fragment");
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

            is_proceeding = bt_vendor_cbacks->xmit_cb(HCI_VSC_DOWNLOAD_FW_PATCH,
                                                    p_buf, USB_hw_config_cback);
            break;

        default:
            break;
        } /* switch(USB_hw_cfg_cb.state) */
    } /* if (p_buf != NULL) */

    /* Free the RX event buffer */
    if (bt_vendor_cbacks && p_evt_buf)
        bt_vendor_cbacks->dealloc(p_evt_buf);

    if (is_proceeding == FALSE) {
        SYSLOGE("bt hw config aborted!!!");
        if (bt_vendor_cbacks) {
            if (p_buf != NULL)
                bt_vendor_cbacks->dealloc(p_buf);

            bt_vendor_cbacks->fwcfg_cb(BT_VND_OP_RESULT_FAIL);
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

    SYSLOGI("USB_hw_config_start");

    /* Start from clearing controller firmware */
    if (bt_vendor_cbacks) {
        p_buf = (HC_BT_HDR *)bt_vendor_cbacks->alloc(BT_HC_HDR_SIZE + HCI_CMD_PREAMBLE_SIZE);
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

        bt_vendor_cbacks->xmit_cb(HCI_VSC_FIRMWARE_RESET, p_buf, NULL);
        /* As firmware reset vendor cmd has not evt returned, we set a timer
         * waiting for 1 second to ensure that the cmd has been completed.
         * It is sick, you know.
         */
        res = hw_cfg_set_timer(&USB_hw_cfg_cb, fw_reset_timeout, 1000);
        if (res < 0) {
            if (bt_vendor_cbacks) {
                SYSLOGE("bt hw config aborted[no timer]");
                bt_vendor_cbacks->fwcfg_cb(BT_VND_OP_RESULT_FAIL);
            }
        }
    } else {
        if (bt_vendor_cbacks) {
            SYSLOGE("bt hw config aborted[no buffer]");
            bt_vendor_cbacks->fwcfg_cb(BT_VND_OP_RESULT_FAIL);
        }
    }
}
