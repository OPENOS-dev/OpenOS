#!/bin/bash

# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -xe

# <cros>/src/third_party/chromiumos-overlay/dev-libs/vulkan-shader-profiler/files
SCRIPT_DIR=$(dirname "$(realpath "${BASH_SOURCE[0]}")")
CROS_PATH="$(realpath "${SCRIPT_DIR}"/../../../../../..)"
VKSP_PATH="$(realpath "${SCRIPT_DIR}"/../)"

SHA1="$1"
[ "${#SHA1}" -eq 40 ]

VKSP_ARCHIVE_NAME="vulkan-shader-profiler-${SHA1}"
VKSP_ARCHIVE="${VKSP_ARCHIVE_NAME}.zip"
VKSP_ARCHIVE_CACHED="${CROS_PATH}/.cache/distfiles/${VKSP_ARCHIVE}"
[ -f "${VKSP_ARCHIVE_CACHED}" ] \
    || wget "https://github.com/rjodinchr/vulkan-shader-profiler/archive/${SHA1}.zip" -O "${VKSP_ARCHIVE_CACHED}"

GS_PATH="gs://chromeos-localmirror/distfiles"
if [ "$(gsutil ls "${GS_PATH}"/"${VKSP_ARCHIVE}" 2>/dev/null | wc -l)" -eq 0 ]
then
    read -rp 'gsutil cp -a public-read '"${VKSP_ARCHIVE_CACHED}"' '"${GS_PATH}"'? (y/N) ' COPY_ARCHIVE_TO_GS
    if [ "${COPY_ARCHIVE_TO_GS}" == "y" ]
    then
        gsutil cp -a public-read "${VKSP_ARCHIVE_CACHED}" "${GS_PATH}"
    fi
fi

VKSP_EBUILD_PATH="${VKSP_PATH}/vulkan-shader-profiler-0.0.1.ebuild"
sed -i 's|^VKSP_ARCHIVE_NAME=.*$|VKSP_ARCHIVE_NAME="'"${VKSP_ARCHIVE_NAME}"'"|' "${VKSP_EBUILD_PATH}"
ebuild "${VKSP_EBUILD_PATH}" manifest

CURRENT_REVISION=$(find "${VKSP_PATH}" -name "vulkan-shader-profiler-0.0.1-r*" | sed 's|.*vulkan-shader-profiler-0.0.1-r\(.*\).ebuild|\1|')
NEXT_REVISION=$((CURRENT_REVISION + 1 ))
read -rp "git mv r${CURRENT_REVISION} r${NEXT_REVISION}? (y/N) " UPDATE_REVISION
if [ "${UPDATE_REVISION}" == "y" ]
then
    git mv "${VKSP_PATH}/vulkan-shader-profiler-0.0.1-r${CURRENT_REVISION}.ebuild" "${VKSP_PATH}/vulkan-shader-profiler-0.0.1-r${NEXT_REVISION}.ebuild"
fi
