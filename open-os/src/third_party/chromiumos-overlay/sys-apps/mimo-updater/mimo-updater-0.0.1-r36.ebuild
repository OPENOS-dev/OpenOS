# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="b2167efa4d3d78a965e843560b53c804095d6222"
CROS_WORKON_TREE="83de62a8714d508777dc23054dcf11be70ba58ea"
CROS_WORKON_PROJECT="chromiumos/third_party/mimo-updater"

inherit cros-debug cros-workon libchrome udev user

DESCRIPTION="A tool to interact with a Mimo device from Chromium OS."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/third_party/mimo-updater"

LICENSE="BSD-Google"
KEYWORDS="*"

DEPEND="
	chromeos-base/libbrillo:=
	virtual/libusb:1
	virtual/libudev:0="

RDEPEND="${DEPEND}"

src_configure() {
	# See crbug/1078297
	cros-debug-add-NDEBUG
	default
}

src_install() {
	dosbin mimo-updater
	udev_dorules conf/90-displaylink-usb.rules
}

pkg_preinst() {
	enewuser cfm-firmware-updaters
	enewgroup cfm-firmware-updaters
}
