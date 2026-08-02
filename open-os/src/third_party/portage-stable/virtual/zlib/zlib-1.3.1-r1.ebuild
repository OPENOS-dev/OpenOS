# Copyright 2025 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit multilib-build

DESCRIPTION="Virtual for libz.so providers"
SLOT="0/1"
KEYWORDS="*"
IUSE="static-libs"

RDEPEND="
	|| (
		>=sys-libs/zlib-1.3.1[${MULTILIB_USEDEP},static-libs?]
		sys-libs/zlib-ng[${MULTILIB_USEDEP},compat,static-libs(-)?]
	)
"
