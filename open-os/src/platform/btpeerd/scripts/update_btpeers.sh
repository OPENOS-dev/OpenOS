#!/bin/bash

# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Replaces the existing btpeerd version on the btpeer(s) with the current
# local state of this repository to allow for quick testing of changes to
# btpeerd.
#
# The btpeers' ImageUUID in its build info is updated with a `-needs-reinstall`
# suffix so that auto-repair re-images it, undoing these changes.
#
# For development purposes only. Normal deployments should still be bundled
# within images.
#
# Specify a dut hostname to update all of its btpeers or btpeer hostname(s).
# Assumes anything ending in "-host\d+" to be a dut hostname, and its btpeers to
# be <dut_hostname>-btpeerN for N [1,4]. Attempts to ssh each host before
# any updates to validate access and hostname resolution, any hosts that failed
# are skipped. Fails if btpeerd does not exist already on the host.
#
# Usage:
# ./update_btpeers.sh <dut_or_btpeer_host1> [...<dut_or_btpeer_host1>]

set -e

SCRIPT_DIR="$(dirname "$(realpath -e "${BASH_SOURCE[0]}")")"
PROJECT_DIR="$(realpath -e "${SCRIPT_DIR}/..")"
BTPEER_PATH_TO_BTPEERD_DIR="/etc/chromiumos/src/platform/btpeerd/"
BTPEER_PATH_TO_BUILD_INFO_FILE="/etc/chromiumos/raspios_cros_btpeer_build_info.json"
DIRTY_IMAGE_UUID_SUFFIX="-needs-reinstall"
CHROMIUMOS_CONFIG_SRC_LOCAL_DIR="${PROJECT_DIR}/../../config/go"
CHROMIUMOS_CONFIG_SRC_BTPEER_DIR="/etc/chromiumos/src/config/go"

# Collect hosts from script args.
HOSTS_TO_CHECK=()
for HOST in "$@"; do
  # Remove crossk prefix.
  HOST="${HOST#crossk-}"
  if [[ "${HOST}" =~ host[0-9]+$ ]]; then
    # Likely a DUT host, so add all btpeers.
    echo "Assuming host '${HOST}' is a dut hostname, will try to update its 4 btpeers"
    HOSTS_TO_CHECK+=(
      "${HOST}-btpeer1"
      "${HOST}-btpeer2"
      "${HOST}-btpeer3"
      "${HOST}-btpeer4"
    )
  else
    HOSTS_TO_CHECK+=("${HOST}")
  fi
done
if [ "${#HOSTS_TO_CHECK[@]}" -eq 0 ]; then
  echo "ERROR: At least one dut or btpeer hostname is required"
  exit 1
fi
echo "Got ${#HOSTS_TO_CHECK[@]} btpeer hostnames from CLI args"

# Validate hosts as btpeers.
BTPEER_HOSTS_TO_UPDATE=()
for HOST in "${HOSTS_TO_CHECK[@]}"; do
  echo "Checking if host '${HOST}' is a valid host to update"
  set +e
  ssh -q "${HOST}" "test -d ${BTPEER_PATH_TO_BTPEERD_DIR}"
  SSH_RET_CODE=$?
  set -e
  if [ "${SSH_RET_CODE}" -eq 255 ]; then
    echo "WARNING: Failed to connect to host '${HOST}', assuming host does not exist and will not update it"
  elif [ "${SSH_RET_CODE}" -eq 1 ]; then
    echo "ERROR: Invalid host '${HOST}': missing btpeerd install dir ${BTPEER_PATH_TO_BTPEERD_DIR}"
    exit 1
  elif [ "${SSH_RET_CODE}" -ne 0 ]; then
    echo "ERROR: Failed to verify host '${HOST}' has btpeerd install dir ${BTPEER_PATH_TO_BTPEERD_DIR}"
    exit 2
  else
    echo "Successfully verified host '${HOST}' as a valid btpeer, will update it"
    BTPEER_HOSTS_TO_UPDATE+=("${HOST}")
  fi
done
if [ "${#BTPEER_HOSTS_TO_UPDATE[@]}" -eq 0 ]; then
  echo "ERROR: No valid btpeer hosts to update"
  exit 1
fi
echo "Successfully identified ${#BTPEER_HOSTS_TO_UPDATE[@]} btpeer hosts to update"

# Update btpeerd on btpeers.
LOG_DIVIDER="=============================================================================="
for HOST in "${BTPEER_HOSTS_TO_UPDATE[@]}"; do
  echo -e "\n${LOG_DIVIDER}"
  echo "Updating btpeerd on host '${HOST}'"

  echo "Marking btpeer image as dirty"
  BUILD_INFO_JSON=$(ssh -q "${HOST}" "cat '${BTPEER_PATH_TO_BUILD_INFO_FILE}'")
  IMAGE_UUID=$(jq -r ."image_uuid" <<< "${BUILD_INFO_JSON}")
  if [ "${#IMAGE_UUID}" -ne 36 ]; then
    echo "Btpeer image already marked as dirty with image UUID '${IMAGE_UUID}', skipping step"
  else
    IMAGE_UUID="${IMAGE_UUID}${DIRTY_IMAGE_UUID_SUFFIX}"
    BUILD_INFO_JSON=$(jq '."image_uuid" = $val' --arg val "${IMAGE_UUID}" <<< "${BUILD_INFO_JSON}")
    echo "${BUILD_INFO_JSON}" | ssh -q "${HOST}" "cat - > '${BTPEER_PATH_TO_BUILD_INFO_FILE}'"
    echo "Successfully marked btpeer image as dirty with image UUID '${IMAGE_UUID}'"
  fi

  echo "Stopping btpeerd service"
  ssh -q "${HOST}" "systemctl stop btpeerd.service"

  echo "Copying source files to btpeer"
  ssh -q "${HOST}" "mkdir -p '${CHROMIUMOS_CONFIG_SRC_BTPEER_DIR}'"
  rsync -a "${CHROMIUMOS_CONFIG_SRC_LOCAL_DIR}"/* \
  "${HOST}:${CHROMIUMOS_CONFIG_SRC_BTPEER_DIR}"
  rsync -av "${PROJECT_DIR}/" "${HOST}:${BTPEER_PATH_TO_BTPEERD_DIR}/" \
  --exclude .git --exclude .idea --exclude .vscode \
  --exclude go/bin --exclude go/pkg

  echo "Building btpeerd on btpeer"
  ssh "${HOST}" "${BTPEER_PATH_TO_BTPEERD_DIR}/scripts/build.sh"

  echo "Starting btpeerd service"
  ssh -q "${HOST}" "systemctl start btpeerd.service"

  echo "Successfully updated btpeerd on host '${HOST}'"
done

# Print summary.
echo -e "\n${LOG_DIVIDER}\n${LOG_DIVIDER}"
echo "Successfully updated btpeerd on ${#BTPEER_HOSTS_TO_UPDATE[@]} hosts:"
for HOST in "${BTPEER_HOSTS_TO_UPDATE[@]}"; do
  echo "${HOST}"
done
