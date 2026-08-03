# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "4775a5be81f113aa93a6867b9e5c2576fcab81e6" "9d5a2a0cdb9fce70bfb36ab315206c7927d44227" "962cb52e4f08bd4182788a2080e6430dbb4fb576" "213d3b621a1bead4ad0d434f403724951a7eb23b" "ee40e4f4c089bffab280ca19e31e720b45ea658b" "c437000cc83559d1f434d54c191a4768b37dad99" "824ea58991cc7bca28f57df8fedafdf7dec36a29" "0e7d4d4aac5fe2e42c89c6335278db4cd635aec2" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "b8033e453c7d9518619e90fb100d7d90d7b4026d" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "5697d5958a4e8c378cc194a89a2f34759bc417ec" "43eb4f30218ee6fc055f185786d914bccd668086")
SUBTREES=(
	.gn
	camera/build
	camera/common
	camera/features
	camera/gpu
	# TODO(crbug.com/914263): camera/hal is unnecessary for this build but
	# is workaround for unexpected sandbox behavior.
	camera/hal
	camera/hal_adapter
	camera/include
	camera/mojo
	common-mk
	featured
	metrics
	ml_core
	mojo_service_manager
)

CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_SUBTREE="${SUBTREES[*]}"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="camera/hal_adapter"

inherit cros-camera cros-constants cros-protobuf cros-workon platform tmpfiles user udev

DESCRIPTION="ChromeOS camera service. The service is in charge of accessing
camera device. It uses unix domain socket to build a synchronous channel."

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="
	arcvm
	camera_angle_backend
	camera_diagnostics
	camera_feature_face_detection
	camera_feature_super_res
	cheets
	-libcamera
"
# This package has no unittests.
RESTRICT="test"

BDEPEND="virtual/pkgconfig"

RDEPEND="
	>=chromeos-base/cros-camera-libs-0.0.1-r34:=
	chromeos-base/cros-camera-android-deps:=
	chromeos-base/system_api:=
	media-libs/cros-camera-hal-usb:=
	media-libs/libsync:=
	media-libs/libyuv:=
	libcamera? ( media-libs/libcamera )
	!libcamera? ( virtual/cros-camera-hal )
	virtual/cros-camera-hal-configs
"

DEPEND="${RDEPEND}
	chromeos-base/dlcservice-client:=
	camera_angle_backend? ( chromeos-base/featured:= )
	>=chromeos-base/metrics-0.0.1-r3152:=
	media-libs/minigbm:=
	x11-drivers/opengles-headers:=
	x11-libs/libdrm:=
"


BDEPEND="
	chromeos-base/minijail
"

src_configure() {
	cros_optimize_package_for_speed
	platform_src_configure
}

src_install() {
	platform_src_install
	udev_dorules udev/99-camera.rules
	dotmpfiles tmpfiles.d/*.conf
}

pkg_preinst() {
	enewuser "arc-camera"
	enewgroup "arc-camera"
	enewgroup "camera"
}
