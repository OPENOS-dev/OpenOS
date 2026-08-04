# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Realtek updater libraries"
SRC_URI="gs://chromeos-localmirror-private/distfiles/${P}.tar.xz"
RESTRICT="mirror"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"

S="${WORKDIR}/${P}"

src_install() {
	into "/usr/local/"
	dobin "command-line-updater"
	insinto "/usr/share/cambrionix"
	doins "CambrionixFirmware-1.88.7-un.enfir"
}
