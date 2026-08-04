# Copyright 2023 The Chromium OS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit cros-binary cros-workon modem-fw-dlc
# This ebuild only cares about its own FILESDIR and ebuild file, so it tracks
# the canonical empty project.
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="platform/empty-project"

DESCRIPTION="DLC containing the modem firmware for rex."
HOMEPAGE="http://src.chromium.org"
MIRROR_PATH="gs://chromeos-localmirror/distfiles"
SRC_URI="
	${MIRROR_PATH}/cellular-firmware-fibocom-fm101-19500.0000.00.01.02.80.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-fm101-A96.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-fm101-DEV_OTA_6000.0001.0002.0000_00.01.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-fm101-download_agent_rex_fm101-19500.0000.00.01.02.80-A96-1.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-fm101-OEM_OTA_6000.0000.004.tar.xz
	${MIRROR_PATH}/cellular-firmware-fibocom-fm101-OP_OTA_002.077.tar.xz"


SLOT="0"
KEYWORDS="~*"
LICENSE="GPL-2 LGPL-2.1 BSD-Fibocom Apache-2.0 openssl"


# For modem FWs, this value should never increase. See modem-fw-dlc.eclass.
MODEM_FW_DLC_PREALLOC_SIZE_MB="${MODEM_FW_DLC_FM101_DEFAULT_SIZE_3FW}"

#Set the firmware-variants using this DLC.
MODEM_FW_DLC_FIRMWARE_VARIANT="rex_fm101"

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
