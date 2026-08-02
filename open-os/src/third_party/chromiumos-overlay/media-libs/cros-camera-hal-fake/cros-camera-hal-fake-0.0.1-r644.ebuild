# Copyright 2022 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="21ace4788a915eae565039b47f6c862f1b8ebc29"
CROS_WORKON_TREE=("f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "4775a5be81f113aa93a6867b9e5c2576fcab81e6" "9d5a2a0cdb9fce70bfb36ab315206c7927d44227" "6165906bdd43c9bd6d2a5d03e299fca26c4f23e6" "824ea58991cc7bca28f57df8fedafdf7dec36a29" "0e7d4d4aac5fe2e42c89c6335278db4cd635aec2" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3")
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="../platform2"
# TODO(b/187784160): Avoid directly including headers from other packages.
CROS_WORKON_SUBTREE=".gn camera/build camera/common camera/hal/fake camera/include camera/mojo common-mk"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="camera/hal/fake"

inherit cros-camera cros-workon platform

DESCRIPTION="ChromeOS fake camera HAL."

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""
# This package has no unittests.
RESTRICT="test"

RDEPEND="
	chromeos-base/cros-camera-android-deps:=
	media-libs/libsync:=
	media-libs/libyuv:=
	virtual/jpeg:0="
DEPEND="${RDEPEND}"
BDEPEND="virtual/pkgconfig:="
