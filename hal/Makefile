#
# Copyright (C) 2014 Realsil Corporation.
# Tristan Zhang <tristan_zhang@realsil.com.cn>.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

HAL_DIR := $(SRCDIR)/hal
HAL_INC := include

BTIF_DIR := btif/src
BTIF_INC := btif/include

MP_DIR := mp/src
MP_INC := mp/include

GKI_DIR := gki
GKI_INC := gki

HCI_DIR := hci/src
HCI_INC := hci/include

MAIN_DIR := main
#MAIN_INC := main

STACK_DIR := stack
STACK_INC := stack/include

LIBBT_DIR := libbt/src
LIBBT_INC := libbt/include

UTILS_DIR := utils/src
UTILS_INC := utils/include

CFLAGS += -I$(HAL_DIR)/$(MP_INC) -I$(HAL_DIR)/$(BTIF_INC) -I$(HAL_DIR)/$(GKI_INC)/ulinux \
          -I$(HAL_DIR)/$(GKI_INC)/common -I$(HAL_DIR)/$(HCI_INC) -I$(HAL_DIR)/$(STACK_INC) \
          -I$(HAL_DIR)/$(LIBBT_INC) -I$(HAL_DIR)/$(UTILS_INC) -I$(HAL_DIR)/$(HAL_INC)

.PHONY: all

OBJS = $(BTIF_DIR)/btif_core.o $(BTIF_DIR)/bluetooth.o
OBJS += $(MP_DIR)/bt_mp_base.o $(MP_DIR)/bt_mp_build.o $(MP_DIR)/bt_mp_device_base.o \
        $(MP_DIR)/bt_mp_module_base.o $(MP_DIR)/foundation.o $(MP_DIR)/bt_mp_api.o \
        $(MP_DIR)/bt_mp_transport.o $(MP_DIR)/bt_mp_device_efuse_base.o
OBJS += $(GKI_DIR)/ulinux/gki_ulinux.o $(GKI_DIR)/common/gki_debug.o $(GKI_DIR)/common/gki_time.o \
        $(GKI_DIR)/common/gki_buffer.o
OBJS += $(HCI_DIR)/hci_h4.o $(HCI_DIR)/hci_h5.o $(HCI_DIR)/userial.o $(HCI_DIR)/bt_skbuff.o \
        $(HCI_DIR)/bt_list.o $(HCI_DIR)/bt_hci_bdroid.o $(HCI_DIR)/bt_hw.o $(HCI_DIR)/btsnoop.o \
        $(HCI_DIR)/utils.o $(HCI_DIR)/bt_hci_bluez.o
OBJS += $(MAIN_DIR)/bte_conf.o $(MAIN_DIR)/bte_init.o $(MAIN_DIR)/bte_logmsg.o \
        $(MAIN_DIR)/bte_main.o $(MAIN_DIR)/bte_version.o
OBJS += $(STACK_DIR)/btu/btu_hcif.o $(STACK_DIR)/btu/btu_init.o $(STACK_DIR)/btu/btu_task.o \
        $(STACK_DIR)/hcic/hcicmds.o $(STACK_DIR)/hcic/hciblecmds.o
OBJS += $(LIBBT_DIR)/bt_vendor_uart.o $(LIBBT_DIR)/bt_vendor_usb.o $(LIBBT_DIR)/hardware.o \
        $(LIBBT_DIR)/userial_vendor.o $(LIBBT_DIR)/upio.o
OBJS += $(UTILS_DIR)/bt_utils.o $(UTILS_DIR)/bt_syslog.o

INCS = $(BTIF_INC)/btif_api.h $(BTIF)/btif_common.h $(BTIF)/btif_util.h
INCS += $(MP_INC)/bluetoothmp.h $(MP_INC)/bt_mp_api.h $(MP_INC)/bt_mp_base.h \
        $(MP_INC)/bt_mp_build.h $(MP_INC)/bt_mp_device_base.h \
        $(MP_INC)/bt_mp_module_base.h $(MP_INC)/bt_mp_transport.h \
        $(MP_INC)/foundation.h $(MP_INC)/bt_mp_device_efuse_base.h
INCS += $(GKI_INC)/common/gki.h $(GKI_INC)/common/gki_common.h $(GKI_INC)/common/gki_inet.h \
        $(GKI_INC)/ulinux/data_types.h $(GKI_INC)/ulinux/gki_int.h
INCS += $(HCI_INC)/bt_hci_bdroid.h $(HCI_INC)/bt_hci_lib.h $(HCI_INC)/bt_list.h \
        $(HCI_INC)/bt_skbuff.h $(HCI_INC)/bt_vendor_lib.h $(HCI_INC)/hci.h $(HCI_INC)/userial.h \
        $(HCI_INC)/utils.h $(HCI_DIR)/bt_hci_bluez.h
INCS += $(STACK_INC)/bt_types.h $(STACK_INC)/btu.h $(STACK_INC)/dyn_mem.h $(STACK_INC)/hcidefs.h \
        $(STACK_INC)/hcimsgs.h $(STACK_INC)/uipc_msg.h $(STACK_INC)/utfc.h $(STACK_INC)/wbt_api.h \
        $(STACK_INC)/wcassert.h
INCS += $(LIBBT_INC)/bt_vendor_uart.h $(LIBBT_INC)/bt_vendor_usb.h $(LIBBT_INC)/upio.h \
        $(LIBBT_INC)/userial_vendor.h $(LIBBT_INC)/vnd_generic.txt
INCS += $(UTILS_INC)/bt_utils.h $(UTILS_INC)/bt_syslog.h
INCS += $(HAL_INC)/bt_target.h $(HAL_INC)/bt_trace.h $(HAL_INC)/bte.h $(HAL_INC)/bte_appl.h \
        $(HAL_INC)/gki_target.h

all: $(OBJS)

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(MV) $@ $(OUTDIR)

#$(BTIF_DIR)/%.o: $(BTIF_DIR)/%.c $(BTIF_INC)/*.h
#	$(CC) $(CFLAGS) -c $< -o $@
#
#$(MP_DIR)/%.o: $(MP_DIR)/%.c $(MP_INC)/*.h
#	$(CC) $(CFLAGS) -c $< -o $@
#
#$(GKI_DIR)/ulinux/%.o: $(GKI_DIR)/ulinux/%.c $(GKI_INC)/ulinux/*.h
#	$(CC) $(CFLAGS) -c $< -o $@
#
#$(GKI_DIR)/common/%.o: $(GKI_DIR)/common/%.c $(GKI_INC)/common/*.h
#	$(CC) $(CFLAGS) -c $< -o $@
#
#$(HCI_DIR)/%.o: $(HCI_DIR)/%.c $(HCI_INC)/*.h
#	$(CC) $(CFLAGS) -c $< -o $@
#
#$(MAIN_DIR)/%.o: $(MAIN_DIR)/%.c
#	$(CC) $(CFLAGS) -c $< -o $@
#
#$(STACK_DIR)/btu/%.o: $(STACK_DIR)/btu/%.c $(STACK_INC)/*.h
#	$(CC) $(CFLAGS) -c $< -o $@
#
#$(STACK_DIR)/hcic/%.o: $(STACK_DIR)/hcic/%.c $(STACK_INC)/*.h
#	$(CC) $(CFLAGS) -c $< -o $@
#
#$(LIBBT_DIR)/%.o: $(LIBBT_DIR)/%.c $(LIBBT_INC)/*.h
#	$(CC) $(CFLAGS) -c $< -o $@
#
#$(UTILS_DIR)/%.o: $(UTILS_DIR)/%.c $(UTILS_INC)/*.h
#	$(CC) $(CFLAGS) -c $< -o $@
