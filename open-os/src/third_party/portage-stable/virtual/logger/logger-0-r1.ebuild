# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Virtual for system loggers"

LICENSE="metapackage"
SLOT="0"
KEYWORDS="*"

RDEPEND="|| (
	app-admin/metalog
	app-admin/rsyslog
	app-admin/socklog
	app-admin/sysklogd
	app-admin/syslog-ng
	sys-apps/busybox[syslog]
	>=sys-apps/systemd-38
)"
