# Copyright 2020 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit acct-user

DESCRIPTION="User for ssh"

# NB: These settings are ignored in CrOS for now.
# See the files in profiles/base/accounts/ instead.

ACCT_USER_ID=22
ACCT_USER_HOME=/var/empty
ACCT_USER_HOME_OWNER=root:root
ACCT_USER_GROUPS=( sshd )

acct-user_add_deps
