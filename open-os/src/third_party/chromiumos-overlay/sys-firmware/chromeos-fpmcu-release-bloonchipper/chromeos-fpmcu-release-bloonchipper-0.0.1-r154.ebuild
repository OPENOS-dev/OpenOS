# Copyright 2019 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT=("9a5fde56f958be84931b6c17e697b0370f1f2374" "3e89a7e8db8139db356b892ca9993172346c80cf" "6910c9d9165801d8827d628cb72eb7ea9dd538c5" "5a78800e724d701dd9b62e24b11bf2c0d48c2e96")
CROS_WORKON_TREE=("02411d91cc2cd8228016a91dc7a07c1f1f47d814" "01e833fa072117aca555a7b978bc96d45036282c" "48aa5a94a57e8768f1189f5079844b71b3199658" "7e2fc44fbc0d64d7885f522b3f13284a0a616c96")
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
KEYWORDS="*"
