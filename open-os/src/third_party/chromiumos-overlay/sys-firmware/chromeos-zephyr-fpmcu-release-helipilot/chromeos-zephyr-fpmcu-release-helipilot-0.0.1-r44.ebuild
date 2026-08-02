# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT=("d6d9bcf9d2d9120765b42bfe2fdadf4a5c6e710d" "3e89a7e8db8139db356b892ca9993172346c80cf" "6910c9d9165801d8827d628cb72eb7ea9dd538c5" "dbef49a26b9554be0cbbb5089c915fbfca6f6baf")
CROS_WORKON_TREE=("cea2e57c39c3a826cf7411d5f52755a6f89edd8b" "01e833fa072117aca555a7b978bc96d45036282c" "48aa5a94a57e8768f1189f5079844b71b3199658" "d57a94c5cbf27a5153443faa88428fd199e13b66")
ZEPHYR_PROGRAM="helipilot"
export FIRMWARE_RELEASE_REPLACE_RO="yes"

CROS_WORKON_PROJECT=(
	"chromiumos/third_party/zephyrproject"
	"external/gob/boringssl/boringssl"
	"external/github.com/google/googletest"
	"chromiumos/platform/ec"
)

CROS_WORKON_LOCALNAME=(
	"../platform/release-firmware/fpmcu-helipilot/zephyrproject"
	"boringssl"
	"googletest"
	"../platform/release-firmware/fpmcu-helipilot/ec"
)

CROS_WORKON_DESTDIR=(
	"${S}/zephyrproject"
	"${S}/zephyrproject/modules/boringssl"
	"${S}/zephyrproject/modules/googletest"
	"${S}/zephyrproject/modules/ec"
)

CROS_WORKON_EGIT_BRANCH=(
	"firmware-fpmcu-helipilot-release"
	"upstream/master"
	"main"
	"firmware-fpmcu-helipilot-release"
)

inherit cros-workon cros-zephyr-fpmcu

DESCRIPTION="Zephyr based Fingerprint MCU firmware for ${ZEPHYR_PROGRAM}."
KEYWORDS="*"
