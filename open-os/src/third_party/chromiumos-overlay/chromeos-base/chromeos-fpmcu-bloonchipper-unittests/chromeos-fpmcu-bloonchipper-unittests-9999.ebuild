# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.makefile file.

EAPI=7

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
KEYWORDS="~*"
