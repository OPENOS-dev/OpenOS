# Copyright 2025 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Chromeos SCP firmware payload for Skywalker."

RESTRICT="strip"
LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="-* arm arm64"

SRC_URI="gs://chromeos-localmirror/distfiles/${P}.tbz2"

S="${WORKDIR}/${P}"

src_install() {
	insinto /lib/firmware/mediatek/mt8189/
	doins scp.img
}
