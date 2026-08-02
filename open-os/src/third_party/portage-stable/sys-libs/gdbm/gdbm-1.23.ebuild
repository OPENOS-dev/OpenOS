# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

VERIFY_SIG_OPENPGP_KEY_PATH="${BROOT}"/usr/share/openpgp-keys/gdbm.asc
inherit libtool multilib-minimal verify-sig

DESCRIPTION="Standard GNU database libraries"
HOMEPAGE="https://www.gnu.org/software/gdbm/"
SRC_URI="mirror://gnu/gdbm/${P}.tar.gz"
SRC_URI+=" verify-sig? ( mirror://gnu/gdbm/${P}.tar.gz.sig )"

LICENSE="GPL-3"
SLOT="0/6" # libgdbm.so version
KEYWORDS="*"
IUSE="+berkdb nls +readline static-libs"

DEPEND="readline? ( sys-libs/readline:=[${MULTILIB_USEDEP}] )"
RDEPEND="${DEPEND}"
BDEPEND="verify-sig? ( sec-keys/openpgp-keys-gdbm )"

src_prepare() {
	default
	elibtoolize
}

multilib_src_configure() {
	# gdbm doesn't appear to use either of these libraries
	export ac_cv_lib_dbm_main=no ac_cv_lib_ndbm_main=no

	local myeconfargs=(
		--includedir="${EPREFIX}"/usr/include/gdbm
		$(use_enable berkdb libgdbm-compat)
		$(use_enable nls)
		$(use_enable static-libs static)
		$(use_with readline)
	)

	ECONF_SOURCE="${S}" econf "${myeconfargs[@]}"
}

multilib_src_install_all() {
	einstalldocs

	if ! use static-libs ; then
		find "${ED}" -name '*.la' -delete || die
	fi

	mv "${ED}"/usr/include/gdbm/gdbm.h "${ED}"/usr/include/ || die
}
