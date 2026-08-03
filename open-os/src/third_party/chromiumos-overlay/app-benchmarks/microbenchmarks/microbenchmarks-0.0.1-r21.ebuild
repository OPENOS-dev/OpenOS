# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="217fc2ff67fd8ac85722fdd13c2a4f5420b20805"
CROS_WORKON_TREE="9081b026e3ff3cb14dffb2d93d96541c2ed0ba93"
CROS_WORKON_PROJECT="chromiumos/platform/microbenchmarks"
CROS_WORKON_LOCALNAME="../platform/microbenchmarks"

inherit cros-workon cros-common.mk cros-sanitizers

DESCRIPTION="Home for microbenchmarks designed in-house."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/microbenchmarks"

LICENSE="BSD-Google"
KEYWORDS="*"
# This package has no unittests.
RESTRICT="test"

src_configure() {
	sanitizers-setup-env
	default
}

src_install() {
	dobin "${OUT}"/memory-eater/memory-eater
}
