# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# @ECLASS: cros-zephyr-utils.eclass
# @MAINTAINER:
# ChromeOS Test Framework Team
# @BUGREPORTS:
# Please report bugs via
# https://b.corp.google.com/issues/new?component=1034624
# @VCSURL: https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/eclass/@ECLASS@
# @BLURB: handling gtest functional test packages
# @DESCRIPTION:
# Helper functions for manipulating metadata for gtest and Crosier tests.

if [[ -z "${_ECLASS_GTEST}" ]]; then
_ECLASS_GTEST="1"

# Check for EAPI 7+.
case "${EAPI:-0}" in
[0123456]) die "unsupported EAPI (${EAPI}) in eclass (${ECLASS})" ;;
esac

inherit cros-constants

# @ECLASS-VARIABLE: GTEST_METADATA_INSTALL_DIR
# @DESCRIPTION:
# Location of the appropriate metadata install directory.
: "${GTEST_METADATA_INSTALL_DIR:=/usr/local/build/gtest}"

# @ECLASS-VARIABLE: GTEST_METADATA_OUTPUT_FILE
# @DESCRIPTION:
# Name of the binary proto output file containing all test metadata.
: "${GTEST_METADATA_OUTPUT_FILE:=gtest_metadata.pb}"

# Directory of gtest metadata compiler script.
GTEST_BINARY_DIR="${WORKDIR}/${P}/platform/dev/test/gtest"

# @FUNCTION: install_gtest_metadata
# @DESCRIPTION:
# Compiles and installs individual gtest metadata files. Generates proto file
# for every metadata input file.
install_gtest_metadata() {
	[[ $# -eq 0 ]] && die "Missing argument: metadata input file(s)."

	local metadata_files=()
	local f
	for f in "$@"; do
 		local meta_file=$(basename "${f}" .yaml).pb
		"${GTEST_BINARY_DIR}"/generate_gtest_metadata.py \
			--output_file "${meta_file}" \
			--yaml_schema "${GTEST_BINARY_DIR}"/gtest_schema.yaml \
			"${f}" \
			|| die "Failed to generate metadata for '${f}'!"

		metadata_files+=("${meta_file}")
	done

	(
		insinto "${GTEST_METADATA_INSTALL_DIR}"
		doins "${metadata_files[@]}"
	)
}

# @FUNCTION: install_all_gtest_metadata
# @DESCRIPTION:
# Compiles and installs gtest metadata files. Generates single proto file
# containing metadata for all input files.
install_all_gtest_metadata() {
	[[ $# -eq 0 ]] && die "Missing argument: metadata input file(s)."

	local args=( "$@" )
	"${GTEST_BINARY_DIR}"/generate_gtest_metadata.py \
		--output_file "${GTEST_METADATA_OUTPUT_FILE}" \
		--yaml_schema "${GTEST_BINARY_DIR}"/gtest_schema.yaml \
		"${args[@]}" \
		|| die "Failed to generate metadata file '${GTEST_METADATA_OUTPUT_FILE}'!"

	(
		insinto "${GTEST_METADATA_INSTALL_DIR}"
		doins "${GTEST_METADATA_OUTPUT_FILE}"
	)
}

fi  # _ECLASS_GTEST
