# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

inherit acct-user

DESCRIPTION="user for firmware update utilities that use /dev/drm_dp_aux* and /dev/i2c-*"

# NB: These settings are ignored in CrOS for now.
# See the files in profiles/base/accounts/ instead.

ACCT_USER_ID=20145
ACCT_USER_GROUPS=( i2c drm_dp_aux fwupdate-drm_dp_aux-i2c )
