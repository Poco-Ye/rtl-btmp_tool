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

#define LOG_TAG "btmp_socket"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include <bluetoothmp.h>
#include <bt_syslog.h>
#include <btmp_if.h>

static unsigned char main_done = 0;

static int init_evt_sockpair(int *evt_fds)
{
    int ret;

    ret = socketpair(AF_UNIX, SOCK_STREAM, 0, evt_fds);
    if (ret == -1) {
        SYSLOGE("failed to create socket pair: %s(%d)", strerror(errno), errno);
        return -1;
    }

    return 0;
}

static void close_evt_sockpair(int *evt_fds)
{
    close(evt_fds[0]);
    close(evt_fds[1]);
}

int main(int argc, char *argv[])
{
    char cmdline[128], evtline[128];
    int sk_fd, net_fd, evt_fds[2];
    struct sockaddr_in serv_addr;
    int sock_opt = 1;
    fd_set input, output;
    int fd_max, fd_num;
    int output_ready = 0;
    int ret;

    btmp_log_std(":::::::::::::::::::::::::::::::::::::::::::::::::");
    btmp_log_std(":::::::: Bluetooth MP Test Tool Starting 20180829 ::::::::");

    config_permissions();

    ret = init_evt_sockpair(evt_fds);
    if (ret < 0) {
        SYSLOGE("failed to init evt socket pair, exit");
        return -1;
    }

    if (HAL_load(LOG_SKT, evt_fds[1]) < 0) {
        SYSLOGE("HAL failed to initialize, exit");
        return -1;
    }

    sk_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sk_fd == -1) {
        SYSLOGE("failed to create socket: %s(%d)", strerror(errno), errno);
        return -1;
    }

    memset(&serv_addr, 0x00, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(6666);

    ret = setsockopt(sk_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&sock_opt, sizeof(sock_opt));
    if (ret == -1) {
        SYSLOGE("failed to set socket opt: %s(%d)", strerror(errno), errno);
        return -1;
    }

    ret = bind(sk_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (ret == -1) {
        SYSLOGE("failed to bind socket: %s(%d)", strerror(errno), errno);
        return -1;
    }

    ret = listen(sk_fd, 3);
    if (ret == -1) {
        SYSLOGE("failed to listen socket: %s(%d)", strerror(errno), errno);
        return -1;
    }

re_accept:
    net_fd = accept(sk_fd, (struct sockaddr*)NULL, NULL);
    if (net_fd == -1) {
        SYSLOGE("failed to accept socket: %s(%d)", strerror(errno), errno);
        if (errno == EINTR)
            goto re_accept;
        else
            return -1;
    }

    fd_max = net_fd > evt_fds[0] ? net_fd : evt_fds[0];
    output_ready = 0;

    while (!main_done) {
        /* socket prompt */
        FD_ZERO(&input);
        FD_ZERO(&output);
        FD_SET(net_fd, &input);
        FD_SET(evt_fds[0], &input);
        if (!output_ready)
            FD_SET(net_fd, &output);

        fd_num = select(fd_max + 1, &input, &output, NULL, NULL);
        if (fd_num > 0) {
            /* check if client is ready to receive */
            if (FD_ISSET(net_fd, &output))
                output_ready = 1;

            if (FD_ISSET(net_fd, &input)) {
                memset(cmdline, '\0', 128);
                ret = read(net_fd, cmdline, 128);
                SYSLOGW("ret: %d", ret);
                if (ret > 0)
                    process_cmd(cmdline);
                else if (ret == 0) {
                    SYSLOGW("failed to read: %s(%d)", strerror(errno), errno);
                    close(net_fd);
                    goto re_accept;
                } else if (ret == -1 && errno == EINTR)
                    continue;
                else if (ret == -1 && errno == ECONNRESET){
                    SYSLOGW("failed to read: %s(%d)", strerror(errno), errno);
                    close(net_fd);
                    goto re_accept;
                }
                else {
                    SYSLOGE("failed to read: %s(%d)", strerror(errno), errno);
                    return -1;
                }
            }

            if (FD_ISSET(evt_fds[0], &input)) {
                memset(evtline, '\0', 128);
                ret = read(evt_fds[0], evtline, 128);
                if (ret > 0 && output_ready) {
                    ret = write(net_fd, evtline, ret);
                    if (ret > 0)
                        output_ready = 0;
                    else if (ret == 0) {
                        SYSLOGW("failed to write: %s(%d)", strerror(errno), errno);
                        goto re_accept;
                    }  else if (ret == -1 && errno == EINTR)
                        continue;
                    else {
                        SYSLOGE("failed to write: %s(%d)", strerror(errno), errno);
                        return -1;
                    }
                }
            }
        }
    }

    close_evt_sockpair(evt_fds);
    close(net_fd);

    HAL_unload();

    btmp_log_std(":::::::: Bluetooth MP Test Tool Terminating ::::::::");

    return 0;
}
