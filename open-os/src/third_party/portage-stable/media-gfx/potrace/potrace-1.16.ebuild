# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit toolchain-funcs flag-o-matic

DESCRIPTION="Transforming bitmaps into vector graphics"
HOMEPAGE="http://potrace.sourceforge.net/"
SRC_URI="http://potrace.sourceforge.net/download/${PV}/${P}.tar.gz"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="*"
IUSE="metric"

RDEPEND="sys-libs/zlib:="
DEPEND="${RDEPEND}"

DOCS=( AUTHORS ChangeLog NEWS README )

src_configure() {
	# ChromeOS: enabled large file support (https://bugs.gentoo.org/898960)
	append-lfs-flags

	tc-export CC # bug 610098
	local myeconfargs=(
		--disable-static
		--enable-zlib
		--with-libpotrace
		$(use_enable metric a4)
		$(use_enable metric)
	)
	econf "${myeconfargs[@]}"
}

src_install() {
	default

	# no static archives
	find "${ED}" -name '*.la' -delete || die
}
