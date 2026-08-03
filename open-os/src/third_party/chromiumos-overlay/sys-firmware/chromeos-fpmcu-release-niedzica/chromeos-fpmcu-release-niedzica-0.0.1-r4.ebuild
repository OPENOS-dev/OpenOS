# Copyright 2026 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT=("6dbc595795a0e5aa4972de60a063f0bd636b1315" "3e89a7e8db8139db356b892ca9993172346c80cf" "6910c9d9165801d8827d628cb72eb7ea9dd538c5" "216c99840e8b3a1c5493d693cdf30731d57ad936")
CROS_WORKON_TREE=("948def7d6c5e9cb8e7b44d13a25334593b44e375" "01e833fa072117aca555a7b978bc96d45036282c" "48aa5a94a57e8768f1189f5079844b71b3199658" "0172c0ce8f66dfedec5b7e7969323f33d6c3edb2")
ZEPHYR_PROGRAM="niedzica"
# TODO(b/491311325): Set FIRMWARE_EC_RELEASE_REPLACE_RO to "yes"
export FIRMWARE_RELEASE_REPLACE_RO="no"

CROS_WORKON_PROJECT=(
	"chromiumos/third_party/zephyrproject"
	"external/gob/boringssl/boringssl"
	"external/github.com/google/googletest"
	"chromiumos/platform/ec"
)

CROS_WORKON_LOCALNAME=(
	"../platform/release-firmware/fpmcu-niedzica/zephyrproject"
	"boringssl"
	"googletest"
	"../platform/release-firmware/fpmcu-niedzica/ec"
)

CROS_WORKON_DESTDIR=(
	"${S}/zephyrproject"
	"${S}/zephyrproject/modules/boringssl"
	"${S}/zephyrproject/modules/googletest"
	"${S}/zephyrproject/modules/ec"
)

CROS_WORKON_EGIT_BRANCH=(
	"firmware-fpmcu-niedzica-release"
	"upstream/master"
	"main"
	"firmware-fpmcu-niedzica-release"
)

inherit cros-workon cros-zephyr-fpmcu

DESCRIPTION="Zephyr based Fingerprint MCU firmware for ${ZEPHYR_PROGRAM}."
KEYWORDS="*"
