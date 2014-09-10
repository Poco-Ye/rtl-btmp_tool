/******************************************************************************
 *
 *  Copyright (C) 2014 Broadcom Corporation
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
 *  Filename:      bt_syslog.c
 *
 *  Description:   sys log helper functions in Linux
 *
 *
 ***********************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <syslog.h>
#include <stdarg.h>
#include <inttypes.h>

/** trace level for sys log */
#define SYS_TRACE_DEBUG     0
#define SYS_TRACE_INFO      1
#define SYS_TRACE_WARN      2
#define SYS_TRACE_ERROR     3

#define SYS_LOG_BUF_SIZE    1024
#define SYS_LOG_MSG_OFFSET  0
#define SYS_LOG_MSG_MAX (SYS_LOG_BUF_SIZE - SYS_LOG_MSG_OFFSET)

static void sys_log_msg(uint8_t trace_level, const char *fmt_str, va_list ap)
{
    static char buffer[SYS_LOG_BUF_SIZE];
    int priority = LOG_USER;

    vsnprintf(&buffer[SYS_LOG_MSG_OFFSET], SYS_LOG_MSG_MAX, fmt_str, ap);

    switch (trace_level) {
    case SYS_TRACE_DEBUG:
        priority |= LOG_DEBUG;
        break;
    case SYS_TRACE_INFO:
        priority |= LOG_INFO;
        break;
    case SYS_TRACE_WARN:
        priority |= LOG_WARNING;
        break;
    case SYS_TRACE_ERROR:
    default:
        priority |= LOG_ERR;
        break;
    }

    syslog(priority, "%s", buffer);
}

void SYSLOGD(const char *fmt_str, ...)
{
    va_list ap;

    va_start(ap, fmt_str);
    sys_log_msg(SYS_TRACE_DEBUG, fmt_str, ap);
    va_end(ap);
}

void SYSLOGI(const char *fmt_str, ...)
{
    va_list ap;

    va_start(ap, fmt_str);
    sys_log_msg(SYS_TRACE_INFO, fmt_str, ap);
    va_end(ap);
}

void SYSLOGW(const char *fmt_str, ...)
{
    va_list ap;

    va_start(ap, fmt_str);
    sys_log_msg(SYS_TRACE_WARN, fmt_str, ap);
    va_end(ap);
}

void SYSLOGE(const char *fmt_str, ...)
{
    va_list ap;

    va_start(ap, fmt_str);
    sys_log_msg(SYS_TRACE_ERROR, fmt_str, ap);
    va_end(ap);
}
