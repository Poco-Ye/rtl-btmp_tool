/******************************************************************************
 *
 *  Copyright (C) 2014 Realsil Corporation
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
 *  This is the interface file for stack log in Linux
 *
 ******************************************************************************/

#ifndef BT_SYSLOG_H
#define BT_SYSLOG_H

#include <syslog.h>

#ifndef LOG_TAG
#define LOG_TAG NULL
#endif

void SYSLOGD(const char *format, ...);
void SYSLOGI(const char *format, ...);
void SYSLOGW(const char *format, ...);
void SYSLOGE(const char *format, ...);

#endif  /* BT_SYSLOG_H */
