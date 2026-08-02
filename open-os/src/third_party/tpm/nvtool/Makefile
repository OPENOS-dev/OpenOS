# Copyright (c) 2009,2010 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

TPM_NVTOOL = tpm-nvtool

OSNAME = $(shell uname)

CC ?= gcc
CPPFLAGS += -I.
CFLAGS += -Wall -g
LIBS = -ltspi

ifeq ($(OSNAME), Darwin)
CPPFLAGS += -arch i386
LDFLAGS += -arch i386
LIBS += -liconv
endif

TPM_NVTOOL_OBJS = main.o          \
                  tpm_nv.o        \
                  tpm_nv_common.o

all: pre-build $(TPM_NVTOOL)

pre-build:
	@/bin/sh ./help/help-gen.sh

$(TPM_NVTOOL): $(TPM_NVTOOL_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

-include $(OBJS:.o=.d)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $*.c -c -o $*.o
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

clean:
	rm -f $(TPM_NVTOOL) *.o *.d help/help.h
