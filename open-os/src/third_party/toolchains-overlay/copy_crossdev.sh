#!/bin/bash
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Copies the crossdev config into the toolchains overlay.

set -eu -o pipefail

if [[ ! -d /etc/portage ]]; then
  echo "/etc/portage was not found, are you in the chroot?" >&2
  exit 1
fi

PROFILE="$(realpath --relative-to . "$(dirname "$0")")/profiles/base"
mkdir -p "${PROFILE}"

for SRC in \
    /etc/portage/profile/package.use.mask \
    /etc/portage/profile/package.use.force \
    /etc/portage/package.accept_keywords \
    /etc/portage/package.use
do
  DEST="${PROFILE}/$(basename "${SRC}")"
  mkdir -p "${DEST}"
  while read -r FILE; do
    sort "${FILE}" > "${DEST}/${FILE##*/}"
  done < <(find "${SRC}" -iname 'cross-*' -type f)
done

# Profiles don't support the `env` feature, but they do support `bashrc` so we
# use that instead.
mkdir -p "${PROFILE}/bashrc" "${PROFILE}/package.bashrc"

find /etc/portage/package.env -iname 'cross-*' \
  -exec cp -vft "${PROFILE}/package.bashrc" '{}' +

while read -r TOOLCHAIN; do
  TOOLCHAIN_ROOT="/etc/portage/env/${TOOLCHAIN}"

  while read -r FILE; do
    TARGET="${PROFILE}/bashrc/${TOOLCHAIN}/${FILE}"
    mkdir -p "${TARGET%/*}"
    # We only want the env files to apply before src_setup because some ebuilds
    # modify some of the multilib variables in src_setup and we don't want to
    # override them.
    (
      echo 'if [[ "${EBUILD_PHASE}" == "setup" ]]; then'
      cat "${TOOLCHAIN_ROOT}/${FILE}"
      echo 'fi'
    ) >"${TARGET}"
  done < <(find "${TOOLCHAIN_ROOT}" -iname '*.conf' -type f -printf '%P\n')

done < <(find /etc/portage/env -maxdepth 1 -iname 'cross-*' -printf '%P\n')
