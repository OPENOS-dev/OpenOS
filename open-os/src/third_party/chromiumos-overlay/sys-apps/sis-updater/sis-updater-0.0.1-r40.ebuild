# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="eaeefa2739d3043eaa0e76d3859caa8244148649"
CROS_WORKON_TREE="2ae3733b59766588f6701143a164dae773537914"
CROS_WORKON_PROJECT="chromiumos/third_party/sis-updater"

inherit cros-workon cros-common.mk libchrome udev user

DESCRIPTION="A tool to update SiS firmware on Mimo from Chromium OS."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/third_party/sis-updater"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"

DEPEND="chromeos-base/libbrillo:="

RDEPEND="${DEPEND}"

src_install() {
	dosbin "${OUT}/sis-updater"
	udev_dorules conf/99-sis-usb.rules
}

pkg_preinst() {
	enewuser cfm-firmware-updaters
	enewgroup cfm-firmware-updaters
}
