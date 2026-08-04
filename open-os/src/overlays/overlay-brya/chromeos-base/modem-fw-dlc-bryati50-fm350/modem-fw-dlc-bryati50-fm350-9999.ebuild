# Copyright 2022 The Chromium OS Authors. All rights reserved.
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit cros-binary cros-workon modem-fw-dlc
# This ebuild only cares about its own FILESDIR and ebuild file, so it tracks
# the canonical empty project.
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="platform/empty-project"

DESCRIPTION="DLC containing the modem firmware for bryati50_fm350."
HOMEPAGE="http://src.chromium.org"
MIRROR_PATH="gs://chromeos-localmirror/distfiles"
SRC_URI="
	${MIRROR_PATH}/cellular-firmware-fibocom-fm350-81600.0000.00.29.19.16.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-fm350-FM350.C86.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-fm350-DEV_OTA_5001.0001.0000_Default_001.000.000.015.img.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-fm350-OP_OTA_000.037.img.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-fm350-OEM_OTA_6001.0000.001.img.tar.xz
	"

SLOT="0"
KEYWORDS="~*"
LICENSE="BSD-Google" #TODO(b/203807072): Change once Fibocom provides a license


# For modem FWs, this value should never increase. See modem-fw-dlc.eclass.
MODEM_FW_DLC_PREALLOC_SIZE_MB="${MODEM_FW_DLC_FM350_DEFAULT_SIZE_3FW}"

#Set the firmware-variants using this DLC.
export MODEM_FW_DLC_FIRMWARE_VARIANT="bryati50_fm350"

src_unpack() {
	cros-workon_src_unpack
	# Because we are not pulling in any sources, we need to have an empty
	# source directory to satisfy the build success.
	S="${WORKDIR}"
	default
}

src_install() {
	modem_fw_dlc_src_install
}
