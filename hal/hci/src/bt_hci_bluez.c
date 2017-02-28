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
 *  Filename:      bt_hci_bluez.c
 *
 *  Description:   Contains bluez hci interfaces
 *
 ******************************************************************************/

#define LOG_TAG "BT_HCI_BLUEZ"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "bt_syslog.h"
#include "bt_hci_bluez.h"
#include "bt_types.h"

static int bachk(const char *str)
{
    if (!str)
        return -1;

    if (strlen(str) != 17)
        return -1;

    while (*str) {
        if (!isxdigit(*str++))
            return -1;

        if (!isxdigit(*str++))
            return -1;

        if (*str == 0)
            break;

        if (*str++ != ':')
            return -1;
    }

    return 0;
}

static void baswap(bdaddr_t *dst, const bdaddr_t *src)
{
    register unsigned char *d = (unsigned char *) dst;
    register const unsigned char *s = (const unsigned char *) src;
    register int i;

    for (i = 0; i < 6; i++)
        d[i] = s[5-i];
}

static int str2ba(const char *str, bdaddr_t *ba)
{
    bdaddr_t b;
    int i;

    if (bachk(str) < 0) {
        memset(ba, 0, sizeof(*ba));
        return -1;
    }

    for (i = 0; i < 6; i++, str += 3)
        b.b[i] = strtol(str, NULL, 16);

    baswap(ba, &b);

    return 0;
}

static int other_bdaddr(int dd, int dev_id, long arg)
{
    struct hci_dev_info di = { .dev_id = dev_id };

    if (ioctl(dd, HCIGETDEVINFO, (void *) &di))
        return 0;

    if (hci_test_bit(HCI_RAW, &di.flags))
        return 0;

    return bacmp((bdaddr_t *) arg, &di.bdaddr);
}

static int same_bdaddr(int dd, int dev_id, long arg)
{
    struct hci_dev_info di = { .dev_id = dev_id };

    if (ioctl(dd, HCIGETDEVINFO, (void *) &di))
        return 0;

    return !bacmp((bdaddr_t *) arg, &di.bdaddr);
}

void activate_bluetooth(int dev_id)
{
    char cmd[64];
    FILE *fp;

    memset(cmd, 0, sizeof(cmd));
    
    sprintf(cmd, "hciconfig hci%d up", dev_id);

    fp = popen(cmd, "r");
    pclose(fp);
}

/* HCI functions that do not require open device */
static int hci_for_each_dev(int flag, int (*func)(int dd, int dev_id, long arg),
        long arg)
{
    struct hci_dev_list_req *dl;
    struct hci_dev_req *dr;
    int dev_id = -1;
    int i, sk, err = 0;

    sk = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
    if (sk < 0)
        return -1;

    dl = malloc(HCI_MAX_DEV * sizeof(*dr) + sizeof(*dl));
    if (!dl) {
        err = errno;
        goto done;
    }

    memset(dl, 0, HCI_MAX_DEV * sizeof(*dr) + sizeof(*dl));

    dl->dev_num = HCI_MAX_DEV;
    dr = dl->dev_req;

    if (ioctl(sk, HCIGETDEVLIST, (void *) dl) < 0) {
        err = errno;
        goto free;
    }
    for (i = 0; i < dl->dev_num; i++, dr++) {
        if (hci_test_bit(flag, &dr->dev_opt))
            if (!func || func(sk, dr->dev_id, arg)) {
                dev_id = dr->dev_id;
                break;
            }
    }

    if (dev_id < 0) {
        err = ENODEV;
    } else {
        activate_bluetooth(dev_id);
    }

free:
    free(dl);

done:
    close(sk);
    errno = err;

    return dev_id;
}


int hci_devid(const char *str)
{
    bdaddr_t ba;
    int id = -1;

    if (str) {
        str2ba(str, &ba);
        id = hci_for_each_dev(HCI_UP, same_bdaddr, (long)&ba);
    } else {
        id = hci_for_each_dev(HCI_UP, other_bdaddr, (long)BDADDR_ANY);
    }

    return id;
}

/* Open HCI device.
 * Returns device descriptor (dd). */
int hci_open_dev(int dev_id)
{
    struct sockaddr_hci a;
    struct hci_filter flt;
    int dd;

    /* Create HCI socket */
    dd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
    if (dd < 0)
        return dd;

    /* Bind socket to the HCI device */
    memset(&a, 0, sizeof(a));
    a.hci_family = AF_BLUETOOTH;
    a.hci_dev = dev_id;
    if (bind(dd, (struct sockaddr *) &a, sizeof(a)) < 0) {
        SYSLOGE("HCI bind failed");
        goto failed;
    }

    /* Setup filter */
    hci_filter_clear(&flt);
    hci_filter_set_ptype(HCI_EVENT_PKT, &flt);
    hci_filter_all_events(&flt);
    if (setsockopt(dd, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0) {
        SYSLOGE("HCI filter setup failed");
        goto failed;
    }

    return dd;

failed:
    close(dd);
    return -1;
}

int hci_close_dev(int dd)
{
    return close(dd);
}

int hci_bluez_read(int dd, void *pbuf, size_t plen)
{
    int ret;

    while ((ret = read(dd, pbuf, plen)) < 0) {
        if (errno == EAGAIN || errno == EINTR)
            continue;
        break;
    }

    return ret;
}

int hci_bluez_write(int dd, void *pbuf, uint16_t plen)
{
    int ret;

    while ((ret = write(dd, pbuf, plen)) < 0) {
        if (errno == EAGAIN || errno == EINTR)
            continue;
        break;
    }

    return ret;
}
