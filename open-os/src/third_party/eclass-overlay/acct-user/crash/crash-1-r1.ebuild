# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

inherit acct-user

DESCRIPTION="Crash reporter daemon."

# NB: These settings are ignored in CrOS for now.
# See the files in profiles/base/accounts/ instead.

ACCT_USER_ID=20137
ACCT_USER_GROUPS=( crash-access chronos-access crash crash-user-access fbpreprocessor-user-access )
