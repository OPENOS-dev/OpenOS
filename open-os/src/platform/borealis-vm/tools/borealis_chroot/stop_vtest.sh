#!/bin/bash
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Stops vtest in the chroot

set -euo pipefail

VTEST_SERVER_PID=$(cat /tmp/vtest-server-pid)
pkill -P "${VTEST_SERVER_PID}"
rm /tmp/vtest-server-pid
