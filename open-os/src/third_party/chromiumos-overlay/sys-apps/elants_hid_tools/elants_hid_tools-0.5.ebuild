# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

inherit cros-toolchain-funcs

DESCRIPTION="Elan Touchscreen HID Tools for Firmware Update"
HOMEPAGE="https://github.com/PaulLiang01043/elants_hid_tools"
SRC_URI="http://storage.googleapis.com/chromeos-localmirror/distfiles/elants_hid_tools-${PV}.tar.gz"

LICENSE="Apache-2.0"
SLOT="0"
KEYWORDS="*"
IUSE=""

src_configure() {
	tc-export CC
}

src_install() {
	newsbin hid_read_fwid/bin/hid_read_fwid elants_hid_read_fwid
	newsbin hid_iap/bin/hid_iap elants_hid_iap
}
