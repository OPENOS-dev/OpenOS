# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

SUBTREES=(
	.gn
	camera/build
	camera/common
	camera/features
	camera/feature_benchmark
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
CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_SUBTREE="${SUBTREES[*]}"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="camera/feature_benchmark"

inherit cros-camera cros-constants cros-workon platform

DESCRIPTION="ChromeOS camera feature benchmark tool"

LICENSE="BSD-Google"
KEYWORDS="~*"

CAMERA_FEATURE_PREFIX="camera_feature_"
IUSE_FEATURE_FLAGS=(
	auto_framing
	effects
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
	dlc
	houdini
"
# This package has no unittests.
RESTRICT="test"

BDEPEND="
	chromeos-base/minijail
	virtual/pkgconfig
"

RDEPEND="
	chromeos-base/chromeos-config-tools:=
	chromeos-base/cros-camera-android-deps:=
	chromeos-base/mojo_service_manager:=
	dev-libs/re2:=
	media-libs/cros-camera-libfs:=
	media-libs/libexif:=
	media-libs/libsync:=
	media-libs/libyuv:=
	media-libs/minigbm:=
"

DEPEND="
	${RDEPEND}
	>=chromeos-base/metrics-0.0.1-r3152:=
"

src_configure() {
	cros_optimize_package_for_speed
	platform_src_configure
}
