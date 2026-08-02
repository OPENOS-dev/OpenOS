# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

inherit cros-toolchain-funcs

DESCRIPTION="Firmware updater for Parade Technologies Touch devices"
HOMEPAGE="https://github.com/ParadeTechnologies/paradetech-updater"
SRC_URI="https://github.com/ParadeTechnologies/paradetech-updater/archive/v${PV}.tar.gz -> ${P}.tar.gz"


LICENSE="MIT"
SLOT="0"
KEYWORDS="*"
IUSE=""

src_configure() {
	tc-export CC
}

src_install() {
	dosbin bin/ptupdater
}
