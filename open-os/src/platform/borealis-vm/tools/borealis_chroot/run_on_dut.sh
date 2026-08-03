#!/bin/bash
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -euo pipefail

SCRIPT_PATH=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
HOMEDIR="/mnt/stateful_partition"

# Running on Linux (not ChromeOS).
LINUX=false
# Mount the stateful image file from an existing Borealis install.
MOUNT_BOREALIS_STATEFUL=false
# Btrfs external disk image to mount on /mnt/external/0.
EXTERNAL_DISK_IMAGE=""

while getopts 'l:mx:' flag; do
  case "${flag}" in
    l)
      LINUX=true
      HOMEDIR="${OPTARG}"
      ;;
    m) MOUNT_BOREALIS_STATEFUL=true ;;
    x) EXTERNAL_DISK_IMAGE="${OPTARG}" ;;
    *) error "Unexpected option ${flag}" ;;
  esac
done
shift $((OPTIND - 1))

DUT="$1"

# scripts is an associative array of the form:
# script name -> destination path
declare -A scripts

scripts[start_chroot.sh]=${HOMEDIR}/dev_image/borealis_home_chronos
scripts[start_vtest.sh]=${HOMEDIR}/dev_image/borealis_home_chronos
scripts[stop_vtest.sh]=${HOMEDIR}/dev_image/borealis_home_chronos
scripts[.bashrc]=${HOMEDIR}/dev_image/borealis_home_chronos

if [ "${LINUX}" = true ]; then
  scripts[borealis_chroot.sh]=${HOMEDIR}
else
   scripts[borealis_chroot.sh]=/usr/local/bin
fi

# Copy over scripts the chroot depends on
for key in "${!scripts[@]}"; do
  echo "Copying ${key}"
  quoted_path="${scripts[${key}]@Q}"
  quoted_key="${key@Q}"
  # Silence shellcheck because we do want $quoted_path to expand locally.
  # shellcheck disable=SC2029
  ssh "${DUT}" "mkdir -p ${quoted_path}; \
                cat > ${quoted_path}/${quoted_key}; \
                chmod +x ${quoted_path}/${quoted_key}" < "${SCRIPT_PATH}/${key}"
done

echo "Launching chroot"
if [ "${LINUX}" = true ]; then
  ssh -t "${DUT}" "${HOMEDIR}/borealis_chroot.sh -l ${HOMEDIR}"
else
  ARGS=""
  if [ "${MOUNT_BOREALIS_STATEFUL}" = true ]; then
    ARGS+=" -m"
  fi
  if [ "${EXTERNAL_DISK_IMAGE}" != "" ]; then
    ARGS+=" -x ${EXTERNAL_DISK_IMAGE@Q}"
  fi
  ssh -t "${DUT}" "/usr/local/bin/borealis_chroot.sh ${ARGS}"
fi
