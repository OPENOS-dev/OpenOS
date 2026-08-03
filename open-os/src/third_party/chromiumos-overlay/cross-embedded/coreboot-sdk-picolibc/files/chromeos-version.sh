#!/bin/bash
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e

SCRIPT_DIR="$(dirname "$(realpath -e "${BASH_SOURCE[0]}")")"

# Use the coreboot-sdk GCC version for the subtool version.
sed -Ene "s/^DIST gcc-([0-9.]+)\.tar.*$/\1/p" "${SCRIPT_DIR}/../../coreboot-sdk/Manifest"
