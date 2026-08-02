#!/bin/bash
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

SCRIPT_DIR="$(dirname "$(realpath -e "${BASH_SOURCE[0]}")")"

# Track the QEMU ebuild version so CIPD metadata is a little more obvious.
QEMU_DIR="${SCRIPT_DIR}/../../../../portage-stable/app-emulation/qemu"
find "${QEMU_DIR}" -maxdepth 1 -name '*.ebuild' -printf '%f\n' | \
  sed -e 's:^qemu-::' -e 's:\.ebuild$::' -e 's:-r[0-9]*::' | \
  sort -V | \
  tail -n1
