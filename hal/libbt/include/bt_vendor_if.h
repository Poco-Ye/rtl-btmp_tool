/******************************************************************************
 *
 *  Copyright (C) 2015 Realtek Corporation
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
 *  Filename:      bt_vendor_if.h
 *
 *  Description:   Contains definitions used for I/O controls
 *
 ******************************************************************************/

#ifndef BT_VENDOR_IF_H
#define BT_VENDOR_IF_H

#include <termios.h>
#include <unistd.h>

#include "bt_vendor_lib.h"
#include "userial.h"

#ifndef FALSE
#define FALSE  0
#endif

#ifndef TRUE
#define TRUE   (!FALSE)
#endif

#define BT_POWER_OFF 0
#define BT_POWER_ON  1

#ifndef USE_CONTROLLER_BDADDR
#define USE_CONTROLLER_BDADDR   FALSE
#endif

/* Device port name where Bluetooth controller attached */
#ifndef BLUETOOTH_UART_DEVICE_PORT
#define BLUETOOTH_UART_DEVICE_PORT      "/dev/ttyS0"
#endif

/* Device port name where Bluetooth controller attached */
#ifndef BLUETOOTH_USB_DEVICE_PORT
#define BLUETOOTH_USB_DEVICE_PORT      "/dev/rtk_btusb"
#endif

/**** baud rates ****/
#define USERIAL_BAUD_300        0
#define USERIAL_BAUD_600        1
#define USERIAL_BAUD_1200       2
#define USERIAL_BAUD_2400       3
#define USERIAL_BAUD_9600       4
#define USERIAL_BAUD_19200      5
#define USERIAL_BAUD_57600      6
#define USERIAL_BAUD_115200     7
#define USERIAL_BAUD_230400     8
#define USERIAL_BAUD_460800     9
#define USERIAL_BAUD_921600     10
#define USERIAL_BAUD_1M         11
#define USERIAL_BAUD_1_5M       12
#define USERIAL_BAUD_2M         13
#define USERIAL_BAUD_3M         14
#define USERIAL_BAUD_4M         15
#define USERIAL_BAUD_AUTO       16

/**** Data Format ****/
/* Stop Bits */
#define USERIAL_STOPBITS_1      1
#define USERIAL_STOPBITS_1_5    (1<<1)
#define USERIAL_STOPBITS_2      (1<<2)

/* Parity Bits */
#define USERIAL_PARITY_NONE     (1<<3)
#define USERIAL_PARITY_EVEN     (1<<4)
#define USERIAL_PARITY_ODD      (1<<5)

/* Data Bits */
#define USERIAL_DATABITS_5      (1<<6)
#define USERIAL_DATABITS_6      (1<<7)
#define USERIAL_DATABITS_7      (1<<8)
#define USERIAL_DATABITS_8      (1<<9)

/* Hardware Flow Control */
#define USERIAL_HW_FLOW_CTRL_OFF    0
#define USERIAL_HW_FLOW_CTRL_ON     1

/* Structure used to configure serial port during open */
typedef struct {
    uint16_t fmt;     /* Data format */
    uint8_t  baud;    /* Baud rate */
    uint8_t hw_fctrl; /* hardware flow control */
} tUSERIAL_CFG;

#define VND_PORT_NAME_MAXLEN    256

/* vendor serial control block */
typedef struct {
    int fd;                     /* fd to Bluetooth device */
    struct termios termios;     /* serial terminal of BT port */
    char port_name[VND_PORT_NAME_MAXLEN]; /* port string name */
} vnd_if_cb_t;

extern bt_vendor_callbacks_t *bt_vendor_cbacks;

extern uint8_t vnd_local_bd_addr[6];

/*******************************************************************************
**
** Function        bt_vendor_set_power
**
** Description     Interact with low layer driver to set Bluetooth power
**                 on/off.
**
** Returns         0  : SUCCESS or Not-Applicable
**                 <0 : ERROR
**
*******************************************************************************/
int bt_vendor_set_power(int on);

/*******************************************************************************
**
** Function        userial_vendor_init
**
** Description     Initialize userial vendor-specific control block
**
** Returns         None
**
*******************************************************************************/
void userial_vendor_init(const char *dev_node);

/*******************************************************************************
**
** Function        userial_vendor_open
**
** Description     Open the serial port with the given configuration
**
** Returns         device fd
**
*******************************************************************************/
int userial_vendor_open(tUSERIAL_CFG *p_cfg);

/*******************************************************************************
**
** Function        userial_vendor_close
**
** Description     Conduct vendor-specific close work
**
** Returns         None
**
*******************************************************************************/
void userial_vendor_close(void);

/*******************************************************************************
**
** Function        userial_vendor_set_baud
**
** Description     Set new baud rate
**
** Returns         None
**
*******************************************************************************/
void userial_vendor_set_baud(uint8_t userial_baud);

/*******************************************************************************
**
** Function        userial_vendor_set_hw_fctrl
**
** Description     Set hw flow control
**
** Returns         None
**
*******************************************************************************/
void userial_vendor_set_hw_fctrl(uint8_t hw_fctrl);

#endif /* BT_VENDOR_IF_H */
