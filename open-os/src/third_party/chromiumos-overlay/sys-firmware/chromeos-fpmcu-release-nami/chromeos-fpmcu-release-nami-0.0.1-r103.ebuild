# Copyright 2019 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT=("00eff714edff772871ec3a568ae800df50c37c75" "0dd679081b9c8bfa2583d74e3a17a413709ea362" "3e89a7e8db8139db356b892ca9993172346c80cf" "6910c9d9165801d8827d628cb72eb7ea9dd538c5")
CROS_WORKON_TREE=("29efb35ae672f6fd07ad8c8b703c0e0e0f692768" "d99abee3f825248f344c0638d5f9fcdce114b744" "01e833fa072117aca555a7b978bc96d45036282c" "48aa5a94a57e8768f1189f5079844b71b3199658")
FIRMWARE_EC_BOARD="nami_fp"
FIRMWARE_EC_RELEASE_REPLACE_RO="yes"

CROS_WORKON_PROJECT=(
	"chromiumos/platform/ec"
	"chromiumos/third_party/cryptoc"
	"external/gob/boringssl/boringssl"
	"external/github.com/google/googletest"
)

CROS_WORKON_LOCALNAME=(
	"../platform/release-firmware/fpmcu-dartmonkey/ec"
	"cryptoc"
	"boringssl"
	"googletest"
)

CROS_WORKON_DESTDIR=(
	"${S}/platform/ec-legacy"
	"${S}/third_party/cryptoc"
	"${S}/third_party/boringssl"
	"${S}/third_party/googletest"
)

CROS_WORKON_EGIT_BRANCH=(
	"firmware-fpmcu-dartmonkey-release"
	" " # default value is space. See "cros-workon.eclass" for details.
	" " # default value is space. See "cros-workon.eclass" for details.
	" " # default value is space. See "cros-workon.eclass" for details.
)

inherit cros-workon cros-ec-release

HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/ec/+/ec-legacy/README.md"
LICENSE="BSD-Google"
KEYWORDS="*"
