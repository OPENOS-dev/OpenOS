#!/bin/sh
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# The Realtek rtw8822ce card generates lots of AER logs on some boards after
# v6.1 kernel uprev, causing a huge amount of log spamming in the kernel logs.
# Mediatek and Realtek have confirmed that the AER is harmless, and the power
# consumption impact is minimal. So, disabling L0s state to workaround the log
# spamming until we figure out the root cause.

echo 0 > /sys/bus/pci/drivers/rtw_8822ce/0000:01:00.0/link/l0s_aspm
