# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

UDEV_RULE=99-inputcontrol.rules

MODPROBE_CONF=modprobe.d/inputcontrol.conf

DESTDIR = .

install: $(SCRIPTS)
	install -D -m 0644 $(MODPROBE_CONF) \
		$(DESTDIR)/etc/$(MODPROBE_CONF)
	install -D -m 0644 $(UDEV_RULE) \
		$(DESTDIR)/lib/udev/rules.d/$(UDEV_RULE)
