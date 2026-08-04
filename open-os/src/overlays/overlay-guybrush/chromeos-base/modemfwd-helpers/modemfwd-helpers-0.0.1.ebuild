# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

inherit cros-binary cros-cellular udev user

DESCRIPTION="Chrome OS Modem Update Helpers (guybrush)"
HOMEPAGE="http://src.chromium.org"
MIRROR_PATH="gs://chromeos-localmirror/distfiles"
SRC_URI="
	${MIRROR_PATH}/cellular-firmware-fibocom-fm350-sample.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-l850-18500.5001.00.05.27.12_Secureboot.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-l850-18500.5001.00.05.27.16_Secureboot.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-l850-OEM_cust.6001.04.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-l850-guybrush-carriers_OEM_6001-r7.tar.xz"

RESTRICT="mirror"
LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"

S="${WORKDIR}"
DEPEND="
	chromeos-base/fibocom-firmware
	chromeos-base/modem-fw-dlc-guybrush-fm350
	chromeos-base/modem-fw-dlc-guybrush360-l850
	chromeos-base/modem-fw-dlc-nipperkin
"
RDEPEND="${DEPEND}"

src_install() {
	cellular_domanifest "${FILESDIR}/helper_manifest.textproto"

	# TODO(b/71870985): remove these after b/71870985 is fixed and we can
	# use MBIM commands to reset the modem instead of toggling GPIOs
	insinto /etc/init/
	doins "${FILESDIR}/modemfwd-helpers.conf"

	udev_dorules "${FILESDIR}/94-usb-modem-gpio.rules"

	cellular_dofirmware "${FILESDIR}/firmware_manifest.prototxt"
	# cellular_dofirmware cannot handle this case yet (multiple directories/modems)
	insinto "$(_cellular_get_firmwaredir)/l850"
	for f in cellular-firmware-fibocom-l850-*; do
		doins -r "${f}"/*
	done

	insinto "$(_cellular_get_firmwaredir)/fm350"
	for f in cellular-firmware-fibocom-fm350-*; do
		doins -r "${f}"/*
	done
}

pkg_preinst() {
	enewgroup gpio
}
