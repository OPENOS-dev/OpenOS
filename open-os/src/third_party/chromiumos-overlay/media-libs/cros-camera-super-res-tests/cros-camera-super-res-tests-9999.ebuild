# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="../platform2"
# iioservice/ is included just to make sandbox happy when running `gn gen`.
CROS_WORKON_SUBTREE=".gn camera/build camera/features camera/include camera/gpu camera/common camera/mojo camera/hardware_buffer chromeos-config common-mk iioservice/libiioservice_ipc iioservice/mojo metrics ml_core mojo_service_manager"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="camera/features/super_resolution/tests"

inherit cros-protobuf cros-workon platform

DESCRIPTION="ChromeOS Camera Super Resolution feature tests"

LICENSE="BSD-Google"
KEYWORDS="~*"

IUSE="camera_angle_backend"

BDEPEND="virtual/pkgconfig"

RDEPEND="
	chromeos-base/cros-camera-android-deps:=
	chromeos-base/cros-camera-libs:=
	chromeos-base/dlcservice:=
	chromeos-base/dlcservice-client:=
	chromeos-base/metrics:=
	dev-cpp/gtest:=
	camera_angle_backend? ( media-libs/cros-camera-gl-loader:= )
	media-libs/cros-camera-super-res-dlc:=
	media-libs/libsync:=
	media-libs/libyuv:=
	media-libs/minigbm:=
"

DEPEND="
	${RDEPEND}
"

src_configure() {
	cros_optimize_package_for_speed
	platform_src_configure
}
