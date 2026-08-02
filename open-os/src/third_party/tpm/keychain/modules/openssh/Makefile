# Copyright (c) 2009 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

MODULE_NAME = tpm_openssh

TARGET = lib$(MODULE_NAME).so

OSNAME = $(shell uname)

CC = gcc
CFLAGS = -Wall -g -I../..
LIBS = -ltspi -lcrypto

ifeq ($(OSNAME), Darwin)
ARCHS = -arch i386
CFLAGS += -dynamiclib
LIBS += -liconv
else
ARCHS = -m32
CFLAGS += -fPIC -rdynamic -shared -Wl,-soname,$(TARGET)
LIBS += -luuid
endif

all: $(TARGET)

$(TARGET): $(MODULE_NAME).c ../../tpm_keychain_common.c
	$(CC) $(CFLAGS) $(ARCHS) -o $@ $^ $(LIBS)

clean:
	rm -f *.o *.so
	rm -rf *.dSYM
