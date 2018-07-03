#include <stdio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

static void help(void)
{
    printf("mp_client usage: ./mp_client IP\n");
}

int main (int argc, char **argv)
{
    char serv_ip[32];
    int sk_fd;
    struct sockaddr_in serv_addr;
    fd_set rfds, wfds;
    char cmdline[128];
    char rbuf[128];
    int fd_num;
    int wait_read = 0;
    int ret;

    /* check argc */
    if (argc != 2) {
        help();
        return -1;
    }

    snprintf(serv_ip, sizeof(serv_ip), "%s", argv[1]);

reconnect:
    /* TCP client */
    sk_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sk_fd < 0) {
        printf("failed to create socket: %s(%d)\n", strerror(errno), errno);
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_port = htons(6666);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(serv_ip);

    if (connect(sk_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))  < 0) {
        printf("failed to connect socket: %s(%d)\n", strerror(errno), errno);
        return -1;
    }

    printf("\nMP client test tool\n");

    while (1) {
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_SET(sk_fd, &rfds);
        FD_SET(sk_fd, &wfds);

        fd_num = select(sk_fd + 1, &rfds, &wfds, NULL, NULL);
        if (fd_num > 0) {
            /* read fd has higher priority */
            if (FD_ISSET(sk_fd, &rfds)) {
                memset(rbuf, '\0', 128);
                ret = read(sk_fd, rbuf, sizeof(rbuf));
                if (ret > 0) {
                    wait_read = 0;
                    printf("< %s\n", rbuf);
                }
            }

            if (wait_read == 1) {
                //printf("DEBUG: check wait_read to be 1\n");
                continue;
            }

            memset(cmdline, '\0', 128);

            /* cmd prompt */
            printf("> ");
            fflush(stdout);

            fgets(cmdline, 128, stdin);

            if (cmdline[0] == '\0' || cmdline[0] == '\n')
                continue;
            else if (!strncmp(cmdline, "close", 5)) {
                printf("DEBUG: close sk_fd %d\n", sk_fd);
                //shutdown(sk_fd, SHUT_RDWR);
                close(sk_fd);
                goto reconnect;
            } else if (cmdline[strlen(cmdline)-1] == '\n')
                cmdline[strlen(cmdline)-1] = '\0'; /* remove linefeed */

            if (FD_ISSET(sk_fd, &wfds)) {
                ret = write(sk_fd, cmdline, strlen(cmdline));
                if (ret == strlen(cmdline)) {
                    //printf("DEBUG: set wait_read to 1\n");
                    wait_read = 1; /* any cmd should have a response */
                }
            }
        }
    }

    return 0;
}
