#!/bin/bash

# Copyright 2024 The Chromium OS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

modify_kernel_command_line() {
  local config_file="$1"

  # Allow ChromiumOS LSM to mount overlayfs.
  echo "chromiumos.allow_overlayfs" >> "${config_file}"
}
