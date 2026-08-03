# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit dlc cros-binary

DESCRIPTION="Google handwriting recognition library for ChromeOS"
HOMEPAGE="https://www.chromium.org/chromium-os"

# ABI march flag -> URI mappings
# amd64
# Shellcheck can't understand namedrefs as function arguments.
# shellcheck disable=SC2034
declare -A march_uris_amd64=(
	["march_x86-64"]="gs://chromeos-localmirror/distfiles/libhandwriting_chromeos_amd64-${PV}.tar.gz"
	["march_silvermont"]="gs://chromeos-localmirror/distfiles/libhandwriting_chromeos_silvermont-${PV}.tar.gz"
	["march_goldmont"]="gs://chromeos-localmirror/distfiles/libhandwriting_chromeos_goldmont-${PV}.tar.gz"
)

# arm64
# Shellcheck can't understand namedrefs as function arguments.
# shellcheck disable=SC2034
declare -A march_uris_arm64=(
	["march_armv8-a"]="gs://chromeos-localmirror/distfiles/libhandwriting_chromeos_arm64-${PV}.tar.gz"
)

# arm
# TODO(go/cros-arm64-plan): Remove once all boards have migrated to 64-bit user space.
# Shellcheck can't understand namedrefs as function arguments.
# shellcheck disable=SC2034
declare -A march_uris_arm=(
	["march_armv7-a"]="gs://chromeos-localmirror/distfiles/libhandwriting_chromeos_arm32-${PV}.tar.gz"
	["march_armv8-a"]="gs://chromeos-localmirror/distfiles/libhandwriting_chromeos_arm32-${PV}.tar.gz"
)

SRC_URI="
	$(cros-binary_generate_src_uris march_uris_amd64 march_uris_arm64 march_uris_arm)
	gs://chromeos-localmirror/distfiles/libhandwriting-test-data-0.0.1.tar.gz
"

RESTRICT="mirror"

LICENSE="BSD-Google Apache-2.0 MPL-2.0 icu-58"
SLOT="0"
KEYWORDS="*"

IUSE="
	ondevice_handwriting
	ondevice_handwriting_dlc
	dlc
"

# All possible march USE flags.
# Declared in cros-binary.eclass.
# shellcheck disable=SC2154
IUSE+="${CROS_BINARY_MARCHS_USE}"

# ondevice_handwriting and ondevice_handwriting_dlc should be enabled at most
# one. If ondevice_handwriting_dlc is enabled; dlc should also be enabled.
REQUIRED_USE="
	ondevice_handwriting_dlc? ( dlc )
	?? ( ondevice_handwriting ondevice_handwriting_dlc )
"

# Exactly one march flag is required.
# Declared in cros-binary.eclass.
# shellcheck disable=SC2154
REQUIRED_USE+="${CROS_BINARY_MARCHS_REQUIRED_USE}"

S="${WORKDIR}"

# The storage space for this dlc. This sets up the upper limit of this dlc to be
# DLC_PREALLOC_BLOCKS * 4KB = 40MB for now.
DLC_PREALLOC_BLOCKS="10240"
# Preload DLC data on test images.
DLC_PRELOAD=true

src_install() {
	insinto /usr/include/chromeos/libhandwriting/
	doins handwriting_interface.h
	insinto /usr/include/chromeos/libhandwriting/proto/
	doins handwriting_interface.proto
	sed -i 's!chrome/knowledge/handwriting/!!g' handwriting_validate.proto || die
	doins handwriting_validate.proto

	if ! use ondevice_handwriting && ! use ondevice_handwriting_dlc; then
		return
	fi

	if use ondevice_handwriting; then
		local handwritinglib_path="/opt/google/chrome/ml_models/handwriting/"
	else
		local handwritinglib_path="$(dlc_add_path /)"
	fi

	insinto "${handwritinglib_path}"
	# Install the shared library.
	insopts -m0755
	newins "libhandwriting.so" "libhandwriting.so"
	insopts -m0644
	# Install the model files for english.
	doins latin_indy.compact.fst latin_indy.pb latin_indy.tflite
	doins latin_indy_conf.tflite latin_indy_seg.tflite
	# Install the model files for gesture recognition.
	newins gic20190510.tflite gic.reco_model.tflite
	newins gic20190510_cros.ondevice.recospec.pb gic.recospec.pb

	# Run dlc_src_install.
	if use ondevice_handwriting_dlc; then
		dlc_src_install
	fi

	# Only enable the tests for ondevice_handwriting.
	if use ondevice_handwriting; then
		# Install the testing data.
		insinto /build/share/libhandwriting/
		doins libhandwriting-test-data-0.0.1/handwriting_labeled_requests.pb
		doins libhandwriting-test-data-0.0.1/gesture_labeled_requests.pb
	fi
}
