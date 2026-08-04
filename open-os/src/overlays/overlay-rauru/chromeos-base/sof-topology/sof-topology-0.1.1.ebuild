# Copyright 2024 The Chromium OS Authors.
# Distributed under the terms of the GNU General Public License v2.

EAPI=7

DESCRIPTION="MT8196 SOF topology"
SRC_URI="gs://chromeos-localmirror/distfiles/${PN}-rauru-${PV}.tar.gz"

LICENSE="SOF"
SLOT="0"
KEYWORDS="*"
IUSE=""

DEPEND=""
RDEPEND="${DEPEND}"
BDEPEND=""

S=${WORKDIR}/${PN}-rauru-${PV}

src_install() {
	insinto /lib/firmware/mediatek/sof-tplg
	doins ./*.tplg
}
