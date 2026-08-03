# Copyright 1999-2026 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

inherit multilib-minimal verify-sig

DESCRIPTION="Small C library to run an HTTP server as part of another application"
HOMEPAGE="https://www.gnu.org/software/libmicrohttpd/"
SRC_URI="mirror://gnu/${PN}/${P}.tar.gz
         verify-sig? ( mirror://gnu/${PN}/${P}.tar.gz.sig )"

LICENSE="LGPL-2.1+"
SLOT="0/12"
KEYWORDS="*"
IUSE="debug +epoll ssl static-libs test +thread-names verify-sig"
RESTRICT="!test? ( test )"

KEYRING_VER=201906

RDEPEND="ssl? ( >net-libs/gnutls-2.12.20:=[${MULTILIB_USEDEP}] )"
# libcurl and the curl binary are used during tests on CHOST
DEPEND="${RDEPEND}
	test? ( net-misc/curl[ssl?] )"
BDEPEND="ssl? ( virtual/pkgconfig )
         verify-sig? ( ~sec-keys/openpgp-keys-libmicrohttpd-${KEYRING_VER} )"

VERIFY_SIG_OPENPGP_KEY_PATH=/usr/share/openpgp-keys/libmicrohttpd-${KEYRING_VER}.asc

DOCS=( AUTHORS NEWS COPYING README ChangeLog )

multilib_src_configure() {
	ECONF_SOURCE="${S}" \
	econf \
		--enable-shared \
		$(use_enable static-libs static) \
		--enable-bauth \
		--enable-dauth \
		--disable-examples \
		--enable-messages \
		--enable-postprocessor \
		--enable-httpupgrade \
		--disable-tools \
		--disable-experimental \
		--disable-heavy-tests \
		$(use_enable debug asserts) \
		$(use_enable thread-names) \
		$(use_enable epoll) \
		$(use_enable test curl) \
		$(use_enable ssl https) \
		$(use_with ssl gnutls)
}

multilib_src_install_all() {
	default

	if ! use static-libs; then
		find "${ED}" -name '*.la' -delete || die
	fi
}
