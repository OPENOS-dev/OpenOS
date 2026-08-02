#!/bin/bash

# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -xe

# <cros>/src/third_party/chromiumos-overlay/dev-libs/opencl-kernel-profiler/files
SCRIPT_DIR=$(dirname "$(realpath "${BASH_SOURCE[0]}")")
CROS_PATH="$(realpath "${SCRIPT_DIR}"/../../../../../..)"
CLKP_PATH="$(realpath "${SCRIPT_DIR}"/../)"

SHA1="$1"
[ "${#SHA1}" -eq 40 ]

CLKP_ARCHIVE_NAME="opencl-kernel-profiler-${SHA1}"
CLKP_ARCHIVE="${CLKP_ARCHIVE_NAME}.zip"
CLKP_ARCHIVE_CACHED="${CROS_PATH}/.cache/distfiles/${CLKP_ARCHIVE}"
[ -f "${CLKP_ARCHIVE_CACHED}" ] \
    || wget "https://github.com/rjodinchr/opencl-kernel-profiler/archive/${SHA1}.zip" -O "${CLKP_ARCHIVE_CACHED}"

GS_PATH="gs://chromeos-localmirror/distfiles"
if [ "$(gsutil ls "${GS_PATH}"/"${CLKP_ARCHIVE}" 2>/dev/null | wc -l)" -eq 0 ]
then
    read -rp 'gsutil cp -a public-read '"${CLKP_ARCHIVE_CACHED}"' '"${GS_PATH}"'? (y/N) ' COPY_ARCHIVE_TO_GS
    if [ "${COPY_ARCHIVE_TO_GS}" == "y" ]
    then
        gsutil cp -a public-read "${CLKP_ARCHIVE_CACHED}" "${GS_PATH}"
    fi
fi

CLKP_EBUILD_PATH="${CLKP_PATH}/opencl-kernel-profiler-0.0.1.ebuild"
sed -i 's|^CLKP_ARCHIVE_NAME=.*$|CLKP_ARCHIVE_NAME="'"${CLKP_ARCHIVE_NAME}"'"|' "${CLKP_EBUILD_PATH}"
ebuild "${CLKP_EBUILD_PATH}" manifest

CURRENT_REVISION=$(find "${CLKP_PATH}" -name "opencl-kernel-profiler-0.0.1-r*" | sed 's|.*opencl-kernel-profiler-0.0.1-r\(.*\).ebuild|\1|')
NEXT_REVISION=$((CURRENT_REVISION + 1 ))
read -rp "git mv r${CURRENT_REVISION} r${NEXT_REVISION}? (y/N) " UPDATE_REVISION
if [ "${UPDATE_REVISION}" == "y" ]
then
    git mv "${CLKP_PATH}/opencl-kernel-profiler-0.0.1-r${CURRENT_REVISION}.ebuild" "${CLKP_PATH}/opencl-kernel-profiler-0.0.1-r${NEXT_REVISION}.ebuild"
fi
