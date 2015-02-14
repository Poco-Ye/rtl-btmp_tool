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
 *  Filename:      bt_vendor_if.c
 *
 *  Description:   Contains vendor I/O functions, like
 *                 -- rfkill control
 *                 -- userial settings
 *
 ******************************************************************************/

#define LOG_TAG "bt_vendor_if"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "bt_syslog.h"
#include "bt_vendor_if.h"

bt_vendor_callbacks_t *bt_vendor_cbacks = NULL;

uint8_t vnd_local_bd_addr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static int rfkill_id = -1;
static char *rfkill_state_path = NULL;
static vnd_if_cb_t vnd_uart;

static int init_rfkill()
{
    char path[64];
    char buf[16];
    int fd, sz, id;

    for (id = 0; ; id++) {
        snprintf(path, sizeof(path), "/sys/class/rfkill/rfkill%d/type", id);
        fd = open(path, O_RDONLY);
        if (fd < 0) {
            SYSLOGE("init_rfkill : open(%s) failed: %s (%d)",
                    path, strerror(errno), errno);
            return -1;
        }

        sz = read(fd, &buf, sizeof(buf));
        close(fd);

        if (sz >= 9 && memcmp(buf, "bluetooth", 9) == 0) {
            rfkill_id = id;
            break;
        }
    }

    asprintf(&rfkill_state_path, "/sys/class/rfkill/rfkill%d/state", rfkill_id);
    return 0;
}

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
int bt_vendor_set_power(int on)
{
    int sz;
    int fd = -1;
    int ret = -1;
    char buffer = '0';

    switch (on) {
    case BT_POWER_OFF:
        buffer = '0';
        break;

    case BT_POWER_ON:
        buffer = '1';
        break;
    }

    if (rfkill_id == -1) {
        if (init_rfkill())
            return ret;
    }

    fd = open(rfkill_state_path, O_WRONLY);
    if (fd < 0) {
        SYSLOGE("set_bluetooth_power : open(%s) for write failed: %s (%d)",
                rfkill_state_path, strerror(errno), errno);
        return ret;
    }

    sz = write(fd, &buffer, 1);
    if (sz < 0) {
        SYSLOGE("set_bluetooth_power : write(%s) failed: %s (%d)",
                rfkill_state_path, strerror(errno),errno);
    } else
        ret = 0;

    if (fd >= 0)
        close(fd);

    return ret;
}

/*******************************************************************************
**
** Function        userial_to_tcio_baud
**
** Description     helper function converts USERIAL baud rates into TCIO
**                 conforming baud rates
**
** Returns         TRUE/FALSE
**
*******************************************************************************/
uint8_t userial_to_tcio_baud(uint8_t cfg_baud, uint32_t *baud)
{
    if (cfg_baud == USERIAL_BAUD_115200)
        *baud = B115200;
    else if (cfg_baud == USERIAL_BAUD_4M)
        *baud = B4000000;
    else if (cfg_baud == USERIAL_BAUD_3M)
        *baud = B3000000;
    else if (cfg_baud == USERIAL_BAUD_2M)
        *baud = B2000000;
    else if (cfg_baud == USERIAL_BAUD_1M)
        *baud = B1000000;
    else if (cfg_baud == USERIAL_BAUD_1_5M)
        *baud = B1500000;
    else if (cfg_baud == USERIAL_BAUD_921600)
        *baud = B921600;
    else if (cfg_baud == USERIAL_BAUD_460800)
        *baud = B460800;
    else if (cfg_baud == USERIAL_BAUD_230400)
        *baud = B230400;
    else if (cfg_baud == USERIAL_BAUD_57600)
        *baud = B57600;
    else if (cfg_baud == USERIAL_BAUD_19200)
        *baud = B19200;
    else if (cfg_baud == USERIAL_BAUD_9600)
        *baud = B9600;
    else if (cfg_baud == USERIAL_BAUD_1200)
        *baud = B1200;
    else if (cfg_baud == USERIAL_BAUD_600)
        *baud = B600;
    else {
        SYSLOGE( "userial vendor open: unsupported baud idx %i", cfg_baud);
        *baud = B115200;
        return FALSE;
    }

    return TRUE;
}

/*******************************************************************************
**
** Function        userial_vendor_init
**
** Description     Initialize userial control block
**
** Returns         None
**
*******************************************************************************/
void userial_vendor_init(const char *dev_node)
{
    const char *port;

    vnd_uart.fd = -1;

    port = dev_node ?: BLUETOOTH_UART_DEVICE_PORT;
    snprintf(vnd_uart.port_name, VND_PORT_NAME_MAXLEN, "%s", port);
}

/*******************************************************************************
**
** Function        userial_vendor_open
**
** Description     Open the serial port with the given configuration
**
** Returns         device fd
**
*******************************************************************************/
int userial_vendor_open(tUSERIAL_CFG *p_cfg)
{
    uint32_t baud;
    uint8_t data_bits;
    uint16_t parity;
    uint8_t stop_bits;

    vnd_uart.fd = -1;

    if (!userial_to_tcio_baud(p_cfg->baud, &baud)) {
        return -1;
    }

    if (p_cfg->fmt & USERIAL_DATABITS_8)
        data_bits = CS8;
    else if(p_cfg->fmt & USERIAL_DATABITS_7)
        data_bits = CS7;
    else if(p_cfg->fmt & USERIAL_DATABITS_6)
        data_bits = CS6;
    else if(p_cfg->fmt & USERIAL_DATABITS_5)
        data_bits = CS5;
    else {
        SYSLOGE("userial vendor open: unsupported data bits");
        return -1;
    }

    if (p_cfg->fmt & USERIAL_PARITY_NONE)
        parity = 0;
    else if(p_cfg->fmt & USERIAL_PARITY_EVEN)
        parity = PARENB;
    else if(p_cfg->fmt & USERIAL_PARITY_ODD)
        parity = (PARENB | PARODD);
    else {
        SYSLOGE("userial vendor open: unsupported parity bit mode");
        return -1;
    }

    if (p_cfg->fmt & USERIAL_STOPBITS_1)
        stop_bits = 0;
    else if(p_cfg->fmt & USERIAL_STOPBITS_2)
        stop_bits = CSTOPB;
    else {
        SYSLOGE("userial vendor open: unsupported stop bits");
        return -1;
    }

    SYSLOGI("userial vendor open: opening %s", vnd_uart.port_name);

    vnd_uart.fd = open(vnd_uart.port_name, O_RDWR);
    if (vnd_uart.fd == -1) {
        SYSLOGE("userial vendor open: unable to open %s, %s (%d)",
                vnd_uart.port_name, strerror(errno), errno);
        return -1;
    }

    tcflush(vnd_uart.fd, TCIOFLUSH);

    tcgetattr(vnd_uart.fd, &vnd_uart.termios);
    cfmakeraw(&vnd_uart.termios);

    if (p_cfg->hw_fctrl == USERIAL_HW_FLOW_CTRL_ON) {
        SYSLOGI("userial vendor open: with HW flowctrl ON");
        vnd_uart.termios.c_cflag |= (CRTSCTS | stop_bits| parity);
    } else {
        SYSLOGI("userial vendor open: with HW flowctrl OFF");
        vnd_uart.termios.c_cflag &= ~CRTSCTS;
        vnd_uart.termios.c_cflag |= (stop_bits| parity);
    }

    tcsetattr(vnd_uart.fd, TCSANOW, &vnd_uart.termios);
    tcflush(vnd_uart.fd, TCIOFLUSH);

    tcsetattr(vnd_uart.fd, TCSANOW, &vnd_uart.termios);
    tcflush(vnd_uart.fd, TCIOFLUSH);
    tcflush(vnd_uart.fd, TCIOFLUSH);

    /* set input/output baudrate */
    cfsetospeed(&vnd_uart.termios, baud);
    cfsetispeed(&vnd_uart.termios, baud);
    tcsetattr(vnd_uart.fd, TCSANOW, &vnd_uart.termios);

    SYSLOGI("device fd = %d open", vnd_uart.fd);

    return vnd_uart.fd;
}

/*******************************************************************************
**
** Function        userial_vendor_close
**
** Description     Conduct vendor-specific close work
**
** Returns         None
**
*******************************************************************************/
void userial_vendor_close(void)
{
    int result;

    SYSLOGI("device fd = %d close", vnd_uart.fd);

    if (vnd_uart.fd == -1)
        return;

    if ((result = close(vnd_uart.fd)) < 0)
        SYSLOGE( "close(fd:%d) FAILED result:%d", vnd_uart.fd, result);

    vnd_uart.fd = -1;
}

/*******************************************************************************
**
** Function        userial_vendor_set_baud
**
** Description     Set new baud rate
**
** Returns         None
**
*******************************************************************************/
void userial_vendor_set_baud(uint8_t userial_baud)
{
    uint32_t tcio_baud;

    SYSLOGI("userial_vendor_set_baud: userial_baud 0x%02x", userial_baud);

    userial_to_tcio_baud(userial_baud, &tcio_baud);

    if (cfsetospeed(&vnd_uart.termios, tcio_baud) < 0)
        SYSLOGE("cfsetospeed fail");

    if (cfsetispeed(&vnd_uart.termios, tcio_baud) < 0)
        SYSLOGE("cfsetispeed fail");

    if (tcsetattr(vnd_uart.fd, TCSANOW, &vnd_uart.termios) < 0)
        SYSLOGE("tcsetattr fail");

    tcflush(vnd_uart.fd, TCIOFLUSH);
}

/*******************************************************************************
**
** Function        userial_vendor_set_hw_fctrl
**
** Description     Conduct vendor-specific close work
**
** Returns         None
**
*******************************************************************************/
void userial_vendor_set_hw_fctrl(uint8_t hw_fctrl)
{
    struct termios termios_old;

    if (vnd_uart.fd == -1) {
        SYSLOGE("vnd_uart.fd is -1");
        return;
    }

    tcgetattr(vnd_uart.fd, &termios_old);
    if (hw_fctrl) {
        SYSLOGI("Set HW flow control ON");
        if (termios_old.c_cflag & CRTSCTS) {
            SYSLOGI("userial_vendor_set_hw_fctrl already hw flowcontrol on");
            return;
        } else {
            termios_old.c_cflag |= CRTSCTS;
            tcsetattr(vnd_uart.fd, TCSANOW, &termios_old);
            SYSLOGI("userial_vendor_set_hw_fctrl set hw flowcontrol on");
        }
    } else {
        SYSLOGI("Set HW FlowControl Off");
        if (termios_old.c_cflag & CRTSCTS) {
            termios_old.c_cflag &= ~CRTSCTS;
            tcsetattr(vnd_uart.fd, TCSANOW, &termios_old);
            return;
        } else {
            SYSLOGI("userial_vendor_set_hw_fctrl set hw flowcontrol off");
            return;
        }
    }
}
