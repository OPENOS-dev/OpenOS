# Copyright 1999-2025 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

XORG_MULTILIB=yes
XORG_TARBALL_SUFFIX="xz"
inherit xorg-3

DESCRIPTION="X.Org Inter-Client Exchange library"
KEYWORDS="*"

RDEPEND="
	elibc_glibc? (
		|| ( >=sys-libs/glibc-2.36 dev-libs/libbsd[${MULTILIB_USEDEP}] )
	)
"
DEPEND="${RDEPEND}
	x11-base/xorg-proto
	x11-libs/xtrans
"

XORG_CONFIGURE_OPTIONS=(
	--enable-ipv6
	--disable-docs
	--disable-specs
	--without-fop
)
