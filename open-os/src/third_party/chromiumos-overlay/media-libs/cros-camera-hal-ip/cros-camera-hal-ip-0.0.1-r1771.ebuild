# Copyright 2019 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="1b7a3e54d8c660cddc38d7869af656d801d22b19"
CROS_WORKON_TREE=("f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "4775a5be81f113aa93a6867b9e5c2576fcab81e6" "9d5a2a0cdb9fce70bfb36ab315206c7927d44227" "c25333159467faa1ff2fed4ade8e0f65ba6b233d" "ce58cd961aa5392c526f0fcd7628a79d7fd76c38" "824ea58991cc7bca28f57df8fedafdf7dec36a29" "0e7d4d4aac5fe2e42c89c6335278db4cd635aec2" "16839d1d0fcecb39e52d1bc3a65aa02d62c1ee6d" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3")
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_SUBTREE=".gn camera/build camera/common camera/hal/ip camera/hal/usb camera/include camera/mojo chromeos-config common-mk"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="camera/hal/ip"

inherit cros-camera cros-workon platform

DESCRIPTION="Chrome OS IP camera HAL v3."

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
RDEPEND="
	chromeos-base/chromeos-config-tools
	chromeos-base/cros-camera-android-deps
	chromeos-base/cros-camera-libs
	media-libs/libsync"

DEPEND="${RDEPEND}
	virtual/pkgconfig
	dev-cpp/abseil-cpp"
