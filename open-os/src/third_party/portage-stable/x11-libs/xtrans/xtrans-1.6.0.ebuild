# Copyright 1999-2025 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

XORG_PACKAGE_NAME="lib${PN}"
# this package just installs some .c and .h files, no libraries
XORG_DOC=doc
XORG_TARBALL_SUFFIX="xz"
inherit xorg-3

DESCRIPTION="X.Org xtrans library"
KEYWORDS="*"

src_configure() {
	local XORG_CONFIGURE_OPTIONS=(
		$(use_enable doc docs)
		$(use_with doc xmlto)
		--without-fop
	)
	xorg-3_src_configure
}
