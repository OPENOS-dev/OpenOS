# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "5697d5958a4e8c378cc194a89a2f34759bc417ec" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_DESTDIR="${S}/platform2"
CROS_WORKON_SUBTREE="common-mk metrics ml_core .gn"

DESCRIPTION="Chrome OS ML Core Feature Library"

PLATFORM_SUBDIR="ml_core"

inherit cros-workon platform user unpacker

LICENSE="BSD-Google"
KEYWORDS="*"
# This package has no unittests.
RESTRICT="test"

# camera_feature_effects needed as `use.camera_feature_effects` is
# referenced in BUILD.gn
IUSE="
	local_ml_core_internal
	camera_feature_effects
	ondevice_image_content_annotation
	intel_openvino_delegate
	mtk_neuron_delegate
"

SRC_URI="gs://chromeos-localmirror/distfiles/ml-core-headers-20241107.tar.xz"

RDEPEND="
	chromeos-base/dlcservice-client:=
	chromeos-base/libbrillo:=
	chromeos-base/libchrome:=
	chromeos-base/metrics:=
	chromeos-base/session_manager-client:=
	chromeos-base/system_api:=
	camera_feature_effects? (
		dev-libs/ml-core-dlc:=
		virtual/opengles:=
	)
	ondevice_image_content_annotation? ( dev-libs/ml-core-dlc:= )
"

DEPEND="
	camera_feature_effects? (
		x11-drivers/opengles-headers:=
	)
	${RDEPEND}
"

BDEPEND="
	chromeos-base/minijail
"

src_unpack() {
	platform_src_unpack

	# Unpack the headers into the srcdir
	pushd "${S}" > /dev/null || die
	if use local_ml_core_internal; then
		# Unpack local build.
		local dev_tarball="/mnt/google3_staging/ml-core-libcros_ml_core_internal-dev.tar.xz"
		echo "Checking for ${dev_tarball}"
		[[ ! -f "${dev_tarball}" ]] && die "Couldn't find ${dev_tarball} used by local_ml_core_internal. Did you run chromeos/ml/build_dev.sh in google3?"
		echo "Unpacking ${dev_tarball}"
		unpack "${dev_tarball}"
	else
		# Unpack SRC_URI
		unpacker
	fi
	popd > /dev/null || die
}

src_configure() {
	if use local_ml_core_internal; then
		append-cppflags "-DUSE_LOCAL_ML_CORE_INTERNAL"
	fi
	platform_src_configure
}

pkg_setup() {
	# Has to be done in pkg_setup() instead of pkg_preinst() since
	# src_install() needs ml-core.
	enewuser "ml-core"
	enewgroup "ml-core"
	cros-workon_pkg_setup
}
