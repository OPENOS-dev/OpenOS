# Copyright 2024 The ChromiumOS Authors. All rights reserved.
# Distributed under the terms of the GNU General Public License v3

EAPI=7

inherit cros-workon dlc

DESCRIPTION='FaceGaze is an accessibility feature on ChromeOS that allows users
to move the mouse with their forehead and perform clicks/keyboard actions with
facial gestures. The FaceGaze feature relies on the mediapipe FaceLandmarker API
to detect forehead position and facial gestures of the user.

This DLC packages the FaceLandmarker ML model and the mediapipe web assembly
required to run the FaceLandmarker API in JavaScript.'
HOMEPAGE=""
SRC_URI="gs://chromeos-localmirror/distfiles/${PN}-2.0.tar.xz"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="~*"

# "cros_workon info" expects these variables to be set, so use the standard
# empty project.
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

# DLC variables.
# 4KB * 4225 = ~16.9MB
DLC_PREALLOC_BLOCKS="4225"
DLC_SCALED=true
DLC_PRELOAD=true

S="${WORKDIR}"
src_unpack() {
	local archive="${SRC_URI##*/}"
	unpack "${archive}"
}

src_install() {
	insinto "$(dlc_add_path /)"
	doins face_landmarker.task vision_wasm_internal.wasm
	dlc_src_install
}
