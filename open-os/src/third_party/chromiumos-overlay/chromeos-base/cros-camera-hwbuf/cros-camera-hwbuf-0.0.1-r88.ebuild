# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "4775a5be81f113aa93a6867b9e5c2576fcab81e6" "7a9e526334943d388f00f39cd3e4fdfcfabe6426" "824ea58991cc7bca28f57df8fedafdf7dec36a29" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3")
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
KEYWORDS="*"

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
