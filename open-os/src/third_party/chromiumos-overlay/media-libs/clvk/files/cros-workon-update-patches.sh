#!/bin/bash

# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -xe

[ $# -eq 2 ]
REPO=$1
BASE=$2

SCRIPT_DIR=$(dirname "$(realpath "${BASH_SOURCE[0]}")")
THIRD_PARTY_DIR="${SCRIPT_DIR}/../../../.."
REPO_DIR="${THIRD_PARTY_DIR}/${REPO}"
REV_PARSED_BASE="$(git -C "${REPO_DIR}" rev-parse "${BASE}")"

i=0
while [[ "$(git -C "${REPO_DIR}" rev-parse HEAD~"${i}")" != "${REV_PARSED_BASE}" ]]; do
    git -C "${REPO_DIR}" diff HEAD~"${i}" ^HEAD~"$((i + 1))" \
        --src-prefix="a/${REPO}/" --dst-prefix="b/${REPO}/" \
        > "${SCRIPT_DIR}/$(git -C "${REPO_DIR}" log --format=%s -n 1 HEAD~"${i}")"
    i=$((i + 1))
done
