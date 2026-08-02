# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7
CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "bf7545596f4effcd59c040c4c45d8fb29f449ece" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_DESTDIR="${S}/platform2"
CROS_WORKON_SUBTREE="common-mk flex_bluetooth .gn"

PLATFORM_SUBDIR="flex_bluetooth"

inherit cros-workon platform

DESCRIPTION="Apply (Floss) Bluetooth overrides for ChromeOS Flex"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/flex_bluetooth"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

platform_pkg_test() {
	platform_test "run" "${OUT}/flex_bluetooth_overrides_test"
}
