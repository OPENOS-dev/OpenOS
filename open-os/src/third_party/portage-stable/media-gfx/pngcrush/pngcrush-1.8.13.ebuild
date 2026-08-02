# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit toolchain-funcs

DESCRIPTION="Portable Network Graphics (PNG) optimizing utility"
HOMEPAGE="https://pmt.sourceforge.io/pngcrush/"
SRC_URI="https://downloads.sourceforge.net/pmt/${P}-nolib.tar.xz"

LICENSE="pngcrush"
SLOT="0"
KEYWORDS="*"

RDEPEND="media-libs/libpng:0=
	sys-libs/zlib:="
DEPEND="${RDEPEND}"
BDEPEND="app-arch/xz-utils"

PATCHES=(
	"${FILESDIR}"/${PN}-1.8.13-adler32_check.patch
)

DOCS=( ChangeLog.html )

S="${WORKDIR}"/${P}-nolib

src_compile() {
	emake \
		CC="$(tc-getCC)" \
		LD="$(tc-getCC)" \
		CFLAGS="${CFLAGS} ${CPPFLAGS} -Wall" \
		LDFLAGS="${LDFLAGS}"
}

src_install() {
	dobin ${PN}
	einstalldocs
}
