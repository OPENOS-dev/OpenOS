# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT="38272f61162fa7fa1b6dfa7a6afc1ac8d2f9650b"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "95d4df7f9212a6560d43b5e6eee5c905a64eefd8" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk biod .gn"

PLATFORM_SUBDIR="biod/mock-biod-test-deps"

inherit cros-workon platform

DESCRIPTION="biod test-only dbus policies. This package resides in test image only."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/biod/"

LICENSE="BSD-Google"
KEYWORDS="*"
# This package has no unittests.
RESTRICT="test"

DEPEND=""
RDEPEND="${DEPEND}"
BDEPEND=""

src_compile() {
	# We only install policy files here, no need to compile.
	:
}

src_install() {
	platform_src_install
}
