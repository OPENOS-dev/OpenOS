#!/bin/bash
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -euo pipefail

SCRIPT_PATH=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

: "${MEMTRACKER_INSTALL_PATH:=/home/chronos/user/MyFiles/Downloads}"

host="$1"
shift

echo "RUNNING ON DEVICE ${host}"

# Disable warning about $* expanding on the client side; this is what we want.
# shellcheck disable=SC2029
ssh "${host}" \
    "rm -f ${MEMTRACKER_INSTALL_PATH}/memtracker_metrics.json \
     && cat >${MEMTRACKER_INSTALL_PATH}/memtracker.py \
     && python3 -u ${MEMTRACKER_INSTALL_PATH}/memtracker.py $*" \
    < "${SCRIPT_PATH}/memtracker.py"
