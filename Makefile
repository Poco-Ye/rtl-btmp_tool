#
# Copyright (C) 2014 Realsil Corporation.
# Tristan Zhang <tristan_zhang@realsil.com.cn>

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

DESTDIR =
PREFIX ?= /usr
SBINDIR ?= $(PREFIX)/sbin
SRCDIR := $(PWD)
OUTDIR := $(SRCDIR)/out
TARGET := rtlbtmp
TARGET_SKT := rtlbtmp_skt

MKDIR := mkdir -p
RM := rm -f
MV := mv -f

INSTALL := install
MYDIR := /home/roc_wang/mp_tool_linux_compile_exercise_20170809/toolchain-sunxi-musl/toolchain
CC := $(MYDIR)/bin/arm-openwrt-linux-muslgnueabi-gcc
BITS := $(MYDIR)/include
CFLAGS := -I $(BITS)  --sysroot= $(MYDIR) \
          -O2 -D_GNU_SOURCE -Wall -Wundef -Wno-unused-result -Wno-unused-variable \
          -Wno-unused-but-set-variable -Werror-implicit-function-declaration \
          -Wno-error=uninitialized -Wno-strict-aliasing
		  
LDFLAGS := -lpthread -lrt -lm
#LDFLAGS := -lc

	
export SRCDIR OUTDIR MV CC CFLAGS
	
.PHONY: all rtlbtmp install uninstall clean
	
all:  $(TARGET) $(TARGET_SKT)
	
CMD_DIR := cmd
HAL_DIR := hal
SUBDIRS := $(CMD_DIR) $(HAL_DIR)

$(TARGET): $(OUTDIR)
	$(CC) $(CFLAGS) $(filter-out $(OUTDIR)/btmp_socket.o,$(shell ls $(OUTDIR)/*.o)) -o $(TARGET) $(LDFLAGS)

$(TARGET_SKT): $(OUTDIR)
	$(CC) $(CFLAGS) $(filter-out $(OUTDIR)/btmp_shell.o,$(shell ls $(OUTDIR)/*.o)) -o $(TARGET_SKT) $(LDFLAGS)
	
	

$(OUTDIR):
	$(MKDIR) $(OUTDIR)
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; done

	
		
install: $(TARGET) $(TARGET_SKT)
	$(MKDIR) $(DESTDIR)$(SBINDIR)
	$(INSTALL) -m 755 -t $(DESTDIR)$(SBINDIR) $(TARGET)
	$(INSTALL) -m 755 -t $(DESTDIR)$(SBINDIR) $(TARGET_SKT)

uninstall:
	$(RM) $(DESTDIR)$(SBINDIR)/$(TARGET)
	$(RM) $(DESTDIR)$(SBINDIR)/$(TARGET_SKT)

envprint:
	$(ENV)
clean:
	$(RM) $(TARGET)
	$(RM) $(TARGET_SKT)
	$(RM) -r $(OUTDIR)
