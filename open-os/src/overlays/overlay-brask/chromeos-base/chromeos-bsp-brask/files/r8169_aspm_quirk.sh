#!/bin/bash

# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

enable_state() {
  local file_path="$1"

  # Check if file exists
  if [[ ! -f "${file_path}" ]]; then
    logger -p ERR "r8169-aspm-qurik: file doesn't exist '${file_path}'"
    exit 1
  fi

  # Check if file is writeable
  if [[ ! -w "${file_path}" ]]; then
    logger -p ERR "r8169-aspm-qurik: file isn't writeable '${file_path}'"
    exit 1
  fi

  if ! echo "1" > "${file_path}"; then
    logger -p ERR "r8169-aspm-qurik: Failed to write to file '${file_path}'"
    exit 1
  fi
}

main() {
  logger -p INFO "r8169-aspm-quirk: started"

  if [[ $# -ne 1 ]]; then
    logger -p ERR "r8169-aspm-quirk: wrong number of arguments; exiting"
    exit 1
  fi

  #the setters of the attributes also take care of the parent/children dev
  enable_state "/sys/${1}/link/l1_aspm"
  enable_state "/sys/${1}/link/l1_1_aspm"
  enable_state "/sys/${1}/link/l1_1_pcipm"
  enable_state "/sys/${1}/link/l1_2_pcipm"

  logger -p INFO "r8169-aspm-quirk: sucess"
}

main "$@"
