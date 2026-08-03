# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

SUBTREES=(
	.gn
	camera/build
	camera/hardware_buffer
	camera/include
	common-mk
)

CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_SUBTREE="${SUBTREES[*]}"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="camera/hardware_buffer"

inherit cros-camera cros-constants cros-workon platform

DESCRIPTION="ChromeOS camera hardware buffer library."

LICENSE="BSD-Google"
KEYWORDS="~*"

IUSE="test"

RDEPEND="
	test? ( dev-cpp/benchmark:= )
	media-libs/minigbm:=
"

DEPEND="${RDEPEND}"

src_configure() {
	cros_optimize_package_for_speed
	platform_src_configure
}
