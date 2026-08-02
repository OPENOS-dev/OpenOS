# Copyright 2014 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI="7"

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "048dfcda726535de6be71f7b250b1108be03207e" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_DESTDIR="${S}"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk avtest_label_detect .gn"

inherit cros-sanitizers cros-workon cros-common.mk

DESCRIPTION="Autotest label detector for audio/video/camera"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/avtest_label_detect"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE="-asan v4l2_codec vaapi cdm_factory_daemon"
# This package has no unittests.
RESTRICT="test"

RDEPEND="vaapi? ( x11-libs/libva )"
DEPEND="${RDEPEND}"

src_unpack() {
	cros-workon_src_unpack
	S+="/avtest_label_detect"
}

src_configure() {
	export USE_VAAPI=$(usex vaapi)
	export USE_V4L2_CODEC=$(usex v4l2_codec)
	export ENABLE_L1HWDRM=$(usex cdm_factory_daemon)
	sanitizers-setup-env
	cros-common.mk_src_configure
}

src_install() {
	# Install built tools
	dobin "${OUT}"/avtest_label_detect

	insinto /etc
	doins "${S}"/avtest_label_detect.conf
}
