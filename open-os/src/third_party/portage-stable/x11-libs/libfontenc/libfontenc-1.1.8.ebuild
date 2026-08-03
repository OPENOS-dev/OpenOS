# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

XORG_TARBALL_SUFFIX="xz"
inherit xorg-3

DESCRIPTION="X.Org fontenc library"

KEYWORDS="*"

RDEPEND="sys-libs/zlib"
DEPEND="${RDEPEND}
	x11-base/xorg-proto"

XORG_CONFIGURE_OPTIONS=(
	--with-encodingsdir="${EPREFIX}/usr/share/fonts/encodings"
)
