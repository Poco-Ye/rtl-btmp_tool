/******************************************************************************
 *
 *  Copyright (C) 2014 Realsil Corporation.
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

#define LOG_TAG "btmp_if"

#include <stdio.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <linux/capability.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include <bluetoothmp.h>
#include <btmp_if.h>
#include <bt_syslog.h>

static bt_status_t status;
static int bt_try_enable = 0; /* 0: try to disable; 1: try to enable */
static char log_buf[1024];
static LOG_TYPE log_type;
static int log_fd = -1;

/* Main API */
static bluetooth_device_t *bt_device;

const bt_interface_t *sBtInterface = NULL;

//static gid_t groups[] = { AID_NET_BT, AID_INET, AID_NET_BT_ADMIN,
//                          AID_SYSTEM, AID_MISC, AID_SDCARD_RW,
//                          AID_NET_ADMIN, AID_VPN };

/* Set to 1 when the Bluetooth stack is enabled */
static unsigned char bt_enabled = 0;

/**
 *  CONSOLE COMMAND TABLE
 */
const t_cmd console_cmd_list[] =
{
    /*
     * API CONSOLE COMMANDS
     */
    { "help", btmp_help, ":: Lists all available console commands" },
    { "quit", btmp_quit, ":: Abort the MP tool test app" },

    { STR_BT_MP_ENABLE, btmp_enable, ":: Enable bluetooth" },
    { STR_BT_MP_DISABLE, btmp_disable, ":: Disable bluetooth" },

    { STR_BT_MP_HCI_CMD, btmp_hci_cmd, ":: Send HCI Commands" },

    { STR_BT_MP_GET_PARAM, btmp_get_param, ":: Get all/individual exposed parameters" },
    { STR_BT_MP_SET_PARAM, btmp_set_param, ":: Set specific parameters<index,value>" },

    { STR_BT_MP_SET_CONFIG, btmp_set_config, ":: Set configurations to the specific file" },

    { STR_BT_MP_EXEC, btmp_exec, ":: Execute specific action<action id>" },

    { STR_BT_MP_REPORT, btmp_report, ":: Report specific info according to item selected" },

    { STR_BT_MP_REG_RW, btmp_reg_RW, ":: R/W Modem, RF, SYS & BB registers" },

    /* add here */

    /* last entry */
    { NULL, NULL, "" },
};

static void skip_blanks(char **p)
{
    while (**p == ' ')
        (*p)++;
}

static void get_cmd_str(char **p, char *buffer)
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

/*******************************************************************************
 ** HAL test functions & callbacks
 *******************************************************************************/
void check_return_status(const char *mp_str, bt_status_t status)
{
    if (status == BT_STATUS_SUCCESS) {
        btmp_log("%s[%s:%d]", mp_str, STR_BT_SUCCESS, status);
    } else {
        btmp_log("%s[%s:%d]", mp_str, STR_BT_FAILED, status);
    }
}

static void adapter_state_changed(bt_state_t state)
{
    const char *bt_try_str;
    bt_status_t bt_status;

    SYSLOGI("ADAPTER STATE UPDATED : %s", (state == BT_STATE_OFF) ? "OFF" : "ON");

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
    btmp_log(buf);
}

static bt_callbacks_t bt_callbacks = {
    sizeof(bt_callbacks_t),
    adapter_state_changed,
    NULL, /* thread_evt_cb */
    dut_mode_recv, /*dut_mode_recv_cb */
};

/*******************************************************************************
 ** Console commands
 *******************************************************************************/
void btmp_log_std(const char *fmt_str, ...)
{
    va_list ap;

    va_start(ap, fmt_str);
    vsnprintf(log_buf, 1024, fmt_str, ap);
    va_end(ap);

    fprintf(stdout, "%s\n", log_buf);
}

void btmp_log_skt(const char *fmt_str, ...)
{
    va_list ap;

    va_start(ap, fmt_str);
    vsnprintf(log_buf, 1024, fmt_str, ap);
    va_end(ap);

    write(log_fd, log_buf, strlen(log_buf));
}

void btmp_log(const char *fmt_str, ...)
{
    va_list ap;

    va_start(ap, fmt_str);
    vsnprintf(log_buf, 1024, fmt_str, ap);
    va_end(ap);

    if (log_type == LOG_STD)
        fprintf(stdout, "%s\n", log_buf);
    else if (log_type == LOG_SKT)
        write(log_fd, log_buf, strlen(log_buf));
}

void config_permissions(void)
{
    //struct __user_cap_header_struct header;
    //struct __user_cap_data_struct cap;
    //int ret;

    SYSLOGI("pre set_aid: pid %d, ppid %d, uid %d, euid %d, gid %d, egid %d",
            getpid(), getppid(), getuid(), geteuid(), getgid(), getegid());
#if 0
    prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0);

    ret = setgid(AID_NET_BT_STACK);
    if (ret < 0) {
        SYSLOGI("setgid AID_NET_BT_STACK failed: %s(%d)", strerror(errno), errno);
    }

    ret = setuid(AID_BLUETOOTH);
    if (ret < 0) {
        SYSLOGI("setuid AID_BLUETOOTH failed: %s(%d)", strerror(errno), errno);
    }

    SYSLOGI("post set_aid: pid %d, ppid %d, uid %d, euid %d, gid %d, egid %d",
            getpid(), getppid(), getuid(), geteuid(), getgid(), getegid());
#endif
#if 0
    header.pid = 0;
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
#endif
}

void btmp_help(char *p)
{
    int i = 0;
    char line[128];

    while (console_cmd_list[i].name != NULL) {
        sprintf(line, "%s", (char *)console_cmd_list[i].name);
        btmp_log("%s %s", (char *)line, (char *)console_cmd_list[i].help);
        i++;
    }
}

void btmp_quit(char *p)
{
    SYSLOGI("shutdown bluetooth MP test tool");
    /* exit from the main loop */
    exit(0);
}

void btmp_enable(char *p)
{
    char parse_buf[30];
    char *p_node = NULL;
    bt_hci_if_t hci_if = BT_HCI_IF_NONE;

    if (bt_enabled) {
        SYSLOGI("Bluetooth is already enabled");
        btmp_log("Skip to execute %s[%s]", STR_BT_MP_ENABLE, STR_BT_HAS_ENABLED);
        return;
    }

    /* check the enable result in bt_callbacks */
    bt_try_enable = 1;

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

    SYSLOGI("ENABLE BT, hci_if[%d], dev_node[%s]", hci_if, p_node);

    status = sBtInterface->init(&bt_callbacks, hci_if, p_node);
    status = sBtInterface->enable();
}

void btmp_disable(char *p)
{
    SYSLOGI("DISABLE BT");

    if (!bt_enabled) {
        SYSLOGI("Bluetooth is already disabled");
        btmp_log("Skip to execute %s[%s]", STR_BT_MP_DISABLE, STR_BT_HAS_DISABLED);
        return;
    }

    /* check the disable result in bt_callbacks */
    bt_try_enable = 0;

    status = sBtInterface->disable();
    sBtInterface->cleanup();
}

void btmp_get_param(char *p)
{
    if (!bt_enabled) {
        SYSLOGI("Bluetooth must be enabled for %s", STR_BT_MP_GET_PARAM);
        btmp_log("Failed to execute %s[%s]", STR_BT_MP_GET_PARAM, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_GetParam, p);

    check_return_status(STR_BT_MP_GET_PARAM, status);
}

void btmp_set_param(char *p)
{
    if (!bt_enabled) {
        SYSLOGI("Bluetooth must be enabled for %s", STR_BT_MP_SET_PARAM);
        btmp_log("Failed to execute %s[%s]", STR_BT_MP_SET_PARAM, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_SetParam, p);

    check_return_status(STR_BT_MP_SET_PARAM, status);
}

void btmp_set_config(char *p)
{
    if (!bt_enabled) {
        SYSLOGI("Bluetooth must be enabled for %s", STR_BT_MP_SET_CONFIG);
        btmp_log("Failed to execute %s[%s]", STR_BT_MP_SET_CONFIG, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_SetConfig, p);

    check_return_status(STR_BT_MP_SET_CONFIG, status);
}

void btmp_exec(char *p)
{
    if (!bt_enabled) {
        SYSLOGI("Bluetooth must be enabled for %s", STR_BT_MP_EXEC);
        btmp_log("Failed to execute %s[%s]", STR_BT_MP_EXEC, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_Exec, p);

    check_return_status(STR_BT_MP_EXEC, status);
}

void btmp_report(char *p)
{
    if (!bt_enabled) {
        SYSLOGI("Bluetooth must be enabled for %s", STR_BT_MP_REPORT);
        btmp_log("Failed to execute %s[%s]", STR_BT_MP_REPORT, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_Report, p);

    check_return_status(STR_BT_MP_REPORT, status);
}

void btmp_reg_RW(char *p)
{
    if (!bt_enabled) {
        SYSLOGI("Bluetooth must be enabled for %s", STR_BT_MP_REG_RW);
        btmp_log("Failed to execute %s[%s]", STR_BT_MP_REG_RW, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_RegRW, p);

    check_return_status(STR_BT_MP_REG_RW, status);
}

void btmp_hci_cmd(char *p)
{
    if (!bt_enabled) {
        SYSLOGI("Bluetooth must be enabled for %s", STR_BT_MP_HCI_CMD);
        btmp_log("Failed to execute %s[%s]", STR_BT_MP_HCI_CMD, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_HCI_SEND_CMD, p);

    check_return_status(STR_BT_MP_HCI_CMD, status);
}

void btmp_cleanup(char *p)
{
    SYSLOGI("CLEANUP");
    sBtInterface->cleanup();
}

#define is_cmd(str) ((strlen(str) == strlen(cmd)) && strncmp((const char *)&cmd, str, strlen(str)) == 0)

/**
 * Main console command handler
 */
void process_cmd(char *p)
{
    char cmd[64];
    char *p_saved = p;
    int i = 0;

    /* no cmd to process for single linefeed */
    if (*p == '\0' || *p == '\n')
        return;

    /* remove trailing linefeed */
    if (p[strlen(p)-1] == '\n')
        p[strlen(p)-1] = '\0';

    get_cmd_str(&p, cmd);

    /* table commands */
    while (console_cmd_list[i].name != NULL) {
        if (is_cmd(console_cmd_list[i].name)) {
            console_cmd_list[i].handler(p);
            return;
        }
        i++;
    }

    btmp_log("Unknown command[%s]", p_saved);

    btmp_help(NULL);
}

int HAL_load(LOG_TYPE type, int fd)
{
    int err = 0;

    SYSLOGI("Loading HAL lib + extensions");

    err = open_bluetooth_stack(BT_HARDWARE_MODULE_ID, &bt_device);
    if (err == 0) {
        sBtInterface = bt_device->get_bluetooth_interface();
        log_type = type;
        log_fd = fd;
    }

    SYSLOGI("HAL library loaded (%s)", strerror(err));

    return err;
}

void HAL_unload(void)
{
    SYSLOGI("Unloading HAL lib");

    /* prevent abnormal exit */
    if (bt_enabled) {
        bt_try_enable = 0;

        sBtInterface->disable();
        sBtInterface->cleanup();
    }

    sBtInterface = NULL;

    SYSLOGI("HAL library unloaded");
}
