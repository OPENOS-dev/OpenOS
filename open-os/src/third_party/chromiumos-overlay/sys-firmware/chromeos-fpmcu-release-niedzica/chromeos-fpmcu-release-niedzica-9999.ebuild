# Copyright 2026 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

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
KEYWORDS="~*"
