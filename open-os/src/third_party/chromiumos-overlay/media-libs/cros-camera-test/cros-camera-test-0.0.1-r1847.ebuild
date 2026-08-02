# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="1b7a3e54d8c660cddc38d7869af656d801d22b19"
CROS_WORKON_TREE=("f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "4775a5be81f113aa93a6867b9e5c2576fcab81e6" "2adccd58ec84e8ea826d265dfff085ce2011d845" "9d5a2a0cdb9fce70bfb36ab315206c7927d44227" "824ea58991cc7bca28f57df8fedafdf7dec36a29" "16839d1d0fcecb39e52d1bc3a65aa02d62c1ee6d" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3")
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="../platform2"
# TODO(crbug.com/809389): Avoid directly including headers from other packages.
CROS_WORKON_SUBTREE=".gn camera/build camera/camera3_test camera/common camera/include chromeos-config common-mk"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="camera/camera3_test"

inherit cros-camera cros-workon platform

DESCRIPTION="Chrome OS camera HAL native test."

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE="-asan"
# This package has no unittests.
RESTRICT="test"

RDEPEND="
	>=chromeos-base/cros-camera-libs-0.0.1-r34:=
	chromeos-base/cros-camera-android-deps
	media-libs/libexif
	media-libs/libsync
	media-libs/minigbm
	virtual/jpeg:0="

DEPEND="${RDEPEND}
	chromeos-base/system_api
	dev-cpp/gtest:=
	media-libs/libyuv
	virtual/pkgconfig"
