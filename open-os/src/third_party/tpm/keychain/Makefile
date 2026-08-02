# Copyright (c) 2009,2010 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

TPM_KEYCHAIN = tpm-keychain

OSNAME = $(shell uname)

CC = gcc
CFLAGS = -Wall -g -I.
LIBS = -ltspi -lcrypto

ifeq ($(OSNAME), Darwin)
ARCHS = -arch i386
LIBS += -liconv
else
ARCHS =
LIBS += -luuid
endif

TPM_KEYCHAIN_OBJS = main.o                  \
                    tpm_keychain_common.o   \
                    tpm_keychain.o          \
                    tpm_keychain_internal.o \
                    util.o

all: pre-build $(TPM_KEYCHAIN)

pre-build: help/help.txt
	@/bin/sh ./help/help-gen.sh

$(TPM_KEYCHAIN): $(TPM_KEYCHAIN_OBJS)
	$(CC) $(CFLAGS) $(ARCHS) -o $@ $^ $(LIBS)

-include $(OBJS:.o=.d)

%.o: %.c
	$(CC) $(CFLAGS) $(ARCHS) $*.c -c -o $*.o
	$(CC) $(CFLAGS) $(ARCHS) -MM $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

clean:
	rm -f $(TPM_KEYCHAIN) *.o *.d help/help.h
