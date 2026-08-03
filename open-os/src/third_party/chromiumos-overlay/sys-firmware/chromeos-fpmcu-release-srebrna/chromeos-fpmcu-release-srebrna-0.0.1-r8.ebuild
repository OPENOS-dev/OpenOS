# Copyright 2026 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT=("037bed37602a54a00433495c298e2565b6d388ba" "3e89a7e8db8139db356b892ca9993172346c80cf" "6910c9d9165801d8827d628cb72eb7ea9dd538c5" "874b5a2fcea502a8a6656879e9fd63885c31c022")
CROS_WORKON_TREE=("306d9f001d1d96b4088a3fad939a036bdbce3ebe" "01e833fa072117aca555a7b978bc96d45036282c" "48aa5a94a57e8768f1189f5079844b71b3199658" "28f71918a805b459878ebfe37ec7093a5db98974")
ZEPHYR_PROGRAM="srebrna"
# TODO(b/489229299): Set FIRMWARE_EC_RELEASE_REPLACE_RO to "yes"
export FIRMWARE_RELEASE_REPLACE_RO="no"

CROS_WORKON_PROJECT=(
	"chromiumos/third_party/zephyrproject"
	"external/gob/boringssl/boringssl"
	"external/github.com/google/googletest"
	"chromiumos/platform/ec"
)

CROS_WORKON_LOCALNAME=(
	"../platform/release-firmware/fpmcu-srebrna/zephyrproject"
	"boringssl"
	"googletest"
	"../platform/release-firmware/fpmcu-srebrna/ec"
)

CROS_WORKON_DESTDIR=(
	"${S}/zephyrproject"
	"${S}/zephyrproject/modules/boringssl"
	"${S}/zephyrproject/modules/googletest"
	"${S}/zephyrproject/modules/ec"
)

CROS_WORKON_EGIT_BRANCH=(
	"firmware-fpmcu-srebrna-release"
	"upstream/master"
	"main"
	"firmware-fpmcu-srebrna-release"
)

inherit cros-workon cros-zephyr-fpmcu

DESCRIPTION="Zephyr based Fingerprint MCU firmware for ${ZEPHYR_PROGRAM}."
KEYWORDS="*"
