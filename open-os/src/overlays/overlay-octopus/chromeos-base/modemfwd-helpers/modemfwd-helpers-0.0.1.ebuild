# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

inherit cros-cellular udev user

DESCRIPTION="Chrome OS Modem Update Helpers (octopus)"
HOMEPAGE="http://src.chromium.org"
MIRROR_PATH="gs://chromeos-localmirror/distfiles"
SRC_URI="
	${MIRROR_PATH}/cellular-firmware-fibocom-l850-18500.5001.00.02.24.09_Secureboot.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-l850-18500.5001.00.03.25.18_Secureboot.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-l850-18500.5001.00.04.26.06_6000.05_Secureboot.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-l850-18500.5001.00.04.26.06_Secureboot.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-l850-18500.5001.00.05.27.12_Secureboot.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-l850-18500.5001.00.07.29.08_Secureboot.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-l850-OEM_cust.6000.03.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-l850-OEM_cust.6000.05.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-l850-OEM_cust.6005.13.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-l850-octopus-carriers_OEM_6000-r24.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-l850-octopus-carriers_OEM_6005-r16.tar.xz
	"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE="internal"

S="${WORKDIR}"
DEPEND="
	internal? ( chromeos-base/fibocom-firmware )
"
RDEPEND="${DEPEND}"

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
