# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

SUBTREES=(
	.gn
	camera/build
	camera/common
	camera/features
	camera/gpu
	camera/hardware_buffer
	camera/include
	camera/mojo
	chromeos-config
	common-mk
	iioservice/libiioservice_ipc
	iioservice/mojo
	metrics
	ml_core
	mojo_service_manager
)

CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_SUBTREE="${SUBTREES[*]}"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="camera/common"

WANT_LIBCHROME="yes"
WANT_LIBBRILLO="yes"

inherit cros-camera cros-constants cros-workon platform

DESCRIPTION="ChromeOS camera common libraries."

LICENSE="BSD-Google"
KEYWORDS="~*"

CAMERA_FEATURE_PREFIX="camera_feature_"
IUSE_FEATURE_FLAGS=(
	auto_framing
	effects
	face_detection
	frame_annotator
	hdrnet
	portrait_mode
	super_res
)
IUSE_PLATFORM_FLAGS=(
	ipu6
	ipu6ep
	ipu6epadln
	ipu6epmtl
	ipu6se
	qualcomm_camx
)

# FEATURE and PLATFORM IUSE flags are passed to and used in BUILD.gn files.
IUSE="
	${IUSE_FEATURE_FLAGS[*]/#/${CAMERA_FEATURE_PREFIX}}
	${IUSE_PLATFORM_FLAGS[*]}
	camera_angle_backend
	camera_diagnostics
	cros_camera_algo
	dlc
	libcamera
"

BDEPEND="
	chromeos-base/minijail
	virtual/pkgconfig
"

RDEPEND="
	chromeos-base/chromeos-config-tools:=
	chromeos-base/cros-camera-android-deps:=
	chromeos-base/mojo_service_manager:=
	camera_feature_effects? ( dev-libs/ml-core:= )
	dev-libs/re2:=
	camera_angle_backend? ( media-libs/cros-camera-gl-loader:= )
	media-libs/cros-camera-libfs:=
	dlc? (
		camera_feature_super_res? ( media-libs/cros-camera-super-res-dlc:= )
	)
	media-libs/libexif:=
	media-libs/libsync:=
	media-libs/minigbm:=
	virtual/jpeg:0=
	virtual/libudev:=
	virtual/opengles:=
	x11-libs/libdrm:=
"

DEPEND="
	${RDEPEND}
	>=chromeos-base/metrics-0.0.1-r3152:=
	chromeos-base/system_api:=
	dev-cpp/abseil-cpp:=
	media-libs/cros-camera-libcamera_connector_headers:=
	media-libs/libyuv:=
	x11-base/xorg-proto:=
	x11-drivers/opengles-headers:=
"

src_configure() {
	cros_optimize_package_for_speed
	platform_src_configure
}

src_install() {
	local fuzzer_component_id="167281"
	platform_fuzzer_install "${S}"/OWNERS \
			"${OUT}"/camera_still_capture_processor_impl_fuzzer \
			--comp "${fuzzer_component_id}"
	platform_src_install
}
