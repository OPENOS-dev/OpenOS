# Copyright 2022 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "4775a5be81f113aa93a6867b9e5c2576fcab81e6" "9d5a2a0cdb9fce70bfb36ab315206c7927d44227" "962cb52e4f08bd4182788a2080e6430dbb4fb576" "213d3b621a1bead4ad0d434f403724951a7eb23b" "7a9e526334943d388f00f39cd3e4fdfcfabe6426" "824ea58991cc7bca28f57df8fedafdf7dec36a29" "0e7d4d4aac5fe2e42c89c6335278db4cd635aec2" "16839d1d0fcecb39e52d1bc3a65aa02d62c1ee6d" "1e601fb1df98e9ea9f5803aeb50bd6fbec835a2a" "58a785339f2df72dc213c8573a3f65d24da322c5" "5697d5958a4e8c378cc194a89a2f34759bc417ec")
CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_DESTDIR="${S}/platform2"
CROS_WORKON_SUBTREE=".gn common-mk metrics camera/build camera/common camera/features camera/gpu camera/hardware_buffer camera/include camera/mojo chromeos-config iioservice/libiioservice_ipc iioservice/mojo ml_core"

DESCRIPTION="Tests Effects Stream Manipulator"

PLATFORM_SUBDIR="camera/features/effects/tests"

SRC_URI="gs://chromeos-localmirror/distfiles/ml-core-cros_effects_test_assets-0.0.18.tar.xz"
RESTRICT="mirror"

inherit cros-workon unpacker platform cros-protobuf

LICENSE="BSD-Google"
KEYWORDS="*"
SLOT=0

RDEPEND="
	chromeos-base/cros-camera-android-deps:=
	chromeos-base/cros-camera-libs:=
	chromeos-base/dlcservice:=
	chromeos-base/dlcservice-client:=
	chromeos-base/metrics:=
	dev-libs/ml-core:=
	media-libs/libsync:=
	media-libs/libyuv:=
	media-libs/minigbm:=
	virtual/opengles:=
	x11-libs/libdrm:=
"

DEPEND="
	${RDEPEND}
	x11-drivers/opengles-headers:=
"

src_unpack() {
	unpacker
	platform_src_unpack
}

src_install() {
	platform_src_install

	into /usr/local
	dobin "${OUT}"/cros_effects_sm_tests

	insinto /usr/local/share/ml-core-effects-test-assets
	doins -r "${WORKDIR}"/cros_effects_test_assets/*
}
