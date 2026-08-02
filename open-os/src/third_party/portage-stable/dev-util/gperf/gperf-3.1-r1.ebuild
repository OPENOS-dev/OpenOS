# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="A perfect hash function generator"
HOMEPAGE="https://www.gnu.org/software/gperf/"
SRC_URI="mirror://gnu/gperf/${P}.tar.gz"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="*"

PATCHES=(
	"${FILESDIR}"/${P}-strncmp-decl-mismatch.patch
	"${FILESDIR}"/${P}-clang-16-wregister.patch
)

src_prepare() {
	sed -i \
		-e "/^CPPFLAGS /s:=:+=:" \
		*/Makefile.in || die #444078

	default
}
