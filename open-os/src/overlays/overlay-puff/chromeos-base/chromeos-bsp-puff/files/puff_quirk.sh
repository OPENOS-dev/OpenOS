#!/bin/sh
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

model=$(cros_config / name)

if [ "${model}" != "dooly" ]; then
    echo "730" > /sys/class/gpio/export
    echo "in" > /sys/class/gpio/gpio730/direction
fi
