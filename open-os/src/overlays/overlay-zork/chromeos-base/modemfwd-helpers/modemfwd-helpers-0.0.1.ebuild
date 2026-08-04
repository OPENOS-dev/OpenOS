# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

inherit cros-cellular udev user

DESCRIPTION="Chrome OS Modem Update Helpers (zork)"
HOMEPAGE="http://src.chromium.org"
MIRROR_PATH="gs://chromeos-localmirror/distfiles"
SRC_URI="
	${MIRROR_PATH}/cellular-firmware-fibocom-l850-18500.5001.00.05.27.12_Secureboot.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-l850-18500.5001.00.05.27.16_Secureboot.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-l850-OEM_cust.6003.02.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-l850-zork-carriers_OEM_6003-r10.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-nl668-19006.1000.00.02.79.62-1.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-nl668-A04.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-nl668-OEM_cust.6002.01.tar.xz
"
LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"

S="${WORKDIR}"

src_install() {
	cellular_domanifest "${FILESDIR}/helper_manifest.textproto"

	# TODO(ejcaruso): remove these after b/71870985 is fixed and we can
	# use MBIM commands to reset the modem instead of toggling GPIOs
	insinto /etc/init/
	doins "${FILESDIR}"/modemfwd-{helpers,mount}.conf

	udev_dorules "${FILESDIR}/94-usb-modem-gpio.rules"

	# Generate firmware patches.
	cellular_generate_firmware_patches

	# Generate and install squashfs with firmware files and manifest.
	cellular_create_squashfs_bundle
}

pkg_preinst() {
	enewgroup gpio
}
