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

CMD_DIR := src
CMD_INC := include
CMD_MP_INC := $(SRCDIR)/hal/mp/include
CMD_LOG_INC := $(SRCDIR)/hal/utils/include

CFLAGS += -I$(CMD_INC) -I$(CMD_MP_INC) -I$(CMD_LOG_INC)

.PHONY: all

OBJS = $(CMD_DIR)/btmp_if.o $(CMD_DIR)/btmp_shell.o $(CMD_DIR)/btmp_socket.o

all: $(OBJS)

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(MV) $@ $(OUTDIR)
