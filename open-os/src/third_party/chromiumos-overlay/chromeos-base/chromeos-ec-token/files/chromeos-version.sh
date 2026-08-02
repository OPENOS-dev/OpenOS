#!/bin/bash
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

SCRIPT=$(realpath "$0")
PKG_ROOT=$(dirname "${SCRIPT}")/../
VERSION_PIN="${PKG_ROOT}/VERSION-PIN"

cat "${VERSION_PIN}"
