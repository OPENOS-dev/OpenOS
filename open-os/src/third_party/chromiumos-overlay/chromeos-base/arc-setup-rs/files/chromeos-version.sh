#!/bin/sh
#
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Assumes the first 'version =' line in the Cargo.toml is the version for the
# crate.
sed '/^ *version *= *"\(.*\)"/!d; s//\1/; q' "$1/arc/setup/rust/Cargo.toml"
