# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Virtual to select between different tmpfiles.d handlers"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	!prefix-guest? (
		|| (
			sys-apps/systemd-utils[tmpfiles]
			sys-apps/systemd-tmpfiles
			sys-apps/systemd
		)
	)"
