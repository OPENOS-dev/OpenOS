# Copyright 1999-2017 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

EAPI=6

inherit flag-o-matic pam linux-info autotools

DESCRIPTION="eCryptfs userspace utilities"
HOMEPAGE="https://launchpad.net/ecryptfs"
SRC_URI="https://launchpad.net/ecryptfs/trunk/${PV}/+download/${PN}_${PV}.orig.tar.gz"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="*"
IUSE="doc gpg gtk nls openssl pam pkcs11 suid tpm"

RDEPEND=">=sys-apps/keyutils-1.0:=
	>=dev-libs/libgcrypt-1.2.0:0
	dev-libs/nss
	gpg? ( app-crypt/gpgme )
	gtk? ( x11-libs/gtk+:2 )
	openssl? ( >=dev-libs/openssl-0.9.7:= )
	pam? ( sys-libs/pam )
	pkcs11? (
		>=dev-libs/openssl-0.9.7:=
		>=dev-libs/pkcs11-helper-1.04
	)
	tpm? ( app-crypt/trousers )"
DEPEND="${RDEPEND}
	virtual/pkgconfig
	sys-devel/gettext
	>=dev-util/intltool-0.41.0"

src_prepare() {
	default
	# Regenerate configure script to avoid hard coded paths e.g. /usr/lib that break
	# cross-compilation on ARM32.
	eautoreconf
}

src_configure() {
	append-cppflags -D_FILE_OFFSET_BITS=64

	econf \
		--enable-nss \
		--with-pamdir=$(getpam_mod_dir) \
		$(use_enable doc docs) \
		$(use_enable gpg) \
		$(use_enable gtk gui) \
		$(use_enable nls) \
		$(use_enable openssl) \
		$(use_enable pam) \
		$(use_enable pkcs11 pkcs11-helper) \
		--disable-pywrap \
		$(use_enable tpm tspi)
}

src_install(){
	emake DESTDIR="${D}" install

	use suid && fperms u+s /sbin/mount.ecryptfs_private

	find "${ED}" -name '*.la' -exec rm -f '{}' + || die
}

pkg_postinst() {
	if use suid; then
		ewarn
		ewarn "You have chosen to install ${PN} with the binary setuid root. This"
		ewarn "means that if there are any undetected vulnerabilities in the binary,"
		ewarn "then local users may be able to gain root access on your machine."
		ewarn
	fi
}
