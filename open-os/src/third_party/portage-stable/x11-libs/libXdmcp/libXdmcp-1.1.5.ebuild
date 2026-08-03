# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

XORG_DOC=doc
XORG_MULTILIB=yes
XORG_TARBALL_SUFFIX="xz"
inherit xorg-3

DESCRIPTION="X.Org X Display Manager Control Protocol library"

KEYWORDS="*"

RDEPEND="
	elibc_glibc? (
		|| ( >=sys-libs/glibc-2.36 dev-libs/libbsd[${MULTILIB_USEDEP}] )
	)
	!elibc_glibc? (
		dev-libs/libbsd[${MULTILIB_USEDEP}]
	)
"
DEPEND="${RDEPEND}
	x11-base/xorg-proto"

src_configure() {
	local XORG_CONFIGURE_OPTIONS=(
		$(use_enable doc docs)
		$(use_with doc xmlto)
		--without-fop
	)
	xorg-3_src_configure
}
