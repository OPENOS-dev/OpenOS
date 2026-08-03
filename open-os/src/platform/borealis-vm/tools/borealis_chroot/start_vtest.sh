#!/bin/bash
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Starts vtest in the chroot

set -euo pipefail

if [ ! -d virglrenderer ]; then
  # Clone and build virglrenderer if it has not been done before
  git clone https://chromium.googlesource.com/chromiumos/third_party/virglrenderer
  cd virglrenderer
  meson out -Dvenus=true
  ninja -C out
  cd -
fi

if [ ! -f /tmp/vtest-server-pid ]; then
  meson devenv -C virglrenderer/out \
    ./vtest/virgl_test_server --no-fork --venus --multi-clients &
  VTEST_SERVER_PID=$!
  echo "${VTEST_SERVER_PID}" > /tmp/vtest-server-pid
fi
