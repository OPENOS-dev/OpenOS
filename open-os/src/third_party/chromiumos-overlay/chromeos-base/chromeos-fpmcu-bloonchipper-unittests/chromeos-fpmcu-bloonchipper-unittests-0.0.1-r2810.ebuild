# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.makefile file.

EAPI=7

CROS_WORKON_COMMIT=("30194971a96d0c1d91ada5d04a84679a3ed45d05" "0dd679081b9c8bfa2583d74e3a17a413709ea362" "c18f94e3b017104284cd541e553472e62e85e526" "3e89a7e8db8139db356b892ca9993172346c80cf" "6910c9d9165801d8827d628cb72eb7ea9dd538c5")
CROS_WORKON_TREE=("4a83749dc7caa0a42caae9edd7f83722293354a8" "d99abee3f825248f344c0638d5f9fcdce114b744" "17878f433c782b4f34ec7180490cdfb371a0fee7" "01e833fa072117aca555a7b978bc96d45036282c" "48aa5a94a57e8768f1189f5079844b71b3199658")
CROS_WORKON_PROJECT=(
	"chromiumos/platform/ec"
	"chromiumos/third_party/cryptoc"
	"external/gitlab.com/libeigen/eigen"
	"external/gob/boringssl/boringssl"
	"external/github.com/google/googletest"
)
CROS_WORKON_LOCALNAME=(
	"platform/ec-legacy"
	"third_party/cryptoc"
	"third_party/eigen3"
	"third_party/boringssl"
	"third_party/googletest"
)
CROS_WORKON_DESTDIR=(
	"${S}/platform/ec-legacy"
	"${S}/third_party/cryptoc"
	"${S}/third_party/eigen3"
	"${S}/third_party/boringssl"
	"${S}/third_party/googletest"
)
CROS_WORKON_EGIT_BRANCH=(
	"ec-legacy"
	"main"
	"upstream/master"
	"upstream/master"
	"main"
)

# shellcheck disable=SC2034
FIRMWARE_EC_BOARD=bloonchipper

inherit cros-fpmcu-unittests cros-workon

DESCRIPTION="ChromeOS fingerprint MCU unittest binaries"
KEYWORDS="*"
