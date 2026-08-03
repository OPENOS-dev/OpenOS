# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit unpacker

DESCRIPTION="Metadata files describing Intel PMT data decoding"
REPO_NAME="Intel-PMT"
REPO_HASH="82edcb69c754595a5c6d90cf6c1c9938f92180d0"
HOMEPAGE="https://github.com/intel/${REPO_NAME}"
SRC_URI="https://github.com/intel/${REPO_NAME}/archive/${REPO_HASH}.tar.gz -> ${P}.tar.gz"

LICENSE="Apache-2.0"
SLOT="0"
KEYWORDS="-* amd64"
IUSE="intel-pmt-mtl"

S="${WORKDIR}/${REPO_NAME}-${REPO_HASH}"

src_install() {
	insinto /usr/local/share/libpmt/metadata
	# Install metadata for each enabled platform but nothing more.
	if use intel-pmt-mtl; then
		doins -r "${S}"/xml/MTL
		doins "${S}"/xml/pmt.xml
	fi
}
