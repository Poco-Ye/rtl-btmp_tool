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

#define LOG_TAG "btmp_shell"

#include <stdio.h>
#include <string.h>

#include <bt_syslog.h>
#include <btmp_if.h>

static unsigned char main_done = 0;

int main(int argc, char *argv[])
{
    char cmdline[128];

    btmp_log_std(":::::::::::::::::::::::::::::::::::::::::::::::::");
    btmp_log_std(":::::::: Bluetooth MP Test Tool Starting ::::::::");

    config_permissions();

    if (HAL_load(LOG_STD, -1) < 0) {
        SYSLOGE("HAL failed to initialize, exit");
        return -1;
    }

    while (!main_done) {
        /* command prompt */
        printf("> ");
        fflush(stdout);

        fgets(cmdline, 128, stdin);

        if (cmdline[0] != '\0') {
            process_cmd(cmdline);
            memset(cmdline, '\0', 128);
        }
    }

    HAL_unload();

    btmp_log_std(":::::::: Bluetooth MP Test Tool Terminating ::::::::");

    return 0;
}
