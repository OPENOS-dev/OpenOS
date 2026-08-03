# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_SUBTREE=".gn camera/common/basic_ops_perf_tests common-mk"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="camera/common/basic_ops_perf_tests"

inherit cros-workon platform

DESCRIPTION="ChromeOS Camera Basic Operations Performance Tests"

LICENSE="BSD-Google"
KEYWORDS="~*"
# This package has no unittests.
RESTRICT="test"

BDEPEND="virtual/pkgconfig"

RDEPEND="
	dev-cpp/benchmark:=
"
DEPEND="
	${RDEPEND}
"

src_configure() {
	cros_optimize_package_for_speed
	platform_src_configure
}
