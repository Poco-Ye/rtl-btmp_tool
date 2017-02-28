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

#ifndef BTMP_IF_H
#define BTMP_IF_H


#include "user_config.h"


#define MP_TOOL_VERSION     "ver 16.10.21"

typedef enum {
    LOG_STD = 0, /* log printed to stdout */
    LOG_SKT      /* log printed to socket */
} LOG_TYPE;

typedef void (t_console_cmd_handler) (char *p);

typedef struct {
    const char *name;
    t_console_cmd_handler *handler;
    const char *help;
} t_cmd;

void btmp_log_std(const char *fmt_str, ...);
void btmp_log_skt(const char *fmt_str, ...);
void btmp_log(const char *fmt_str, ...);

void config_permissions(void);

void btmp_help(char *p);

void btmp_quit(char *p);

void btmp_enable(char *p);

void btmp_disable(char *p);

void btmp_get_param(char *p);

void btmp_set_param(char *p);

void btmp_set_config(char *p);

void btmp_exec(char *p);

void btmp_report(char *p);

void btmp_reg_RW(char *p);

#if (MP_TOOL_COMMAND_SEARCH_EXIST_PERMISSION == 1)
void btmp_search(char *p);
#endif

#if (MP_TOOL_COMMAND_READ_PERMISSION == 1)
void btmp_read(char *p);
#endif

void btmp_hci_cmd(char *p);

void btmp_cleanup(char *p);

void process_cmd(char *p);

int HAL_load(LOG_TYPE type, int fd);

void HAL_unload(void);

#endif /* BTMP_IF_H */
