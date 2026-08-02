# Copyright 2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit acct-user

DESCRIPTION="User for Meet Camera stack communication Daemon"

# NB: These settings are ignored in CrOS for now.
# See the files in profiles/base/accounts/ instead.

ACCT_USER_ID=20223
