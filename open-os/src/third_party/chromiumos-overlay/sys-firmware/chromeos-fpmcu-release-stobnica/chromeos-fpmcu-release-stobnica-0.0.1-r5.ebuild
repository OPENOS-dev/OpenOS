# Copyright 2026 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT=("d6330d28bbd7be7d36986dcdb4d97839f1ee8d0f" "3e89a7e8db8139db356b892ca9993172346c80cf" "6910c9d9165801d8827d628cb72eb7ea9dd538c5" "a91921b8424f561e7adcb498a89262f234ef60c4")
CROS_WORKON_TREE=("631980d552e47f513b69a960b9c9b7031e305e4d" "01e833fa072117aca555a7b978bc96d45036282c" "48aa5a94a57e8768f1189f5079844b71b3199658" "31ffa36801a749941d9101b2a75efdc6d478ee8e")
ZEPHYR_PROGRAM="stobnica"
# TODO(b/489244228): Set FIRMWARE_EC_RELEASE_REPLACE_RO to "yes"
export FIRMWARE_RELEASE_REPLACE_RO="no"

CROS_WORKON_PROJECT=(
	"chromiumos/third_party/zephyrproject"
	"external/gob/boringssl/boringssl"
	"external/github.com/google/googletest"
	"chromiumos/platform/ec"
)

CROS_WORKON_LOCALNAME=(
	"../platform/release-firmware/fpmcu-stobnica/zephyrproject"
	"boringssl"
	"googletest"
	"../platform/release-firmware/fpmcu-stobnica/ec"
)

CROS_WORKON_DESTDIR=(
	"${S}/zephyrproject"
	"${S}/zephyrproject/modules/boringssl"
	"${S}/zephyrproject/modules/googletest"
	"${S}/zephyrproject/modules/ec"
)

CROS_WORKON_EGIT_BRANCH=(
	"firmware-fpmcu-stobnica-release"
	"upstream/master"
	"main"
	"firmware-fpmcu-stobnica-release"
)

inherit cros-workon cros-zephyr-fpmcu

DESCRIPTION="Zephyr based Fingerprint MCU firmware for ${ZEPHYR_PROGRAM}."
KEYWORDS="*"
