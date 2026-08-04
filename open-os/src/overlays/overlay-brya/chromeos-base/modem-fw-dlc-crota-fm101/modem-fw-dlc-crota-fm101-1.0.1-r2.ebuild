# Copyright 2022 The ChromiumOS Authors.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT="d2d95e8af89939f893b1443135497c1f5572aebc"
CROS_WORKON_TREE="776139a53bc86333de8672a51ed7879e75909ac9"
inherit cros-binary cros-workon modem-fw-dlc
# This ebuild only cares about its own FILESDIR and ebuild file, so it tracks
# the canonical empty project.
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="platform/empty-project"

DESCRIPTION="DLC containing the modem firmware for crota_fm101."
HOMEPAGE="http://src.chromium.org"
MIRROR_PATH="gs://chromeos-localmirror/distfiles"
SRC_URI="
	${MIRROR_PATH}/cellular-firmware-fibocom-fm101-brya-19500.0000.00.01.01.52_A54.tar.xz
"

SLOT="0"
KEYWORDS="*"
LICENSE="BSD-Google" #TODO(b/203807072): Change once Fibocom provides a license


# For modem FWs, this value should never increase. See modem-fw-dlc.eclass.
MODEM_FW_DLC_PREALLOC_SIZE_MB="${MODEM_FW_DLC_FM101_DEFAULT_SIZE_3FW}"

#Set the firmware-variants using this DLC.
export MODEM_FW_DLC_FIRMWARE_VARIANT="crota_fm101"

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
