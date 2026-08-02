# Copyright 2022 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v3

EAPI=7

inherit dlc cros-binary

DESCRIPTION='ScreenAI is a binary to provide AI based models to improve
assistive technologies. The binary is written in C++ and is currently used by
ReadAnything and PdfOcr services on Chrome OS.'
HOMEPAGE=""

# ABI march flag -> URI mappings
# amd64
# Shellcheck can't understand namedrefs as function arguments.
# shellcheck disable=SC2034
declare -A march_uris_amd64=(
	["march_alderlake"]="gs://chromeos-localmirror/distfiles/${PN}-x86_64_alderlake-${PV}.tar.xz"
	["march_bdver4"]="gs://chromeos-localmirror/distfiles/${PN}-x86_64_bdver4-${PV}.tar.xz"
	["march_corei7"]="gs://chromeos-localmirror/distfiles/${PN}-x86_64_corei7-${PV}.tar.xz"
	["march_goldmont"]="gs://chromeos-localmirror/distfiles/${PN}-x86_64_goldmont-${PV}.tar.xz"
	["march_silvermont"]="gs://chromeos-localmirror/distfiles/${PN}-x86_64_silvermont-${PV}.tar.xz"
	["march_skylake"]="gs://chromeos-localmirror/distfiles/${PN}-x86_64_skylake-${PV}.tar.xz"
	["march_tigerlake"]="gs://chromeos-localmirror/distfiles/${PN}-x86_64_tigerlake-${PV}.tar.xz"
	["march_tremont"]="gs://chromeos-localmirror/distfiles/${PN}-x86_64_tremont-${PV}.tar.xz"
	["march_x86-64"]="gs://chromeos-localmirror/distfiles/${PN}-x86_64-${PV}.tar.xz"
	["march_znver1"]="gs://chromeos-localmirror/distfiles/${PN}-x86_64_znver1-${PV}.tar.xz"
)

# arm64
# Shellcheck can't understand namedrefs as function arguments.
# shellcheck disable=SC2034
declare -A march_uris_arm64=(
	["march_armv8-a"]="gs://chromeos-localmirror/distfiles/${PN}-arm64-${PV}.tar.xz"
)

# arm
# TODO(go/cros-arm64-plan): Remove once all boards have migrated to 64-bit user space.
# Shellcheck can't understand namedrefs as function arguments.
# shellcheck disable=SC2034
declare -A march_uris_arm=(
	["march_armv7-a"]="gs://chromeos-localmirror/distfiles/${PN}-arm32-${PV}.tar.xz"
	["march_armv8-a"]="gs://chromeos-localmirror/distfiles/${PN}-arm32-${PV}.tar.xz"
)

SRC_URI="
	$(cros-binary_generate_src_uris march_uris_amd64 march_uris_arm64 march_uris_arm)
"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE="dlc"
REQUIRED_USE="dlc"

# All possible march USE flags.
# Declared in cros-binary.eclass.
# shellcheck disable=SC2154
IUSE+=" ${CROS_BINARY_MARCHS_USE}"

# Exactly one march flag is required.
# Declared in cros-binary.eclass.
# shellcheck disable=SC2154
REQUIRED_USE+=" ${CROS_BINARY_MARCHS_REQUIRED_USE}"

# DLC variables.
# 4KB * 16000 = 64 MB
DLC_PREALLOC_BLOCKS="16000"
# Preload on test images
DLC_PRELOAD=true
DLC_SCALED=true

S="${WORKDIR}"

src_install() {
	# Install binary.
	insinto "$(dlc_add_path /)"
	doins libchromescreenai.so

	# Model files lists.
	doins files_list_main_content_extraction.txt
	doins files_list_ocr.txt

	# Install Main Content Extraction model files.
	doins screen2x_config.pbtxt screen2x_model.tflite

	# Install OCR model files.
	# We need to put OCR model files in the same directory
	# structure as their Google3 locations. This requirement will be removed
	# after we update the file handling so that the files would be loaded in
	# Chrome and passed to the binary.
	doins gocr_mobile_chrome_multiscript_2024_q4_engine.binarypb
	insinto "$(dlc_add_path /aksara)"
	doins aksara/aksara_page_layout_analysis_rpn_gro_2024_q4.binarypb
	doins aksara/aksara_page_layout_analysis_ti_rpn_gro_2024_q4.binarypb
	insinto "$(dlc_add_path /gocr)"
	insinto "$(dlc_add_path /gocr/gocr_models)"
	doins \
	    gocr/gocr_models/gocr_line_recognition_omni_mobile_chrome_multiscript_2024_q4.binarypb
	insinto "$(dlc_add_path /gocr/gocr_models/line_recognition_mobile_convnext320_omni)"
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/arab.tflite
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/arab_fst_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/arab_label_map.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/arab_lm.fst
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/arab_lm.syms
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/arab_prior.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/bede.tflite
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/bede_label_map.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/bede_prior.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/beng_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/beng_deva_gujr_guru.tflite
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/beng_deva_gujr_guru_label_map.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/cyrl.tflite
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/cyrl_fst_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/cyrl_label_map.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/cyrl_lm.fst
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/cyrl_lm.syms
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/cyrl_prior.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/deva_fst_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/deva_lm.fst
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/deva_lm.syms
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/geor.tflite
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/geor_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/geor_label_map.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/gocr_mobile_und.tflite
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/gocr_mobile_und_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/gocr_mobile_und_label_map.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/grek.tflite
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/grek_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/grek_label_map.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/gujr.tflite
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/gujr_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/gujr_label_map.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/guru_2024_q3_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/hani_fst_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/hani_lm.fst
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/hani_lm.syms
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/hanijpan.tflite
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/hanijpan_label_map.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/hanijpan_prior.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/hebr.tflite
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/hebr_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/hebr_label_map.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/jpan_fst_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/jpan_lm.fst
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/jpan_lm.syms
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/khmr_2024_q3_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/khmr_laoo_thai_label_map.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/khmr_laoo_thai.tflite
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/khmr_laoo_thai_prior.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/knda_fst_2024_q3_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/knda_lm.fst
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/knda_lm.syms
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/knda_sinh_telu.tflite
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/knda_sinh_telu_label_map.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/knda_sinh_telu_prior.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/kore.tflite
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/kore_fst_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/kore_label_map.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/kore_lm.fst
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/kore_lm.syms
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/kore_prior.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/laoo_2024_q3_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/mlym.tflite
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/mlym_fst_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/mlym_label_map.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/mlym_lm.fst
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/mlym_lm.syms
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/mlym_prior.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/sinh_2024_q3_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/taml.tflite
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/taml_fst_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/taml_label_map.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/taml_lm.fst
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/taml_lm.syms
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/taml_prior.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/telu_fst_2024_q3_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/telu_lm.fst
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/telu_lm.syms
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/tflite_langid.tflite
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/thai_fst_2024_q3_config.pb
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/thai_lm.fst
	doins \
		gocr/gocr_models/line_recognition_mobile_convnext320_omni/thai_lm.syms
	insinto "$(dlc_add_path gocr/gocr_models/detection)"
	doins \
		gocr/gocr_models/detection/gocr_group_rpn_text_detection_config_2024_q4_chrome.binarypb
	doins \
		gocr/gocr_models/detection/gocr_group_rpn_text_detection_model_2024_q4.tflite
	insinto "$(dlc_add_path gocr/layout)"
	insinto "$(dlc_add_path gocr/layout/cluster_sort)"
	doins \
		gocr/layout/cluster_sort/model_v2.tflite
	dlc_src_install
}
