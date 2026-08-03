# Copyright 2022 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "4775a5be81f113aa93a6867b9e5c2576fcab81e6" "962cb52e4f08bd4182788a2080e6430dbb4fb576" "824ea58991cc7bca28f57df8fedafdf7dec36a29" "213d3b621a1bead4ad0d434f403724951a7eb23b" "9d5a2a0cdb9fce70bfb36ab315206c7927d44227" "0e7d4d4aac5fe2e42c89c6335278db4cd635aec2" "7a9e526334943d388f00f39cd3e4fdfcfabe6426" "16839d1d0fcecb39e52d1bc3a65aa02d62c1ee6d" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "1e601fb1df98e9ea9f5803aeb50bd6fbec835a2a" "58a785339f2df72dc213c8573a3f65d24da322c5" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "5697d5958a4e8c378cc194a89a2f34759bc417ec" "43eb4f30218ee6fc055f185786d914bccd668086")
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="../platform2"
# iioservice/ is included just to make sandbox happy when running `gn gen`.
CROS_WORKON_SUBTREE=".gn camera/build camera/features camera/include camera/gpu camera/common camera/mojo camera/hardware_buffer chromeos-config common-mk iioservice/libiioservice_ipc iioservice/mojo metrics ml_core mojo_service_manager"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="camera/features/auto_framing/tests"

inherit cros-workon platform

DESCRIPTION="ChromeOS Camera Auto-framing feature tests"

LICENSE="BSD-Google"
KEYWORDS="*"

IUSE="camera_feature_auto_framing camera_feature_face_detection camera_feature_super_res dlc"

BDEPEND="virtual/pkgconfig"

RDEPEND="
	chromeos-base/chromeos-config-tools:=
	chromeos-base/cros-camera-android-deps:=
	chromeos-base/mojo_service_manager:=
	chromeos-base/system_api:=
	dev-cpp/gtest:=
	media-libs/cros-camera-libfs:=
	dlc? (
		camera_feature_super_res? ( media-libs/cros-camera-super-res-dlc:= )
	)
	media-libs/libexif:=
	media-libs/libsync:=
	media-libs/minigbm:=
	virtual/opengles:=
	x11-libs/libdrm:=
"

DEPEND="
	${RDEPEND}
	chromeos-base/metrics:=
	media-libs/libyuv:=
	x11-drivers/opengles-headers:=
"

src_configure() {
	cros_optimize_package_for_speed
	platform_src_configure
}
