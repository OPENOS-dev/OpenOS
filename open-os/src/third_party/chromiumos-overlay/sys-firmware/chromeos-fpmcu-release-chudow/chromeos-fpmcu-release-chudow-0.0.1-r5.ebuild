# Copyright 2026 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT=("7324909b19be056c3d20cd05b9bebe9658ee508c" "3e89a7e8db8139db356b892ca9993172346c80cf" "6910c9d9165801d8827d628cb72eb7ea9dd538c5" "c54d53fe43ff38b87b661c2c86c33f1ad21a84c7")
CROS_WORKON_TREE=("948def7d6c5e9cb8e7b44d13a25334593b44e375" "01e833fa072117aca555a7b978bc96d45036282c" "48aa5a94a57e8768f1189f5079844b71b3199658" "d29ef4d07385ab7cd2af7bfe15ffe4e2899cafe0")
ZEPHYR_PROGRAM="chudow"
# TODO(b/491245587): Set FIRMWARE_EC_RELEASE_REPLACE_RO to "yes"
export FIRMWARE_RELEASE_REPLACE_RO="no"

CROS_WORKON_PROJECT=(
	"chromiumos/third_party/zephyrproject"
	"external/gob/boringssl/boringssl"
	"external/github.com/google/googletest"
	"chromiumos/platform/ec"
)

CROS_WORKON_LOCALNAME=(
	"../platform/release-firmware/fpmcu-chudow/zephyrproject"
	"boringssl"
	"googletest"
	"../platform/release-firmware/fpmcu-chudow/ec"
)

CROS_WORKON_DESTDIR=(
	"${S}/zephyrproject"
	"${S}/zephyrproject/modules/boringssl"
	"${S}/zephyrproject/modules/googletest"
	"${S}/zephyrproject/modules/ec"
)

CROS_WORKON_EGIT_BRANCH=(
	"firmware-fpmcu-chudow-release"
	"upstream/master"
	"main"
	"firmware-fpmcu-chudow-release"
)

inherit cros-workon cros-zephyr-fpmcu

DESCRIPTION="Zephyr based Fingerprint MCU firmware for ${ZEPHYR_PROGRAM}."
KEYWORDS="*"
