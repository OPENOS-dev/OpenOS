# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit multilib-minimal toolchain-funcs usr-ldscript

DESCRIPTION="zstd fast compression library"
HOMEPAGE="https://facebook.github.io/zstd/"
SRC_URI="https://github.com/facebook/${PN}/archive/v${PV}.tar.gz -> ${P}.tar.gz"

LICENSE="|| ( BSD GPL-2 )"
SLOT="0/1"
KEYWORDS="*"
IUSE="lz4 static-libs"

RDEPEND="
	app-arch/xz-utils
	sys-libs/zlib
	lz4? ( app-arch/lz4 )
"
DEPEND="${RDEPEND}"

src_prepare() {
	default
	multilib_copy_sources
}

mymake() {
	emake \
		CC="$(tc-getCC)" \
		CXX="$(tc-getCXX)" \
		AR="$(tc-getAR)" \
		PREFIX="${EPREFIX}/usr" \
		LIBDIR="${EPREFIX}/usr/$(get_libdir)" \
		V=1 \
		"${@}"
}

multilib_src_compile() {
	local libzstd_targets=( libzstd{,.a}-mt )

	mymake -C lib ${libzstd_targets[@]} libzstd.pc

	if multilib_is_native_abi ; then
		mymake HAVE_LZ4="$(usex lz4 1 0)" zstd

		mymake -C contrib/pzstd
	fi
}

multilib_src_install() {
	mymake -C lib DESTDIR="${D}" install

	if multilib_is_native_abi ; then
		mymake -C programs DESTDIR="${D}" install
		gen_usr_ldscript -a zstd

		mymake -C contrib/pzstd DESTDIR="${D}" install
	fi
}

multilib_src_install_all() {
	einstalldocs

	if ! use static-libs; then
		find "${ED}" -name "*.a" -delete || die
	fi
}
