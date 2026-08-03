# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

inherit cros-toolchain-funcs

DESCRIPTION="FocalTech HID Device for Firmware Update"
HOMEPAGE="https://github.com/waynehuang2022/ftphid_ezupg_ap"
SRC_URI="https://github.com/waynehuang2022/ftphid_ezupg_ap/archive/refs/tags/V${PV}.tar.gz -> ${P}.tar.gz"

LICENSE="Apache-2.0"
SLOT="0"
KEYWORDS="*"
IUSE=""

src_configure() {
	tc-export CXX
}

src_install() {
	dosbin ftphid_ezupg_ap/bin/ftphid_ezupg_ap
}
