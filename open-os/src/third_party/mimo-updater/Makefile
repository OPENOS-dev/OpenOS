# Copyright 2017 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

include common.mk

TARGET_NAME := mimo-updater

PC_DEPS = libbrillo libchrome libusb-1.0
PC_LIBS := $(shell $(PKG_CONFIG) --libs $(PC_DEPS))
PC_CFLAGS := $(shell $(PKG_CONFIG) --cflags $(PC_DEPS))

CPPFLAGS += $(PC_CFLAGS)
LDLIBS += $(PC_LIBS)

all: CXX_BINARY($(TARGET_NAME))

CXX_BINARY($(TARGET_NAME)): \
	mimo_fw_updater.o

clean: CLEAN(CXX_BINARY($(TARGET_NAME)))
