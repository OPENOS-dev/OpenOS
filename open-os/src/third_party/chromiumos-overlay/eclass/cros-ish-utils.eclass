# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# @ECLASS: cros-ish-utils.eclass
# @MAINTAINER:
# ChromiumOS Firmware Team
# @BUGREPORTS:
# Please report bugs via
# https://b.corp.google.com/issues/new?component=1037860&template=1600056
# @VCSURL: https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/eclass/@ECLASS@
# @BLURB: helper eclass for building ChromiumOS ISH firmware
# @DESCRIPTION:
# Common helper functions for working with ChromiumOS Zephyr ISH firmware.

# @VARIABLE: CROS_ISH_UTILS_INSTALL_PATH
# @DESCRIPTION:
# Sets the absolute path where the ISH firmware will be installed.
# If this is not set, the installation defaults to /lib/firmware/intel/ish
: "${CROS_ISH_UTILS_INSTALL_PATH:=}"

# @VARIABLE: CROS_ISH_UTILS_NAME_MAPPING_FN
# @DESCRIPTION:
# Specifies the name of an ebuild-defined function to call for custom firmware
# naming. The function will receive 2 arguments: project name and firmware name.
# The function should echo the new filename to standard output.
#
# Example:
# my_name_mapping_function() {
#   local project="$1"
#   local firmware_name="$2"
#
#   echo "intel-${project}-${firmware_name}".bin
: "${CROS_ISH_UTILS_NAME_MAPPING_FN:=}"

if [[ -z "${_ECLASS_CROS_ISH_UTILS}" ]]; then
_ECLASS_CROS_ISH_UTILS="1"

# Check for EAPI 7+.
case "${EAPI:-0}" in
0|1|2|3|4|5|6) die "unsupported EAPI (${EAPI}) in eclass (${ECLASS})" ;;
*) ;;
esac

inherit cros-unibuild

LICENSE="Apache-2.0 BSD-Google"
IUSE="cros_host zephyr_ish"
REQUIRED_USE="|| ( cros_host unibuild )"

# Loops through each ISH build configuration and calls the function specified in
# the first argument passing the project name and the ISH target name as
# arguments.
_cros_ish_foreach() {
	local func_name="$1"
	while read -r project && read -r ish_name; do
		if [[ -z "${ish_name}" ]]; then
			continue
		fi
		"${func_name}" "${project}" "${ish_name}" || die
	done < <(cros_config_host "get-firmware-build-combinations" ish || die)
}

# See cros-ish-utils-src-unpack for documentation
_cros_ish_unpack_if_in_srcs() {
	local project="$1"
	local firmware_name="$2"
	local bundle="$(cros_config_host "get-firmware-version" "${project}" ish)"

	if [[ -z "${bundle}" ]]; then
		return
	fi

	unpack "${bundle}.tbz2" || die
	mkdir -p "${S}/${project}" || die
	mv "${WORKDIR}/ish_fw.bin" "${S}/${project}/"
	mv "${WORKDIR}/component_manifest.json" "${S}/${project}/"
}

_cros_ish_get_install_path() {
	if [[ -n "${CROS_ISH_UTILS_INSTALL_PATH}" ]]; then
		echo "${CROS_ISH_UTILS_INSTALL_PATH}"
	else
		echo "/lib/firmware/intel/ish"
	fi
}

_cros_ish_get_output_name() {
	local project="$1"
	local firmware_name="$2"

	if declare -F "${CROS_ISH_UTILS_NAME_MAPPING_FN}" &>/dev/null; then
		"${CROS_ISH_UTILS_NAME_MAPPING_FN}" "${project}" "${firmware_name}" || die
	else
		echo "${firmware_name//-/_}.bin"
	fi
}

_cros_ish_get_bin_path() {
	local project="$1"
	local firmware_name="$2"

	if [[ -e "${S}/${project}/ish_fw.bin" ]]; then
		echo "${S}/${project}"
	else
		echo "${ROOT}/firmware/${project}/${firmware_name}"
	fi
}

# See cros-ish-utils-src-install for documentation
_cros_ish_install() {
	local project="$1"
	local firmware_name="$2"
	local output_name=$(_cros_ish_get_output_name "${project}" "${firmware_name}")
	local install_path=$(_cros_ish_get_install_path)
	local bin_path=$(_cros_ish_get_bin_path "${project}" "${firmware_name}")

	insinto "${install_path}"
	newins "${bin_path}/ish_fw.bin" "${output_name}"

	# Install the component manifest
	insinto "/usr/share/cme/ish/${firmware_name}"
	doins "${bin_path}/component_manifest.json"
}

# Unpacks the .tbz2 firmware if pinned.
# This function first checks the current version of the ISH firmware using the
# project. It then checks if that bundle was includes via the SRCS of the
# ebuild. If so, it will be unpacked into ${S}/${project}/.
# Args:
#   1: The project name (example: trulo)
#   2. The ISH firmware name (example: trulo-ish)
cros-ish-utils-src-unpack() {
	if use !zephyr_ish; then
		return
	fi
	mkdir -p "${S}"
	_cros_ish_foreach "_cros_ish_unpack_if_in_srcs"
}

# Installs the firmware into the right /lib directory
# This function first checks if the firmware is available from the src_unpack
# step. If so, the unpacked firmware is used as pinning is preferred. Otherwise,
# the latest ToT build is used.
#
# The behavior of this function can be altered by 2 variables:
#   CROS_ISH_UTILS_INSTALL_PATH - which overrides the install directory
#   CROS_ISH_UTILS_NAME_MAPPING_FN - which allows the ebuild custom name mapping
#
# The default path is: /lib/firmware/intel/ish
# The default name is: The firmware name with '-' replaced with '_'
cros-ish-utils-src-install() {
	if use !zephyr_ish; then
		return
	fi
	_cros_ish_foreach "_cros_ish_install"
}

fi
