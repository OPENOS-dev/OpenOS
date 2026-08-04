# Copyright 2024 The ChromiumOS Authors.
# Distributed under the terms of the GNU General Public License v2.

EAPI=7

DESCRIPTION="MT8196 SOF firmware binary"
SRC_URI="gs://chromeos-localmirror/distfiles/${PN}-mt8196-${PV}.tar.xz"

LICENSE="SOF"
SLOT="0"
KEYWORDS="*"
IUSE=""

DEPEND=""
RDEPEND="
	media-libs/mt8196-tas2563-dsm-param-navi
"
BDEPEND=""

S=${WORKDIR}/${PN}-mt8196-${PV}

src_install() {
	insinto /lib/firmware/mediatek/sof
	doins sof-mt8196.ri
}
