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

MKDIR := mkdir -p
RM := rm -f
MV := mv -f
INSTALL := install
CC := gcc
CFLAGS := -O2 -g -D_GNU_SOURCE -Wall -Wundef -Wno-unused-result -Wno-unused-variable \
          -Wno-unused-but-set-variable -Werror-implicit-function-declaration \
          -Wno-error=uninitialized -Wno-strict-aliasing
LDFLAGS := -lpthread -lrt

export SRCDIR OUTDIR MV CC CFLAGS

.PHONY: all rtlbtmp install uninstall clean

all: $(TARGET)

CMD_DIR := cmd
HAL_DIR := hal
SUBDIRS := $(CMD_DIR) $(HAL_DIR)

#include $(SUBDIRS:%=%/Makefile.mk)

$(TARGET):
	$(MKDIR) $(OUTDIR)
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; done
	$(CC) $(CFLAGS) $(OUTDIR)/*.o -o $(TARGET) $(LDFLAGS)

install: $(TARGET)
	$(MKDIR) $(DESTDIR)$(SBINDIR)
	$(INSTALL) -m 755 -t $(DESTDIR)$(SBINDIR) $(TARGET)

uninstall:
	$(RM) $(DESTDIR)$(SBINDIR)/$(TARGET)

clean:
	$(RM) $(TARGET)
	$(RM) -r $(OUTDIR)