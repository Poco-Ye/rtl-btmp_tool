/******************************************************************************
 *
 *  Copyright (C) 2012 The Android Open Source Project
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
 *  Filename:      bluetoothtest.c
 *
 *  Description:   Bluetooth MP Test application
 *
 ***********************************************************************************/
#define LOG_TAG "rtlmp_test"

#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <linux/capability.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include <private/android_filesystem_config.h>
#include <android/log.h>
#include <utils/Log.h>

#include <hardware/hardware.h>
#include "bluetoothmp.h"

/************************************************************************************
**  Constants & Macros
************************************************************************************/

#define PID_FILE "/data/.bdt_pid"

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif


/************************************************************************************
**  Local type definitions
************************************************************************************/

/************************************************************************************
**  Static variables
************************************************************************************/

static unsigned char main_done = 0;
static bt_status_t status;
static int bt_try_enable = 0; /* 0: try to disable; 1: try to enable */

/* Main API */
static bluetooth_device_t* bt_device;

const bt_interface_t* sBtInterface = NULL;

static gid_t groups[] = { AID_NET_BT, AID_INET, AID_NET_BT_ADMIN,
                          AID_SYSTEM, AID_MISC, AID_SDCARD_RW,
                          AID_NET_ADMIN, AID_VPN};

/* Set to 1 when the Bluedroid stack is enabled */
static unsigned char bt_enabled = 0;

/************************************************************************************
**  Static functions
************************************************************************************/

static void process_cmd(char *p, unsigned char is_job);
static void job_handler(void *param);

/************************************************************************************
**  Externs
************************************************************************************/

/*****************************************************************************
**   Logger API
*****************************************************************************/

static void bdt_log(const char *fmt_str, ...)
{
    static char buffer[1024];
    va_list ap;

    va_start(ap, fmt_str);
    vsnprintf(buffer, 1024, fmt_str, ap);
    va_end(ap);

    fprintf(stdout, "%s\n", buffer);
}

/*****************************************************************************
** Android's init.rc does not yet support applying linux capabilities
*****************************************************************************/

static void config_permissions(void)
{
    struct __user_cap_header_struct header;
    struct __user_cap_data_struct cap;

    ALOGI("set_aid_and_cap : pid %d, uid %d gid %d", getpid(), getuid(), getgid());

    header.pid = 0;

    prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0);

    setuid(AID_BLUETOOTH);
    setgid(AID_NET_BT_STACK);

    header.version = _LINUX_CAPABILITY_VERSION;

    cap.effective = cap.permitted = cap.inheritable =
                    1 << CAP_NET_RAW |
                    1 << CAP_NET_ADMIN |
                    1 << CAP_NET_BIND_SERVICE |
                    1 << CAP_SYS_RAWIO |
                    1 << CAP_SYS_NICE |
                    1 << CAP_SETGID;

    capset(&header, &cap);
    setgroups(sizeof(groups)/sizeof(groups[0]), groups);
}

static void hex_dump(char *msg, void *data, int size, int trunc)
{
    unsigned char *p = data;
    unsigned char c;
    int n;
    char bytestr[4] = {0};
    char addrstr[10] = {0};
    char hexstr[ 16*3 + 5] = {0};
    char charstr[16*1 + 5] = {0};

    ALOGI("%s  \n", msg);

    /* truncate */
    if(trunc && (size>32))
        size = 32;

    for(n=1;n<=size;n++) {
        if (n%16 == 1) {
            /* store address for this line */
            snprintf(addrstr, sizeof(addrstr), "%.4x",
               ((unsigned int)p-(unsigned int)data) );
        }

        c = *p;
        if (isalnum(c) == 0) {
            c = '.';
        }

        /* store hex str (for left side) */
        snprintf(bytestr, sizeof(bytestr), "%02X ", *p);
        strncat(hexstr, bytestr, sizeof(hexstr)-strlen(hexstr)-1);

        /* store char str (for right side) */
        snprintf(bytestr, sizeof(bytestr), "%c", c);
        strncat(charstr, bytestr, sizeof(charstr)-strlen(charstr)-1);

        if(n%16 == 0) {
            /* line completed */
            ALOGI("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
            hexstr[0] = 0;
            charstr[0] = 0;
        } else if(n%8 == 0) {
            /* half line: add whitespaces */
            strncat(hexstr, "  ", sizeof(hexstr)-strlen(hexstr)-1);
            strncat(charstr, " ", sizeof(charstr)-strlen(charstr)-1);
        }
        p++; /* next byte */
    }

    if (strlen(hexstr) > 0) {
        /* print rest of buffer if not empty */
        ALOGI("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
    }
}

/*******************************************************************************
 ** Console helper functions
 *******************************************************************************/

void skip_blanks(char **p)
{
    while (**p == ' ')
        (*p)++;
}

uint32_t get_int(char **p, int DefaultValue)
{
  uint32_t Value = 0;
  unsigned char   UseDefault;

  UseDefault = 1;
  skip_blanks(p);

  while ( ((**p)<= '9' && (**p)>= '0') )
    {
      Value = Value * 10 + (**p) - '0';
      UseDefault = 0;
      (*p)++;
    }

  if (UseDefault)
    return DefaultValue;
  else
    return Value;
}

int get_signed_int(char **p, int DefaultValue)
{
  int    Value = 0;
  unsigned char   UseDefault;
  unsigned char  NegativeNum = 0;

  UseDefault = 1;
  skip_blanks(p);

  if ( (**p) == '-')
    {
      NegativeNum = 1;
      (*p)++;
    }
  while ( ((**p)<= '9' && (**p)>= '0') )
    {
      Value = Value * 10 + (**p) - '0';
      UseDefault = 0;
      (*p)++;
    }

  if (UseDefault)
    return DefaultValue;
  else
    return ((NegativeNum == 0)? Value : -Value);
}

void get_str(char **p, char *buffer)
{
    skip_blanks(p);

    while (**p != '\0' && **p != ' ') {
        *buffer = **p;
        (*p)++;
        buffer++;
    }

    /* make sure p is pointed to no pre-blank str */
    while (**p == ' ')
        (*p)++;

    *buffer = '\0';
}

uint32_t get_hex(char **p, int DefaultValue)
{
  uint32_t Value = 0;
  unsigned char   UseDefault;

  UseDefault = 1;
  skip_blanks(p);

  while ((**p == '0') && ( (*(*p+1) == 'x') ||(*(*p+1) == 'X') ))
    (*p) = (*p)+2;

  while ( ((**p)<= '9' && (**p)>= '0') ||
          ((**p)<= 'f' && (**p)>= 'a') ||
          ((**p)<= 'F' && (**p)>= 'A') )
    {
      if (**p >= 'a')
        Value = Value * 16 + (**p) - 'a' + 10;
      else if (**p >= 'A')
        Value = Value * 16 + (**p) - 'A' + 10;
      else
        Value = Value * 16 + (**p) - '0';
      UseDefault = 0;
      (*p)++;
    }

  if (UseDefault)
    return DefaultValue;
  else
    return Value;
}

void get_bdaddr(const char *str, bt_bdaddr_t *bd) {
    char *d = ((char *)bd), *endp;
    int i;
    for(i = 0; i < 6; i++) {
        *d++ = strtol(str, &endp, 16);
        if (*endp != ':' && i != 5) {
            memset(bd, 0, sizeof(bt_bdaddr_t));
            return;
        }
        str = endp + 1;
    }
}

#define is_cmd(str) ((strlen(str) == strlen(cmd)) && strncmp((const char *)&cmd, str, strlen(str)) == 0)
#define if_cmd(str)  if (is_cmd(str))

typedef void (t_console_cmd_handler) (char *p);

typedef struct {
    const char *name;
    t_console_cmd_handler *handler;
    const char *help;
    unsigned char is_job;
} t_cmd;

t_console_cmd_handler *mp_cur_handler = NULL;

const t_cmd console_cmd_list[];
static int console_cmd_maxlen = 0;

static void cmdjob_handler(void *param)
{
    char *job_cmd = (char*)param;

    ALOGI("cmdjob starting (%s)", job_cmd);

    process_cmd(job_cmd, 1);

    ALOGI("cmdjob terminating");

    free(job_cmd);
}

static int create_cmdjob(char *cmd)
{
    pthread_t thread_id;
    char *job_cmd;

    job_cmd = malloc(strlen(cmd)+1); /* freed in job handler */
    strcpy(job_cmd, cmd);

    if (pthread_create(&thread_id, NULL,
                       (void*)cmdjob_handler, (void*)job_cmd)!=0)
      perror("pthread_create");

    return 0;
}

/*******************************************************************************
 ** Load stack lib
 *******************************************************************************/

int HAL_load(void)
{
    int err = 0;

    hw_module_t* module;
    hw_device_t* device;

    ALOGI("Loading HAL lib + extensions");

    err = hw_get_module(BT_HARDWARE_MODULE_ID, (hw_module_t const**)&module);
    if (err == 0) {
        err = module->methods->open(module, BT_HARDWARE_MODULE_ID, &device);
        if (err == 0) {
            bt_device = (bluetooth_device_t *)device;
            sBtInterface = bt_device->get_bluetooth_interface();
        }
    }

    ALOGI("HAL library loaded (%s)", strerror(err));

    return err;
}

int HAL_unload(void)
{
    int err = 0;

    ALOGI("Unloading HAL lib");

    sBtInterface = NULL;

    ALOGI("HAL library unloaded (%s)", strerror(err));

    return err;
}

/*******************************************************************************
 ** HAL test functions & callbacks
 *******************************************************************************/

void setup_test_env(void)
{
    int i = 0;

    while (console_cmd_list[i].name != NULL)
    {
        console_cmd_maxlen = MAX(console_cmd_maxlen, (int)strlen(console_cmd_list[i].name));
        i++;
    }
}

void check_return_status(const char *mp_str, bt_status_t status)
{
    if (status == BT_STATUS_SUCCESS) {
        bdt_log("%s[%s:%d]", mp_str, STR_BT_SUCCESS, status);
    } else {
        bdt_log("%s[%s:%d]", mp_str, STR_BT_FAILED, status);
    }
}

static void adapter_state_changed(bt_state_t state)
{
    const char *bt_try_str;
    bt_status_t bt_status;

    ALOGI("ADAPTER STATE UPDATED : %s", (state == BT_STATE_OFF)?"OFF":"ON");

    if (state == BT_STATE_ON) {
        bt_enabled = 1;
    } else {
        bt_enabled = 0;
    }

    bt_try_str = (bt_try_enable == 1) ? STR_BT_MP_ENABLE: STR_BT_MP_DISABLE;
    bt_status = (bt_try_enable == bt_enabled) ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;

    check_return_status(bt_try_str, bt_status);

}

static void dut_mode_recv(uint8_t evtcode, char *buf)
{
    bdt_log(buf);
}

static bt_callbacks_t bt_callbacks = {
    sizeof(bt_callbacks_t),
    adapter_state_changed,
    NULL, /* thread_evt_cb */
    dut_mode_recv, /*dut_mode_recv_cb */
};

static void bdt_shutdown(void)
{
    ALOGI("shutdown bdroid test app");
    main_done = 1;
}

void bdt_init(bt_hci_if_t hci_if, const char *dev_node)
{
    ALOGI("INIT BT");
    status = sBtInterface->init(&bt_callbacks, hci_if, dev_node);
    //check_return_status(status);
}

void bdt_enable(bt_hci_if_t hci_if, const char *dev_node)
{
    ALOGI("ENABLE BT, hci_if[%d], dev_node[%s]", hci_if, dev_node);

    if (bt_enabled) {
        ALOGI("Bluetooth is already enabled");
        bdt_log("Skip to execute %s[%s]", STR_BT_MP_ENABLE, STR_BT_HAS_ENABLED);
        return;
    }

    /* check the enable result in bt_callbacks */
    bt_try_enable = 1;

    status = sBtInterface->init(&bt_callbacks, hci_if, dev_node);
    status = sBtInterface->enable();
}

void bdt_disable(void)
{
    ALOGI("DISABLE BT");

    if (!bt_enabled) {
        ALOGI("Bluetooth is already disabled");
        bdt_log("Skip to execute %s[%s]", STR_BT_MP_DISABLE, STR_BT_HAS_DISABLED);
        return;
    }

    /* check the disable result in bt_callbacks */
    bt_try_enable = 0;

    status = sBtInterface->disable();
    sBtInterface->cleanup();
}

void bdt_dut_mode_configure(char *p)
{
    int32_t mode = -1;

    ALOGI("BT DUT MODE CONFIGURE");
    if (!bt_enabled) {
        ALOGI("Bluetooth must be enabled for test_mode to work.");
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_DUT_MODE, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_DUT_MODE_CONFIGURE, p);

    check_return_status(STR_BT_MP_DUT_MODE, status);
}

void bdt_get_params(char *p)
{
    if (!bt_enabled) {
        ALOGI("Bluetooth must be enabled for %s", STR_BT_MP_GET_PARAM);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_GET_PARAM, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_GetParam, p);

    check_return_status(STR_BT_MP_GET_PARAM, status);
}

void bdt_set_params(char *p)
{
    if (!bt_enabled) {
        ALOGI("Bluetooth must be enabled for %s", STR_BT_MP_SET_PARAM);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_SET_PARAM, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_SetParam, p);

    check_return_status(STR_BT_MP_SET_PARAM, status);
}

void bdt_set_param1(char *p)
{
    if (!bt_enabled) {
        ALOGI("Bluetooth must be enabled for %s", STR_BT_MP_SET_PARAM1);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_SET_PARAM1, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_SetParam1, p);

    check_return_status(STR_BT_MP_SET_PARAM1, status);
}

void bdt_set_param2(char *p)
{
    if (!bt_enabled) {
        ALOGI("Bluetooth must be enabled for %s", STR_BT_MP_SET_PARAM2);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_SET_PARAM2, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_SetParam2, p);

    check_return_status(STR_BT_MP_SET_PARAM2, status);
}

void bdt_set_config(char *p)
{
    if (!bt_enabled) {
        ALOGI("Bluetooth must be enabled for %s", STR_BT_MP_SET_CONFIG);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_SET_CONFIG, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_SetConfig, p);

    check_return_status(STR_BT_MP_SET_CONFIG, status);
}

void bdt_set_gain_table(char *p)
{
    if (!bt_enabled) {
        ALOGI("Bluetooth must be enabled for %s", STR_BT_MP_SET_GAIN_TABLE);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_SET_GAIN_TABLE, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_SetGainTable, p);

    check_return_status(STR_BT_MP_SET_GAIN_TABLE, status);
}

void bdt_set_dac_table(char *p)
{
    if (!bt_enabled) {
        ALOGI("Bluetooth must be enabled for %s", STR_BT_MP_SET_DAC_TABLE);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_SET_DAC_TABLE, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_SetDacTable, p);

    check_return_status(STR_BT_MP_SET_DAC_TABLE, status);
}

void bdt_exec(char *p)
{
    if (!bt_enabled) {
        ALOGI("Bluetooth must be enabled for %s", STR_BT_MP_EXEC);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_EXEC, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_Exec, p);

    check_return_status(STR_BT_MP_EXEC, status);
}

void bdt_report_tx(char *p)
{
    if (!bt_enabled) {
        ALOGI("Bluetooth must be enabled for %s", STR_BT_MP_REPORT_TX);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_REPORT_TX, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_ReportTx, p);

    check_return_status(STR_BT_MP_REPORT_TX, status);
}

void bdt_report_cont_tx(char *p)
{
    if (!bt_enabled) {
        ALOGI("Bluetooth must be enabled for %s", STR_BT_MP_REPORT_CONT_TX);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_REPORT_CONT_TX, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_ReportContTx, p);

    check_return_status(STR_BT_MP_REPORT_CONT_TX, status);
}

void bdt_report_rx(char *p)
{
    if (!bt_enabled) {
        ALOGI("Bluetooth must be enabled for %s", STR_BT_MP_REPORT_RX);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_REPORT_RX, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_ReportRx, p);

    check_return_status(STR_BT_MP_REPORT_RX, status);
}

void bdt_reg_rw(char *p)
{
    if (!bt_enabled) {
        ALOGI("Bluetooth must be enabled for %s", STR_BT_MP_REG_RW);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_REG_RW, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_RegRW, p);

    check_return_status(STR_BT_MP_REG_RW, status);
}

void bdt_hci(char *p)
{
    if (!bt_enabled) {
        ALOGI("Bluetooth must be enabled for %s", STR_BT_MP_HCI_CMD);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_HCI_CMD, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_HCI_SEND_CMD, p);

    check_return_status(STR_BT_MP_HCI_CMD, status);
}

void bdt_cleanup(void)
{
    ALOGI("CLEANUP");
    sBtInterface->cleanup();
}

/*******************************************************************************
 ** Console commands
 *******************************************************************************/

void do_help(char *p)
{
    int i = 0;
    int max = 0;
    char line[128];
    int pos = 0;

    while (console_cmd_list[i].name != NULL)
    {
        pos = sprintf(line, "%s", (char*)console_cmd_list[i].name);
        bdt_log("%s %s", (char*)line, (char*)console_cmd_list[i].help);
        i++;
    }
}

void do_quit(char *p)
{
    bdt_shutdown();
}

/*******************************************************************
 *
 *  BT TEST  CONSOLE COMMANDS
 *
 *  Parses argument lists and passes to API test function
 *
*/

void do_enable(char *p)
{
    char parse_buf[30];
    char *p_node = NULL;
    bt_hci_if_t hci_if = BT_HCI_IF_NONE;

    /* Parse hci interface & device node first */
    snprintf(parse_buf, sizeof(parse_buf), "%s", p);

    p_node = strchr(parse_buf, ':');
    if (p_node) {
        *p_node++ = '\0';
    }

    if (!strcasecmp(parse_buf, "UART")) {
        hci_if = BT_HCI_IF_UART;
    } else if (!strcasecmp(parse_buf, "USB")) {
        hci_if = BT_HCI_IF_USB;
    }

    bdt_enable(hci_if, p_node);
}

void do_disable(char *p)
{
    bdt_disable();
}

void do_dut_mode_configure(char *p)
{
    bdt_dut_mode_configure(p);
}

void do_GetParam(char *p)
{
    bdt_get_params(p);
}

void do_SetParam(char *p)
{
    bdt_set_params(p);
}

void do_SetParam1(char *p)
{
    bdt_set_param1(p);
}

void do_SetParam2(char *p)
{
    bdt_set_param2(p);
}

void do_SetConfig(char *p)
{
    bdt_set_config(p);
}

void do_SetGainTable(char *p)
{
    bdt_set_gain_table(p);
}

void do_SetDacTable(char *p)
{
    bdt_set_dac_table(p);
}

void do_Exec(char *p)
{
    bdt_exec(p);
}

void do_ReportTx(char *p)
{
    bdt_report_tx(p);
}

void do_ReportContTx(char *p)
{
    bdt_report_cont_tx(p);
}

void do_ReportRx(char *p)
{
    bdt_report_rx(p);
}

void do_RegRW(char *p)
{
    bdt_reg_rw(p);
}

void do_hci(char *p)
{
    bdt_hci(p);
}

void do_cleanup(char *p)
{
    bdt_cleanup();
}

/*******************************************************************
 *
 *  CONSOLE COMMAND TABLE
 *
*/

const t_cmd console_cmd_list[] =
{
    /*
     * INTERNAL
     */
    { "help", do_help, ":: Lists all available console commands", 0 },
    { "quit", do_quit, ":: Abort the MP tool test app", 0},

    /*
     * API CONSOLE COMMANDS
     */
    /* Init and Cleanup shall be called automatically */
    { STR_BT_MP_ENABLE, do_enable, ":: Enable bluetooth", 0 },
    { STR_BT_MP_DISABLE, do_disable, ":: Disable bluetooth", 0 },
    { STR_BT_MP_DUT_MODE, do_dut_mode_configure, ":: DUT mode - 1 to enter,0 to exit", 0 },

    { STR_BT_MP_HCI_CMD, do_hci, ":: Send HCI Commands", 0 },

    { STR_BT_MP_GET_PARAM, do_GetParam, ":: Get all exposed parameters", 0 },
    { STR_BT_MP_SET_PARAM, do_SetParam, ":: Set specific parameters<index,value>", 0 },
    { STR_BT_MP_SET_PARAM1, do_SetParam1, ":: Set series 1 parameters", 0 },
    { STR_BT_MP_SET_PARAM2, do_SetParam2, ":: Set series 2 parameters", 0 },

    { STR_BT_MP_SET_CONFIG, do_SetConfig, ":: Set configurations to the specific file", 0 },

    { STR_BT_MP_SET_GAIN_TABLE, do_SetGainTable, ":: Set Tx GAIN table", 0 },
    { STR_BT_MP_SET_DAC_TABLE, do_SetDacTable, ":: Set Tx DAC table", 0 },

    { STR_BT_MP_EXEC, do_Exec, ":: Execute specific action<action id>", 0 },

    { STR_BT_MP_REPORT_TX, do_ReportTx, ":: Report Pkt Tx<TotalTxBits,TotalTxCounts>", 0 },
    { STR_BT_MP_REPORT_CONT_TX, do_ReportContTx, ":: Report Pkt Continue Tx<TotalTxBits,TotalTxCounts>", 0 },
    { STR_BT_MP_REPORT_RX, do_ReportRx, ":: Report Pkt Rx<RxRssi,TotalRxBits,TotalRxCounts,TotalRxErrorBits>", 0 },

    { STR_BT_MP_REG_RW, do_RegRW, ":: R/W Modem, RF, SYS & BB registers", 0 },

    /* add here */

    /* last entry */
    {NULL, NULL, "", 0},
};

/*
 * Main console command handler
*/
static void process_cmd(char *p, unsigned char is_job)
{
    char cmd[64];
    char *p_saved = p;
    int i = 0;

    /* no cmd to process for single linefeed */
    if (*p == '\0')
        return;

    get_str(&p, cmd);

    /* table commands */
    while (console_cmd_list[i].name != NULL) {
        if (is_cmd(console_cmd_list[i].name)) {

            if (!is_job && console_cmd_list[i].is_job) {
                create_cmdjob(p_saved);
            } else {
                console_cmd_list[i].handler(p);
            }

            return;
        }

        i++;
    }

    bdt_log("Unknown command[%s]", p_saved);

    do_help(NULL);
}

int main(int argc, char *argv[])
{
    int opt;
    char cmd[128];
    int args_processed = 0;
    int pid = -1;


    config_permissions();
    bdt_log("\n:::::::::::::::::::::::::::::::::::::::::::::::::");
    bdt_log(":::::::: Bluetooth MP Test Tool Starting ::::::::");

    if (HAL_load() < 0) {
        perror("HAL failed to initialize, exit\n");
        unlink(PID_FILE);
        exit(0);
    }

    setup_test_env();

    /* Automatically perform the init */
//    bdt_init();

    while (!main_done) {
        char line[128];

        /* command prompt */
        printf("> ");
        fflush(stdout);

        fgets(line, 128, stdin);

        if (line[0] != '\0') {
            /* remove linefeed */
            line[strlen(line)-1] = 0;

            process_cmd(line, 0);
            memset(line, '\0', 128);
        }
    }

    /* FIXME: Commenting this out as for some reason, the application does not exit otherwise*/
    //bdt_cleanup();

    HAL_unload();

    bdt_log(":::::::: Bluetooth MP Test Tool Terminating ::::::::");

    return 0;
}
