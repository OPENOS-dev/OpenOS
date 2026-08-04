# Copyright 2023 The Chromium OS Authors.
# Distributed under the terms of the GNU General Public License v2.

EAPI=7

DESCRIPTION="MT8188 SOF topology"
SRC_URI="gs://chromeos-localmirror/distfiles/${PN}-geralt-${PV}.tar.gz"

LICENSE="SOF"
SLOT="0"
KEYWORDS="*"
IUSE=""

DEPEND=""
RDEPEND="${DEPEND}"
BDEPEND=""

S=${WORKDIR}/${PN}-geralt-${PV}

src_install() {
	insinto /lib/firmware/mediatek/sof-tplg
	doins sof-mt8188.tplg
}
