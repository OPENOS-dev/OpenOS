# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

XORG_TARBALL_SUFFIX="xz"
inherit xorg-3

DESCRIPTION="XKB keyboard description compiler"
KEYWORDS="*"

RDEPEND="
	>=x11-libs/libX11-1.6.9
	x11-libs/libxkbfile"
DEPEND="${RDEPEND}
	x11-base/xorg-proto"
BDEPEND="sys-devel/bison"
