# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Libcamera tuning files for mtkisp7"
SRC_URI="gs://chromeos-localmirror/distfiles/${P}.tar.bz2"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="-* arm arm64"

S="${WORKDIR}"

src_install() {
	local CONFIG_DIR="/etc/camera"
	insinto "${CONFIG_DIR}"
	doins -r "back_settings"
}
