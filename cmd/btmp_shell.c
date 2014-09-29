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
 *  Filename:      btmp_shell.c
 *
 *  Description:   Bluetooth MP Shell Test application
 *
 ***********************************************************************************/
#define LOG_TAG "rtlmp_test"

#include <stdio.h>
#include <stdarg.h>
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

#include <bluetoothmp.h>
#include <bt_syslog.h>

/************************************************************************************
**  Constants & Macros
************************************************************************************/

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
static bluetooth_device_t *bt_device;

const bt_interface_t *sBtInterface = NULL;

//static gid_t groups[] = { AID_NET_BT, AID_INET, AID_NET_BT_ADMIN,
//                          AID_SYSTEM, AID_MISC, AID_SDCARD_RW,
//                          AID_NET_ADMIN, AID_VPN};

/* Set to 1 when the Bluedroid stack is enabled */
static unsigned char bt_enabled = 0;

/************************************************************************************
**  Static functions
************************************************************************************/

static void process_cmd(char *p, unsigned char is_job);

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

/*******************************************************************************
 ** Console helper functions
 *******************************************************************************/

void skip_blanks(char **p)
{
    while (**p == ' ')
        (*p)++;
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

static void cmdjob_handler(void *param)
{
    char *job_cmd = (char*)param;

    SYSLOGI("cmdjob starting (%s)", job_cmd);

    process_cmd(job_cmd, 1);

    SYSLOGI("cmdjob terminating");

    free(job_cmd);
}

static int create_cmdjob(char *cmd)
{
    pthread_t thread_id;
    char *job_cmd;

    job_cmd = malloc(strlen(cmd) + 1); /* freed in job handler */
    strcpy(job_cmd, cmd);

    if (pthread_create(&thread_id, NULL,
                       (void*)cmdjob_handler, (void*)job_cmd) != 0)
      perror("pthread_create");

    return 0;
}

/*******************************************************************************
 ** Load stack lib
 *******************************************************************************/

static int HAL_load(void)
{
    int err = 0;

    SYSLOGI("Loading HAL lib + extensions");

    err = open_bluetooth_stack(BT_HARDWARE_MODULE_ID, &bt_device);
    if (err == 0) {
        sBtInterface = bt_device->get_bluetooth_interface();
    }

    SYSLOGI("HAL library loaded (%s)", strerror(err));

    return err;
}

static void HAL_unload(void)
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

/*******************************************************************************
 ** HAL test functions & callbacks
 *******************************************************************************/

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
    SYSLOGI("shutdown bluetooth MP test tool");
    main_done = 1;
}

void bdt_enable(bt_hci_if_t hci_if, const char *dev_node)
{
    SYSLOGI("ENABLE BT, hci_if[%d], dev_node[%s]", hci_if, dev_node);

    if (bt_enabled) {
        SYSLOGI("Bluetooth is already enabled");
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
    SYSLOGI("DISABLE BT");

    if (!bt_enabled) {
        SYSLOGI("Bluetooth is already disabled");
        bdt_log("Skip to execute %s[%s]", STR_BT_MP_DISABLE, STR_BT_HAS_DISABLED);
        return;
    }

    /* check the disable result in bt_callbacks */
    bt_try_enable = 0;

    status = sBtInterface->disable();
    sBtInterface->cleanup();
}

void bdt_get_param(char *p)
{
    if (!bt_enabled) {
        SYSLOGI("Bluetooth must be enabled for %s", STR_BT_MP_GET_PARAM);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_GET_PARAM, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_GetParam, p);

    check_return_status(STR_BT_MP_GET_PARAM, status);
}

void bdt_set_param(char *p)
{
    if (!bt_enabled) {
        SYSLOGI("Bluetooth must be enabled for %s", STR_BT_MP_SET_PARAM);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_SET_PARAM, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_SetParam, p);

    check_return_status(STR_BT_MP_SET_PARAM, status);
}

void bdt_set_config(char *p)
{
    if (!bt_enabled) {
        SYSLOGI("Bluetooth must be enabled for %s", STR_BT_MP_SET_CONFIG);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_SET_CONFIG, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_SetConfig, p);

    check_return_status(STR_BT_MP_SET_CONFIG, status);
}

void bdt_exec(char *p)
{
    if (!bt_enabled) {
        SYSLOGI("Bluetooth must be enabled for %s", STR_BT_MP_EXEC);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_EXEC, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_Exec, p);

    check_return_status(STR_BT_MP_EXEC, status);
}

void bdt_report(char *p)
{
    if (!bt_enabled) {
        SYSLOGI("Bluetooth must be enabled for %s", STR_BT_MP_REPORT);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_REPORT, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_Report, p);

    check_return_status(STR_BT_MP_REPORT, status);
}

void bdt_reg_rw(char *p)
{
    if (!bt_enabled) {
        SYSLOGI("Bluetooth must be enabled for %s", STR_BT_MP_REG_RW);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_REG_RW, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_USER_DEF_RegRW, p);

    check_return_status(STR_BT_MP_REG_RW, status);
}

void bdt_hci_cmd(char *p)
{
    if (!bt_enabled) {
        SYSLOGI("Bluetooth must be enabled for %s", STR_BT_MP_HCI_CMD);
        bdt_log("Failed to execute %s[%s]", STR_BT_MP_HCI_CMD, STR_BT_NOT_ENABLED);
        return;
    }

    status = sBtInterface->hal_mp_op_send(BT_MP_OP_HCI_SEND_CMD, p);

    check_return_status(STR_BT_MP_HCI_CMD, status);
}

void bdt_cleanup(void)
{
    SYSLOGI("CLEANUP");
    sBtInterface->cleanup();
}

/*******************************************************************************
 ** Console commands
 *******************************************************************************/

void do_help(char *p)
{
    int i = 0;
    char line[128];

    while (console_cmd_list[i].name != NULL) {
        sprintf(line, "%s", (char*)console_cmd_list[i].name);
        bdt_log("%s %s", (char*)line, (char*)console_cmd_list[i].help);
        i++;
    }
}

void do_quit(char *p)
{
    bdt_shutdown();
}

/**
 *  BT TEST  CONSOLE COMMANDS
 *
 *  Parses argument lists and passes to API test function
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

void do_get_param(char *p)
{
    bdt_get_param(p);
}

void do_set_param(char *p)
{
    bdt_set_param(p);
}

void do_set_config(char *p)
{
    bdt_set_config(p);
}

void do_exec(char *p)
{
    bdt_exec(p);
}

void do_report(char *p)
{
    bdt_report(p);
}

void do_reg_RW(char *p)
{
    bdt_reg_rw(p);
}

void do_hci_cmd(char *p)
{
    bdt_hci_cmd(p);
}

void do_cleanup(char *p)
{
    bdt_cleanup();
}

/**
 *  CONSOLE COMMAND TABLE
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

    { STR_BT_MP_HCI_CMD, do_hci_cmd, ":: Send HCI Commands", 0 },

    { STR_BT_MP_GET_PARAM, do_get_param, ":: Get all/individual exposed parameters", 0 },
    { STR_BT_MP_SET_PARAM, do_set_param, ":: Set specific parameters<index,value>", 0 },

    { STR_BT_MP_SET_CONFIG, do_set_config, ":: Set configurations to the specific file", 0 },

    { STR_BT_MP_EXEC, do_exec, ":: Execute specific action<action id>", 0 },

    { STR_BT_MP_REPORT, do_report, ":: Report specific info according to item selected", 0 },

    { STR_BT_MP_REG_RW, do_reg_RW, ":: R/W Modem, RF, SYS & BB registers", 0 },

    /* add here */

    /* last entry */
    {NULL, NULL, "", 0},
};

/**
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
    char cmdline[128];

    config_permissions();
    bdt_log("\n:::::::::::::::::::::::::::::::::::::::::::::::::");
    bdt_log(":::::::: Bluetooth MP Test Tool Starting ::::::::");

    if (HAL_load() < 0) {
        perror("HAL failed to initialize, exit\n");
        exit(0);
    }

    while (!main_done) {
        /* command prompt */
        printf("> ");
        fflush(stdout);

        fgets(cmdline, 128, stdin);

        if (cmdline[0] != '\0') {
            /* remove linefeed */
            cmdline[strlen(cmdline)-1] = 0;

            process_cmd(cmdline, 0);
            memset(cmdline, '\0', 128);
        }
    }

    HAL_unload();

    bdt_log(":::::::: Bluetooth MP Test Tool Terminating ::::::::");

    return 0;
}
