# Copyright 2019 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

ZEPHYR_PROGRAM="bloonchipper"
export FIRMWARE_RELEASE_REPLACE_RO="yes"

CROS_WORKON_PROJECT=(
	"chromiumos/third_party/zephyrproject"
	"external/gob/boringssl/boringssl"
	"external/github.com/google/googletest"
	"chromiumos/platform/ec"
)

CROS_WORKON_LOCALNAME=(
	"../platform/release-firmware/fpmcu-bloonchipper/zephyrproject"
	"boringssl"
	"googletest"
	"../platform/release-firmware/fpmcu-bloonchipper/ec"
)

CROS_WORKON_DESTDIR=(
	"${S}/zephyrproject"
	"${S}/zephyrproject/modules/boringssl"
	"${S}/zephyrproject/modules/googletest"
	"${S}/zephyrproject/modules/ec"
)

CROS_WORKON_EGIT_BRANCH=(
	"firmware-fpmcu-bloonchipper-release"
	"upstream/master"
	"main"
	"firmware-fpmcu-bloonchipper-release"
)

inherit cros-workon cros-zephyr-fpmcu

HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/ec/+/HEAD/README.md"
LICENSE="BSD-Google"
DESCRIPTION="Zephyr based Fingerprint MCU firmware for ${ZEPHYR_PROGRAM}."
KEYWORDS="~*"
