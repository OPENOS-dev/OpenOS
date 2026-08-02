#!/bin/bash
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -eu

. "$(dirname "$0")/docker-deps.sh"
sudo apt-get install -y libguestfs-tools zstd python3-guestfs
