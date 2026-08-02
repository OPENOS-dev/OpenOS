#!/usr/bin/env bash
# Copyright 2021 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd )"

"${DIR}"/build_tast_binaries.py \
  --stage=deqp-build \
  --output=deqp \
  --image-name=deqp-build \
  --root=scratch/VK-GL-CTS/build
