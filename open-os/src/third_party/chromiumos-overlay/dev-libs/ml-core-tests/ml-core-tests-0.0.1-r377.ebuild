# Copyright 2022 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "5697d5958a4e8c378cc194a89a2f34759bc417ec" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_DESTDIR="${S}/platform2"
CROS_WORKON_SUBTREE="common-mk metrics ml_core .gn"

DESCRIPTION="Tests for Chrome OS ML Core Feature Library"

PLATFORM_SUBDIR="ml_core/tests"

# Do not run test parallelly until unit tests are fixed.
# shellcheck disable=SC2034
PLATFORM_PARALLEL_GTEST_TEST="no"

inherit cros-workon platform cros-protobuf

# camera_feature_effects needed as `use.camera_feature_effects` is
# referenced in BUILD.gn
IUSE="camera_feature_effects"
LICENSE="BSD-Google"
KEYWORDS="*"

RDEPEND="
	chromeos-base/dlcservice:=
	chromeos-base/dlcservice-client:=
	chromeos-base/metrics:=
	dev-libs/ml-core:=
	media-libs/libpng:=
"

DEPEND="
	${RDEPEND}
	x11-drivers/opengles-headers:=
"

src_install() {
	platform_src_install

	insinto /usr/local/
	dobin "${OUT}"/ml_core_effects_pipeline_test

	insinto /usr/local/share/ml_core
	doins -r testdata/*.png
}
