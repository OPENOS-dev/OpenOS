#!/bin/bash
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -euo pipefail

SCRIPT_PATH=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

dut="$1"
pattern="$2"
shift 2

# Disable warning about $* expanding on the client side; this is what we want.
# shellcheck disable=SC2029
OUT="${pattern}-host.txt"
echo "Running on host ${dut} and writing to ${OUT}."
"${SCRIPT_PATH}/run_on_dut.sh" -h "${dut}" "$*" > "${OUT}"
OUT="${pattern}-guest.txt"
echo "Running on Borealis guest on ${dut} and writing to ${OUT}."
"${SCRIPT_PATH}/run_on_dut.sh" -b "${dut}" "$*" > "${OUT}"
