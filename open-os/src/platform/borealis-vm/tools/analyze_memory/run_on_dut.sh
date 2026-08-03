#!/bin/bash
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -euo pipefail

SCRIPT_PATH=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

RUN_ON_HOST=false
RUN_ON_BOREALIS=false
while getopts 'hb' flag; do
  case "${flag}" in
    h)
      RUN_ON_HOST=true
      ;;
    b)
      RUN_ON_BOREALIS=true
      ;;
    *) error "Unexpected option ${flag}" ;;
  esac
done
shift $((OPTIND - 1))
host="$1"
shift

if [ "${RUN_ON_HOST}" = "${RUN_ON_BOREALIS}" ]; then
    echo "Syntax: $0 [-h | -b] host"
    echo -n "Use -h to analyze host memory, or -b to analyze"
    echo " Borealis memory (but not both)."
fi

# Disable warning about $* expanding on the client side; this is what we want.
# shellcheck disable=SC2029
if [ "${RUN_ON_HOST}" = true ]; then
    echo "RUNNING ON HOST ON ${host}" >&2
    ssh "${host}" \
        "cat >/home/chronos/analyze_cros_memory.py \
        && python3 -u /home/chronos/analyze_cros_memory.py $*" \
        < "${SCRIPT_PATH}/analyze_cros_memory.py"
elif [ "${RUN_ON_BOREALIS}" = true ]; then
    echo "RUNNING ON BOREALIS GUEST ON ${host}" >&2
    ssh "${host}" \
        "borealis-sh -- \
            bash -c 'cat >/home/chronos/analyze_cros_memory.py' \
        && borealis-sh --user=root -- \
            python3 -u /home/chronos/analyze_cros_memory.py $*" \
        < "${SCRIPT_PATH}/analyze_cros_memory.py"
fi
