# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Library for handling paper characteristics"
HOMEPAGE="https://github.com/rrthomas/libpaper"
SRC_URI="https://github.com/rrthomas/libpaper/releases/download/v${PV}/${P}.tar.gz"

# See README.
# paperspecs is public-domain
LICENSE="LGPL-2.1+ GPL-3+ public-domain"
SLOT="0/$(ver_cut 1)"
KEYWORDS="*"

src_configure() {
	econf --enable-relocatable
}

src_install() {
	default

	find "${ED}" -type f -name '*.la' -delete || die
}
