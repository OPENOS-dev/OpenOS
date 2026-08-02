#!/bin/sh
#
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Retrieve line 1, 2 and 4 of VERSION
awk '{ if (NR==1 || NR==2) { printf("%s.", $3) } else if (NR==4) { printf $3 } }' "$1/zephyr/VERSION"
