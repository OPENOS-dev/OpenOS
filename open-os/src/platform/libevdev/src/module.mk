# Copyright 2012 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
include common.mk

CFLAGS += -std=gnu99 -fvisibility=default
CPPFLAGS += -I$(SRC)/include
SONAME=libevdev-cros.so.0
SONAME_HOLLOW=libevdev_hollow.so.0

CC_LIBRARY(src/$(SONAME_HOLLOW)): CPPFLAGS += -DEVDEV_HOLLOW
CC_LIBRARY(src/$(SONAME_HOLLOW)): LDFLAGS += -Wl,-soname,$(SONAME)
CC_LIBRARY(src/$(SONAME_HOLLOW)): src/libevdev.o \
	src/libevdev_mt.o \
	src/libevdev_event.o

CC_LIBRARY(src/$(SONAME)): LDFLAGS += -Wl,-soname,$(SONAME)
CC_LIBRARY(src/$(SONAME)): src/libevdev.o \
	src/libevdev_mt.o \
	src/libevdev_event.o

install-lib: CC_LIBRARY(src/$(SONAME))
	install -D -m 0755 src/$(SONAME) \
			$(DESTDIR)$(LIBDIR)/$(SONAME)
	ln -f -s $(SONAME) $(DESTDIR)$(LIBDIR)/libevdev-cros.so
	install -D -m 0644 $(SRC)/libevdev-cros.pc \
			$(DESTDIR)$(LIBDIR)/pkgconfig/libevdev-cros.pc

setup-lib-in-place:
	mkdir -p $(SRC)/in-place || true
	ln -sf $(SRC)/src/$(SONAME_HOLLOW) $(SRC)/in-place/libevdev-cros.so
	ln -sf $(SRC)/src/$(SONAME_HOLLOW) $(SRC)/in-place/libevdev-cros.so.0
