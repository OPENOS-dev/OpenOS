# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

XORG_MULTILIB=yes
XORG_TARBALL_SUFFIX="xz"
inherit xorg-3 meson-multilib

DESCRIPTION="Library providing generic access to the PCI bus and devices"
KEYWORDS="*"
IUSE="zlib"

DEPEND="
	zlib? (	>=sys-libs/zlib-1.2.8-r1:=[${MULTILIB_USEDEP}] )"
RDEPEND="${DEPEND}
	sys-apps/hwdata"

src_prepare() {
	default
}

multilib_src_configure() {
	local emesonargs=(
		-Dpci-ids="${EPREFIX}"/usr/share/hwdata
		$(meson_feature zlib)
	)
	meson_src_configure
}
