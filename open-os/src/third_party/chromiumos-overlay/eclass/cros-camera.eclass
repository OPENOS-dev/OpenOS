# Copyright 2018 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# @ECLASS: cros-camera.eclass
# @MAINTAINER:
# Chromium OS Camera Team
# @BUGREPORTS:
# Please report bugs via http://crbug.com/new (with label Build)
# @VCSURL: https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/eclass/@ECLASS@
# @BLURB: helper eclass for building Chromium package in src/platform2/camera
# @DESCRIPTION:
# Packages in src/platform2/camera are in active development. We want builds
# to be incremental and fast. This centralized the logic needed for this.

inherit multilib cros-binary

# All possible march USE flags.
# Declared in cros-binary.eclass.
# shellcheck disable=SC2154
IUSE="${CROS_BINARY_MARCHS_USE}"
# Exactly one march flag is required.
# Declared in cros-binary.eclass.
# shellcheck disable=SC2154
REQUIRED_USE="${CROS_BINARY_MARCHS_REQUIRED_USE}"

# @FUNCTION: cros-camera_dohal
# @USAGE: <source HAL file> <destination HAL file>
# @DESCRIPTION:
# Install the given camera HAL library to /usr/lib/camera_hal or
# /usr/lib64/camera_hal, depending on the architecture and/or platform.
cros-camera_dohal() {
	[[ $# -eq 2 ]] || die "Usage: ${FUNCNAME[0]} <src> <dst>"

	local src=$1
	local dst=$2
	(
		insinto "/usr/$(get_libdir)/camera_hal"
		newins "${src}" "${dst}"
	)
}

# @FUNCTION: cros-camera_generate_auto_framing_package_SRC_URI
# @USAGE:
# @DESCRIPTION:
# Generate SRC_URI for auto framing package by PV.
cros-camera_generate_auto_framing_package_SRC_URI() {
	local pv="$1"
	local prefix="gs://chromeos-localmirror/distfiles/chromeos-camera-libautoframing"
	local suffix="${pv}.tar.zst"
	# ABI march flag -> URI mappings
	# amd64
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_amd64=(
		["march_x86-64"]="${prefix}-x86_64-${suffix}"
	)

	# arm64
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_arm64=()

	# arm
	# TODO(go/cros-arm64-plan): Remove once all boards have migrated to 64-bit user space.
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_arm=()

	cros-binary_generate_src_uris march_uris_amd64 march_uris_arm64 march_uris_arm
}

# @FUNCTION: cros-camera_generate_document_scanning_package_SRC_URI
# @USAGE:
# @DESCRIPTION:
# Generate SRC_URI for document scanning package by PV.
cros-camera_generate_document_scanning_package_SRC_URI() {
	local pv="$1"
	local prefix="gs://chromeos-localmirror/distfiles/chromeos-document-scanning-lib"
	local suffix="${pv}.tar.zst"
	# ABI march flag -> URI mappings
	# amd64
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_amd64=(
		["march_alderlake"]="${prefix}-x86_64-alderlake-${suffix}"
		["march_bdver4"]="${prefix}-x86_64-bdver4-${suffix}"
		["march_corei7"]="${prefix}-x86_64-corei7-${suffix}"
		["march_goldmont"]="${prefix}-x86_64-goldmont-${suffix}"
		["march_silvermont"]="${prefix}-x86_64-silvermont-${suffix}"
		["march_skylake"]="${prefix}-x86_64-skylake-${suffix}"
		["march_tigerlake"]="${prefix}-x86_64-tigerlake-${suffix}"
		["march_tremont"]="${prefix}-x86_64-tremont-${suffix}"
		["march_x86-64"]="${prefix}-x86_64-${suffix}"
		["march_znver1"]="${prefix}-x86_64-znver1-${suffix}"
	)

	# arm64
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_arm64=(
		["march_armv8-a"]="${prefix}-arm-${suffix}"
	)

	# arm
	# TODO(go/cros-arm64-plan): Remove once all boards have migrated to 64-bit user space.
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_arm=(
		["march_armv7-a"]="${prefix}-armv7-${suffix}"
		["march_armv8-a"]="${prefix}-armv7-armv8-${suffix}"
	)

	cros-binary_generate_src_uris march_uris_amd64 march_uris_arm64 march_uris_arm
}

# @FUNCTION: cros-camera_generate_blur_detection_package_SRC_URI
# @USAGE:
# @DESCRIPTION:
# Generate SRC_URI for blur detection package by PV.
cros-camera_generate_blur_detection_package_SRC_URI() {
	local pv="$1"
	local prefix="gs://chromeos-localmirror/distfiles/chromeos-blur-detector-lib"
	local suffix="${pv}.tar.zst"
	# ABI march flag -> URI mappings
	# amd64
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_amd64=(
		["march_alderlake"]="${prefix}-x86_64-alderlake-${suffix}"
		["march_bdver4"]="${prefix}-x86_64-bdver4-${suffix}"
		["march_corei7"]="${prefix}-x86_64-corei7-${suffix}"
		["march_goldmont"]="${prefix}-x86_64-goldmont-${suffix}"
		["march_silvermont"]="${prefix}-x86_64-silvermont-${suffix}"
		["march_skylake"]="${prefix}-x86_64-skylake-${suffix}"
		["march_tigerlake"]="${prefix}-x86_64-tigerlake-${suffix}"
		["march_tremont"]="${prefix}-x86_64-tremont-${suffix}"
		["march_x86-64"]="${prefix}-x86_64-${suffix}"
		["march_znver1"]="${prefix}-x86_64-znver1-${suffix}"
	)

	# arm64
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_arm64=(
		["march_armv8-a"]="${prefix}-arm-${suffix}"
	)

	# arm
	# TODO(go/cros-arm64-plan): Remove once all boards have migrated to 64-bit user space.
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_arm=(
		["march_armv7-a"]="${prefix}-armv7-${suffix}"
		["march_armv8-a"]="${prefix}-armv7-armv8-${suffix}"
	)

	cros-binary_generate_src_uris march_uris_amd64 march_uris_arm64 march_uris_arm
}

# @FUNCTION: cros-camera_generate_facessd_package_SRC_URI
# @USAGE:
# @DESCRIPTION:
# Generate SRC_URI for facessd package by PV.
cros-camera_generate_facessd_package_SRC_URI() {
	local pv="$1"
	local prefix="gs://chromeos-localmirror/distfiles/chromeos-facessd-lib"
	local suffix="${pv}.tar.zst"
	# ABI march flag -> URI mappings
	# amd64
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_amd64=(
		["march_x86-64"]="${prefix}-x86_64-${suffix}"
		["march_alderlake"]="${prefix}-x86_64-alderlake-${suffix}"
		["march_bdver4"]="${prefix}-x86_64-bdver4-${suffix}"
		["march_corei7"]="${prefix}-x86_64-corei7-${suffix}"
		["march_goldmont"]="${prefix}-x86_64-goldmont-${suffix}"
		["march_silvermont"]="${prefix}-x86_64-silvermont-${suffix}"
		["march_skylake"]="${prefix}-x86_64-skylake-${suffix}"
		["march_tigerlake"]="${prefix}-x86_64-tigerlake-${suffix}"
		["march_tremont"]="${prefix}-x86_64-tremont-${suffix}"
		["march_znver1"]="${prefix}-x86_64-znver1-${suffix}"
	)

	# arm64
	# TODO: Update to armv8-a optimized build.
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_arm64=(
		["march_armv8-a"]="${prefix}-arm-${suffix}"
	)

	# arm
	# TODO(go/cros-arm64-plan): Remove once all boards have migrated to 64-bit user space.
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_arm=(
		["march_armv8-a"]="${prefix}-armv7-armv8-${suffix}"
		["march_armv7-a"]="${prefix}-armv7-${suffix}"
	)

	cros-binary_generate_src_uris march_uris_amd64 march_uris_arm64 march_uris_arm
}

# @FUNCTION: cros-camera_generate_gcam_package_SRC_URI
# @USAGE:
# @DESCRIPTION:
# Generate SRC_URI for gcam package by PV.
cros-camera_generate_gcam_package_SRC_URI() {
	local pv="$1"
	local prefix="gs://chromeos-localmirror/distfiles/chromeos-camera-libgcam"
	local suffix="${pv}.tar.zst"
	# ABI march flag -> URI mappings
	# amd64
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_amd64=(
		["march_alderlake"]="${prefix}-x86_64-alderlake-${suffix}"
		["march_skylake"]="${prefix}-x86_64-skylake-${suffix}"
	)

	# arm64
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_arm64=()

	# arm
	# TODO(go/cros-arm64-plan): Remove once all boards have migrated to 64-bit user space.
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_arm=()

	cros-binary_generate_src_uris march_uris_amd64 march_uris_arm64 march_uris_arm
}

# @FUNCTION: cros-camera_generate_portrait_mode_package_SRC_URI
# @USAGE:
# @DESCRIPTION:
# Generate SRC_URI for portrait mode package by PV.
cros-camera_generate_portrait_mode_package_SRC_URI() {
	local pv="$1"
	local prefix="gs://chromeos-localmirror/distfiles/portrait-processor-lib"
	local suffix="${pv}.tar.zst"
	# ABI march flag -> URI mappings
	# amd64
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_amd64=(
		["march_x86-64"]="${prefix}-x86_64-${suffix}"
	)

	# arm64
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_arm64=(
		["march_armv8-a"]="${prefix}-arm-${suffix}"
	)

	# arm
	# TODO(go/cros-arm64-plan): Remove once all boards have migrated to 64-bit user space.
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_arm=(
		["march_armv8-a"]="${prefix}-armv7-${suffix}"
		["march_armv7-a"]="${prefix}-armv7-${suffix}"
	)

	cros-binary_generate_src_uris march_uris_amd64 march_uris_arm64 march_uris_arm
}
