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
 *  Filename:      bt_hci_bluez.h
 *
 *  Description:   bluez interfaces declaration
 *
 ******************************************************************************/

#ifndef BT_HCI_BLUEZ_H
#define BT_HCI_BLUEZ_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/socket.h>


#ifndef AF_BLUETOOTH
#define AF_BLUETOOTH    31
#define PF_BLUETOOTH    AF_BLUETOOTH
#endif

#define BTPROTO_L2CAP   0
#define BTPROTO_HCI     1
#define BTPROTO_SCO     2
#define BTPROTO_RFCOMM  3
#define BTPROTO_BNEP    4
#define BTPROTO_CMTP    5
#define BTPROTO_HIDP    6
#define BTPROTO_AVDTP   7

#define SOL_HCI     0
#define SOL_L2CAP   6
#define SOL_SCO     17
#define SOL_RFCOMM  18

#ifndef SOL_BLUETOOTH
#define SOL_BLUETOOTH   274
#endif

struct sockaddr_hci {
    sa_family_t hci_family;
    unsigned short  hci_dev;
    unsigned short  hci_channel;
};

#define HCI_MAX_DEV 16

/* HCI device flags */
enum {
    HCI_UP,
    HCI_INIT,
    HCI_RUNNING,

    HCI_PSCAN,
    HCI_ISCAN,
    HCI_AUTH,
    HCI_ENCRYPT,
    HCI_INQUIRY,

    HCI_RAW,
};

/* HCI ioctl defines */
#define HCIDEVUP    _IOW('H', 201, int)
#define HCIDEVDOWN  _IOW('H', 202, int)
#define HCIDEVRESET _IOW('H', 203, int)
#define HCIDEVRESTAT    _IOW('H', 204, int)

#define HCIGETDEVLIST   _IOR('H', 210, int)
#define HCIGETDEVINFO   _IOR('H', 211, int)
#define HCIGETCONNLIST  _IOR('H', 212, int)
#define HCIGETCONNINFO  _IOR('H', 213, int)
#define HCIGETAUTHINFO  _IOR('H', 215, int)

#define HCISETRAW   _IOW('H', 220, int)
#define HCISETSCAN  _IOW('H', 221, int)
#define HCISETAUTH  _IOW('H', 222, int)
#define HCISETENCRYPT   _IOW('H', 223, int)
#define HCISETPTYPE _IOW('H', 224, int)
#define HCISETLINKPOL   _IOW('H', 225, int)
#define HCISETLINKMODE  _IOW('H', 226, int)
#define HCISETACLMTU    _IOW('H', 227, int)
#define HCISETSCOMTU    _IOW('H', 228, int)

#define HCIBLOCKADDR    _IOW('H', 230, int)
#define HCIUNBLOCKADDR  _IOW('H', 231, int)

#define HCIINQUIRY  _IOR('H', 240, int)

/* HCI Packet types */
#define HCI_COMMAND_PKT     0x01
#define HCI_ACLDATA_PKT     0x02
#define HCI_SCODATA_PKT     0x03
#define HCI_EVENT_PKT       0x04
#define HCI_VENDOR_PKT      0xff

/* --------  HCI Packet structures  -------- */
#define HCI_TYPE_LEN    1

typedef struct {
    uint16_t    opcode; /* OCF & OGF */
    uint8_t     plen;
} __attribute__ ((packed)) hci_command_hdr;
#define HCI_COMMAND_HDR_SIZE    3

typedef struct {
    uint8_t     evt;
    uint8_t     plen;
} __attribute__ ((packed)) hci_event_hdr;
#define HCI_EVENT_HDR_SIZE  2

typedef struct {
    uint16_t    handle; /* Handle & Flags(PB, BC) */
    uint16_t    dlen;
} __attribute__ ((packed)) hci_acl_hdr;
#define HCI_ACL_HDR_SIZE    4

typedef struct {
    uint16_t    handle;
    uint8_t     dlen;
} __attribute__ ((packed)) hci_sco_hdr;
#define HCI_SCO_HDR_SIZE    3

typedef union {
    hci_command_hdr cmd_hdr;
    hci_event_hdr   evt_hdr;
    hci_acl_hdr     acl_hdr;
    hci_sco_hdr     sco_hdr;
} __attribute__ ((packed)) hci_pkt_hdr;


/* BD Address */
typedef struct {
    uint8_t b[6];
} __attribute__((packed)) bdaddr_t;

#define BDADDR_ANY   (&(bdaddr_t) {{0, 0, 0, 0, 0, 0}})

/* Ioctl requests structures */
struct hci_dev_stats {
    uint32_t err_rx;
    uint32_t err_tx;
    uint32_t cmd_tx;
    uint32_t evt_rx;
    uint32_t acl_tx;
    uint32_t acl_rx;
    uint32_t sco_tx;
    uint32_t sco_rx;
    uint32_t byte_rx;
    uint32_t byte_tx;
};

struct hci_dev_info {
    uint16_t dev_id;
    char     name[8];

    bdaddr_t bdaddr;

    uint32_t flags;
    uint8_t  type;

    uint8_t  features[8];

    uint32_t pkt_type;
    uint32_t link_policy;
    uint32_t link_mode;

    uint16_t acl_mtu;
    uint16_t acl_pkts;
    uint16_t sco_mtu;
    uint16_t sco_pkts;

    struct   hci_dev_stats stat;
};

struct hci_dev_req {
    uint16_t dev_id;
    uint32_t dev_opt;
};

struct hci_dev_list_req {
    uint16_t dev_num;
    struct hci_dev_req dev_req[0];  /* hci_dev_req structures */
};

static inline int bacmp(const bdaddr_t *ba1, const bdaddr_t *ba2)
{
    return memcmp(ba1, ba2, sizeof(bdaddr_t));
}

static inline int hci_test_bit(int nr, void *addr)
{
    return *((uint32_t *) addr + (nr >> 5)) & (1 << (nr & 31));
}

int hci_devid(const char *str);
int hci_open_dev(int dev_id);
int hci_close_dev(int dd);
int hci_bluez_read(int dd, void *pbuf, size_t plen);
int hci_bluez_write(int dd, void *pbuf, uint16_t plen);

#ifdef __cplusplus
}
#endif

#endif /* BT_HCI_BLUEZ_H */
