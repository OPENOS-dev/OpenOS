# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

# Please bump with dev-build/libtool.

inherit multilib-minimal flag-o-matic

MY_P="libtool-${PV}"

DESCRIPTION="A shared library tool for developers"
HOMEPAGE="https://www.gnu.org/software/libtool/"
SRC_URI="mirror://gnu/libtool/${MY_P}.tar.xz"
S="${WORKDIR}"/${MY_P}/libltdl

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="*"
IUSE="static-libs"
# libltdl doesn't have a testsuite.

BDEPEND="app-arch/xz-utils"

multilib_src_configure() {
	append-lfs-flags
	ECONF_SOURCE="${S}" \
	econf \
		--enable-ltdl-install \
		$(use_enable static-libs static)
}

multilib_src_install() {
	emake DESTDIR="${D}" install

	# While the libltdl.la file is not used directly, the m4 ltdl logic
	# keys off of its existence when searching for ltdl support. # bug #293921
	#use static-libs || find "${D}" -name libltdl.la -delete
}
