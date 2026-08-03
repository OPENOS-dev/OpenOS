#!/bin/bash

# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -xe

# <cros>/src/third_party/chromiumos-overlay/media-libs/clpeak/files
SCRIPT_DIR=$(dirname "$(realpath "${BASH_SOURCE[0]}")")
CROS_PATH="$(realpath "${SCRIPT_DIR}"/../../../../../..)"

TAG="$1"
[ $# -eq 1 ]

CLPEAK_ARCHIVE_NAME="clpeak-${TAG}"
CLPEAK_ARCHIVE="${CLPEAK_ARCHIVE_NAME}.zip"
CLPEAK_ARCHIVE_CACHED="${CROS_PATH}/.cache/distfiles/${CLPEAK_ARCHIVE}"
[ -f "${CLPEAK_ARCHIVE_CACHED}" ] \
    || wget "https://github.com/krrishnarraj/clpeak/archive/refs/tags/${TAG}.zip" -O "${CLPEAK_ARCHIVE_CACHED}"

GS_PATH="gs://chromeos-localmirror/distfiles"
if [ "$(gsutil ls "${GS_PATH}"/"${CLPEAK_ARCHIVE}" 2>/dev/null | wc -l)" -eq 0 ]
then
    read -rp 'gsutil cp -a public-read '"${CLPEAK_ARCHIVE_CACHED}"' '"${GS_PATH}"'? (y/N) ' COPY_ARCHIVE_TO_GS
    if [ "${COPY_ARCHIVE_TO_GS}" == "y" ]
    then
        gsutil cp -a public-read "${CLPEAK_ARCHIVE_CACHED}" "${GS_PATH}"
    fi
fi
