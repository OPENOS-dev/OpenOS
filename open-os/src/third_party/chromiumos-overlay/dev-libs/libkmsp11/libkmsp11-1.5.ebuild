# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

DESCRIPTION="Ebuild to install Google PKSC11 connector library."
HOMEPAGE="https://github.com/GoogleCloudPlatform/kms-integrations"
LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="-* amd64"
IUSE="verify-sig"

RDEPEND=""
BDEPEND="verify-sig? ( dev-libs/openssl )"

SRC_URI="https://github.com/GoogleCloudPlatform/kms-integrations/releases/download/pkcs11-v${PV}/${P}-linux-amd64.tar.gz"

S="${WORKDIR}/${P}-linux-amd64"

src_install() {
	local pubkey
	local lib

	# Validate library signature.
	pubkey="${FILESDIR}/pkcs11-release-signing-key.pem"
	lib="libkmsp11.so"
	if use verify-sig; then
		openssl dgst -sha384 -verify "${pubkey}" \
			--signature "${lib}.sig" "${lib}" || die "Failed to validate library signature"
	fi
	dolib.so "${lib}"
}
