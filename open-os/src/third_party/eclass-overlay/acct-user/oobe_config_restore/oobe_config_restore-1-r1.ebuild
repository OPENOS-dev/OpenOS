# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

inherit acct-user

DESCRIPTION="oobe config restore utility"

# NB: These settings are ignored in CrOS for now.
# See the files in profiles/base/accounts/ instead.

ACCT_USER_ID=20121
ACCT_USER_GROUPS=( tss pstore-access oobe_config oobe_config_restore )
