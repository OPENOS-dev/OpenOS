# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="30a5abfd06297e0a86d2e4c4da08d98058df54d5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "6254ee88618b3a25710d425883a96ce5da6e351d" "ee9c0d9e5d2a98ba4313fae503ce3a73352e6580" "5697d5958a4e8c378cc194a89a2f34759bc417ec" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_DESTDIR="${S}/platform2"
# ml and ml_core for building libml_for_benchmark.so.
CROS_WORKON_SUBTREE="common-mk ml ml_benchmark ml_core .gn"

DESCRIPTION="Chrome OS ML Benchmarking Suite"

PLATFORM_SUBDIR="ml_benchmark"
# Do not run test parallelly until unit tests are fixed.
# shellcheck disable=SC2034
PLATFORM_PARALLEL_GTEST_TEST="no"

inherit cros-workon platform cros-protobuf

# chromeos-base/ml_benchmark blocked due to package rename
RDEPEND="
	chromeos-base/dlcservice-client:=
	>=chromeos-base/metrics-0.0.1-r3152:=
	!chromeos-base/ml_benchmark
	chromeos-base/system_api:=
	dev-libs/re2:=
	vulkan? ( media-libs/clvk )
	sci-libs/tensorflow:=
"

DEPEND="${RDEPEND}"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="ml_benchmark_drivers vulkan"

src_install() {
	platform_src_install

	if use ml_benchmark_drivers; then
		insinto /usr/local/ml_benchmark/ml_service
		insopts -m0755
		doins "${OUT}"/lib/libml_for_benchmark.so
		insopts -m0644
	fi
}
