# Copyright 1999-2021 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

LLVM_MAX_SLOT=12
inherit toolchain-funcs

DESCRIPTION="C semantic parser"
HOMEPAGE="https://sparse.wiki.kernel.org/index.php/Main_Page"

SRC_URI="https://www.kernel.org/pub/software/devel/${PN}/dist/${P}.tar.xz"
KEYWORDS="*"

LICENSE="MIT"
SLOT="0"
IUSE="gtk llvm sqlite test xml"
RESTRICT="!test? ( test )"

RDEPEND="gtk? ( x11-libs/gtk+:2 )
	llvm? ( <sys-devel/llvm-$((${LLVM_MAX_SLOT} + 1)):= )
	sqlite? ( dev-db/sqlite:= )
	xml? ( dev-libs/libxml2 )"
DEPEND="${RDEPEND}"
BDEPEND="gtk? ( virtual/pkgconfig )
	xml? ( virtual/pkgconfig )"

llvm_check_deps() {
	has_version "sys-devel/llvm:${LLVM_SLOT}" && \
		has_version -r "sys-devel/llvm:${LLVM_SLOT}"
}

pkg_setup() {
	use llvm && llvm_pkg_setup
}

_emake() {
	# Makefile does not allow for an easy override of flags.
	# Collect them here and override default phases.
	emake \
		CC="$(tc-getCC)" \
		LD="$(tc-getCC)" \
		AR="$(tc-getAR)" \
		CFLAGS="${CFLAGS}" \
		PKG_CONFIG="$(tc-getPKG_CONFIG)" \
		HAVE_GTK=$(usex gtk) \
		HAVE_LLVM=$(usex llvm) \
		HAVE_SQLITE=$(usex sqlite) \
		HAVE_LIBXML=$(usex xml) \
		V=1 \
		PREFIX="${EPREFIX}/usr" \
		"$@"
}

src_compile() {
	_emake
}

src_test() {
	_emake check
}

src_install() {
	_emake DESTDIR="${D}" install

	dodoc FAQ README
}
