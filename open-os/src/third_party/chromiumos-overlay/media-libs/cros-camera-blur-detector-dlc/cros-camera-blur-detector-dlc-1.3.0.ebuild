# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

inherit cros-camera dlc unpacker

DESCRIPTION="Package for blur detector library as a DLC"
SRC_URI="$(cros-camera_generate_blur_detection_package_SRC_URI ${PV})"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE="dlc"
REQUIRED_USE="dlc"

S="${WORKDIR}"

# The max size of the Blur Detector library is about 5.5 MB. Therefore,
# considering the future growth, we reserve 10 MB. Don't increase this
# size. Create a new ebuild if the size exceeds 10 MB (b/314817068).
DLC_PREALLOC_BLOCKS="$((10 * 256))"
DLC_PRELOAD=true
DLC_SCALED=true

src_install() {
	insinto /usr/include/chromeos/libblurdetector/
	doins "${WORKDIR}"/blur_detector_bindings.h

	exeinto "$(dlc_add_path /)"

	doexe "libblurdetector.so"
	dlc_src_install
}
