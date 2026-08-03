# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

# Maintainers should:
# 1. Join the "Gentoo" project at https://dev.gnupg.org/project/view/27/
# 2. Subscribe to release tasks like https://dev.gnupg.org/T6159
# (find the one for the current release then subscribe to it +
# any subsequent ones linked within so you're covered for a while.)

VERIFY_SIG_OPENPGP_KEY_PATH="${BROOT}"/usr/share/openpgp-keys/gnupg.asc
inherit flag-o-matic toolchain-funcs verify-sig

DESCRIPTION="X.509 and CMS (PKCS#7) library"
HOMEPAGE="https://www.gnupg.org/related_software/libksba"
SRC_URI="mirror://gnupg/${PN}/${P}.tar.bz2"
SRC_URI+=" verify-sig? ( mirror://gnupg/${PN}/${P}.tar.bz2.sig )"

LICENSE="LGPL-3+ GPL-2+ GPL-3"
SLOT="0"
KEYWORDS="*"
IUSE="static-libs"

RDEPEND=">=dev-libs/libgpg-error-1.8"
DEPEND="${RDEPEND}"
BDEPEND="
	sys-devel/bison
	verify-sig? ( sec-keys/openpgp-keys-gnupg )
"

PATCHES=(
	"${FILESDIR}"/${PN}-1.6.0-no-fgrep-ksba-config.patch
)

src_configure() {
	# ChromeOS: add large file support (https://bugs.gentoo.org/902479)
	append-lfs-flags

	export CC_FOR_BUILD="$(tc-getBUILD_CC)"

	local myeconfargs=(
		$(use_enable static-libs static)

		GPG_ERROR_CONFIG="${ESYSROOT}/usr/bin/${CHOST}-gpg-error-config"
		LIBGCRYPT_CONFIG="${ESYSROOT}/usr/bin/${CHOST}-libgcrypt-config"
	)

	econf "${myeconfargs[@]}"
}

src_install() {
	default

	# People need to use ksba-config for --cflags and --libs
	find "${ED}" -type f -name '*.la' -delete || die
}
