# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit cros-binary

DESCRIPTION="Google grammar check library for Chrome OS"
HOMEPAGE="https://www.chromium.org/chromium-os"

LICENSE="BSD-Google"
SLOT="0"

DIST_URL="gs://chromeos-localmirror/distfiles"

# ABI march flag -> URI mappings
# amd64
# Shellcheck can't understand namedrefs as function arguments.
# shellcheck disable=SC2034
declare -A march_uris_amd64=(
	["march_x86-64"]="${DIST_URL}/libgrammar-amd64-${PV}.tar.gz"
)

# arm64
# Shellcheck can't understand namedrefs as function arguments.
# shellcheck disable=SC2034
declare -A march_uris_arm64=(
	["march_armv8-a"]="${DIST_URL}/libgrammar-arm64-${PV}.tar.gz"
)

# arm
# TODO(go/cros-arm64-plan): Remove once all boards have migrated to 64-bit user space.
# Shellcheck can't understand namedrefs as function arguments.
# shellcheck disable=SC2034
declare -A march_uris_arm=(
	["march_armv7-a"]="${DIST_URL}/libgrammar-arm-${PV}.tar.gz"
	["march_armv8-a"]="${DIST_URL}/libgrammar-arm-${PV}.tar.gz"
)

SRC_URI="
	$(cros-binary_generate_src_uris march_uris_amd64 march_uris_arm64 march_uris_arm)
"

KEYWORDS="*"

IUSE="ondevice_grammar"

# All possible march USE flags.
# Declared in cros-binary.eclass.
# shellcheck disable=SC2154
IUSE+=" ${CROS_BINARY_MARCHS_USE}"

# Exactly one march flag is required.
# Declared in cros-binary.eclass.
# shellcheck disable=SC2154
REQUIRED_USE="${CROS_BINARY_MARCHS_REQUIRED_USE}"

S="${WORKDIR}"

LIB_PATH="libgrammar-${ARCH}"
MODEL_PATH="libgrammar-${ARCH}/sentence_explorer_cpu"

src_install() {
	# Always install the header and proto files.
	insinto /usr/include/chromeos/libgrammar/
	doins "${LIB_PATH}/grammar_interface.h"
	insinto /usr/include/chromeos/libgrammar/proto/
	doins "${LIB_PATH}/grammar_interface.proto"

	if use ondevice_grammar; then
		insinto /opt/google/chrome/ml_models/grammar/
		# Install the shared library.
		insopts -m0755
		doins "${LIB_PATH}/libgrammar.so"
		insopts -m0644
		# Install the model files.
		doins "${MODEL_PATH}/translation_model.pb"
		doins "${MODEL_PATH}/model.pb"
		doins "${MODEL_PATH}/decoder_init_0.tflite"
		doins "${MODEL_PATH}/decoder_step_0.tflite"
		doins "${MODEL_PATH}/encoder_0.tflite"
		doins "${MODEL_PATH}/wpm.model"
		doins "${MODEL_PATH}/wpm.vocab"
	fi
}
