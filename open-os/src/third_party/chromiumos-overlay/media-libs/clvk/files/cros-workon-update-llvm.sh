#!/bin/bash

# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -xe

# <cros>/src/third_party/chromiumos-overlay/media-libs/clvk/files
SCRIPT_DIR=$(dirname "$(realpath "${BASH_SOURCE[0]}")")
CROS_PATH="$(realpath "${SCRIPT_DIR}"/../../../../../..)"

LLVM_SHA1="$(grep -A 1 "third_party/llvm" "${CROS_PATH}"/src/third_party/clspv/deps.json | tail -n 1 | sed 's|^.*: "\(.*\)"$|\1|')"
[ "${#LLVM_SHA1}" -eq 40 ]

[ $# -eq 1 ]
LIBCLC_BINARIES_REPO=$1

LIBCLC_FOLDER="libclc-clspv-${LLVM_SHA1}"
LIBCLC_ARCHIVE="${LIBCLC_FOLDER}.zip"
LIBCLC_ARCHIVE_CACHED="${CROS_PATH}/.cache/distfiles/${LIBCLC_ARCHIVE}"

[ -f "${LIBCLC_ARCHIVE_CACHED}" ] \
    || (cd "${LIBCLC_BINARIES_REPO}" && zip "${LIBCLC_ARCHIVE_CACHED}" "spir--/libclc.bc" "spir64--/libclc.bc")

LLVM_FOLDER="llvm-project-${LLVM_SHA1}"
LLVM_ARCHIVE="${LLVM_FOLDER}.zip"
LLVM_ARCHIVE_CACHED="${CROS_PATH}/.cache/distfiles/${LLVM_ARCHIVE}"
[ -f "${LLVM_ARCHIVE_CACHED}" ] \
    || wget "https://github.com/llvm/llvm-project/archive/${LLVM_SHA1}.zip" -O "${LLVM_ARCHIVE_CACHED}"

GS_PATH="gs://chromeos-localmirror/distfiles"
if [ "$(gcloud storage ls "${GS_PATH}"/"${LIBCLC_ARCHIVE}" 2>/dev/null | wc -l)" -eq 0 ]
then
    read -rp 'gcloud storage cp --predefined-acl=publicRead '"${LIBCLC_ARCHIVE_CACHED}"' '"${GS_PATH}"'? (y/N) ' COPY_ARCHIVE_TO_GS
    if [ "${COPY_ARCHIVE_TO_GS}" == "y" ]
    then
        gcloud storage cp --predefined-acl=publicRead "${LIBCLC_ARCHIVE_CACHED}" "${GS_PATH}"
    fi
fi
if [ "$(gcloud storage ls "${GS_PATH}"/"${LLVM_ARCHIVE}" 2>/dev/null | wc -l)" -eq 0 ]
then
    read -rp 'gcloud storage cp --predefined-acl=publicRead '"${LLVM_ARCHIVE_CACHED}"' '"${GS_PATH}"'? (y/N) ' COPY_ARCHIVE_TO_GS
    if [ "${COPY_ARCHIVE_TO_GS}" == "y" ]
    then
        gcloud storage cp --predefined-acl=publicRead "${LLVM_ARCHIVE_CACHED}" "${GS_PATH}"
    fi
fi

CLVK_EBUILD_PATH_CROS="src/third_party/chromiumos-overlay/media-libs/clvk/clvk-9999.ebuild"
CLVK_EBUILD_PATH="${CROS_PATH}/${CLVK_EBUILD_PATH_CROS}"
sed -i 's|^LLVM_FOLDER="llvm-project-.*"$|LLVM_FOLDER="'"${LLVM_FOLDER}"'"|' "${CLVK_EBUILD_PATH}"
sed -i 's|^LIBCLC_FOLDER="libclc-clspv-.*"$|LIBCLC_FOLDER="'"${LIBCLC_FOLDER}"'"|' "${CLVK_EBUILD_PATH}"
cros_sdk -- ebuild "../../${CLVK_EBUILD_PATH_CROS}" manifest

read -rp 'sudo rm -rf /build/*/tmp/portage/media-libs/clvk-9999? (y/N) ' CLEAN_CROS_WORKON
if [ "${CLEAN_CROS_WORKON}" == "y" ]
then
    sudo rm -rf /build/*/tmp/portage/media-libs/clvk-9999
fi
