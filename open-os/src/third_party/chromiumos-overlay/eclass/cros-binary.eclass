# Copyright 2012 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

#
# Original Author: The ChromiumOS Authors <chromium-os-dev@chromium.org>
# Purpose: Install binary packages for Chromium OS
#

# Reject old users of cros-binary eclass that expected us to download files
# directly rather than going through SRC_URI.
cros-binary_dead_usage() {
	die "You must add files to SRC_URI now and install them manually"
}
if [[ ${CROS_BINARY_STORE_DIR:+set} == "set" ||
      ${CROS_BINARY_SUM:+set} == "set" ||
      ${CROS_BINARY_FETCH_REQUIRED:+set} == "set" ||
      ${CROS_BINARY_INSTALL_FLAGS:+set} == "set" ]]; then
	cros-binary_dead_usage
fi

# @ECLASS-FUNCTION: cros-binary_add_uri
# @DESCRIPTION:
# Add a fetch uri to SRC_URI for the given uri.  See
# CROS_BINARY_URI for what is accepted.  Note you cannot
# intermix a non-rewritten ssh w/ (http|https|gs).
cros-binary_add_uri()
{
	if [[ $# -ne 1 ]]; then
		die "cros-binary_add_uri takes exactly one argument; $# given."
	fi
	local uri="$1"
	case "${uri}" in
		http://*|https://*|gs://*)
			SRC_URI+=" ${uri}"
			;;
		*)
			die "Unknown protocol: ${uri}"
			;;
	esac
	RESTRICT+=" mirror"

	if [[ ${uri} =~ -r[0-9]+\.tbz2 ]]; then
		ewarn "${P}: Tarballs should not encode ebuild rev numbers (-r#)."
		ewarn "The ebuild revision field is only for changes to the ebuild itself."
		ewarn "If you want to update the source tarball, update the PV instead."
		ewarn "  bad: foo-0.0.1-r8.tbz2 or foo-0.0.1.tbz2 -> foo-0.0.1-r1.tbz2"
		ewarn " good: foo-0.0.8.tbz2    or foo-0.0.1.tbz2 -> foo-0.0.2.tbz2"
	fi
}

# @ECLASS-FUNCTION: cros-binary_add_gs_uri
# @DESCRIPTION:
# Wrapper around cros-binary_add_uri.  Invoked with 3 arguments;
# the bcs user, the overlay, and the filename (or bcs://<uri> for
# backwards compatibility).
cros-binary_add_gs_uri() {
	if [[ $# -ne 3 ]]; then
		die "cros-binary_add_gs_uri needs 3 arguments; $# given."
	fi
	# Strip leading bcs://...
	[[ "${3:0:6}" == "bcs://" ]] && set -- "${1}" "${2}" "${3#bcs://}"
	cros-binary_add_uri "gs://chromeos-binaries/HOME/$1/$2/$3"
}

# @ECLASS-FUNCTION: cros-binary_add_overlay_uri
# @DESCRIPTION:
# Wrapper around cros-binary_add_gs_uri.  Invoked with 2 arguments;
# the basic board target (x86-alex for example), and the filename; that filename
# is automatically prefixed with "${CATEGORY}/${PN}/" .
cros-binary_add_overlay_uri() {
	if [[ $# -ne 2 ]]; then
		die "cros-binary_add_overlay_uri needs 2 arguments; $# given."
	fi
	cros-binary_add_gs_uri bcs-"$1" overlay-"$1" "${CATEGORY}/${PN}/$2"
}

# @ECLASS-VARIABLE: CROS_BINARY_URI
# @DESCRIPTION:
# URI for the binary may be one of:
#   http://
#   https://
#   ssh://
#   gs://
#   file:// (file is relative to the files directory)
# Additionally, all bcs ssh:// urls are rewritten to gs:// automatically
# the appropriate GS bucket- although cros-binary_add_uri is the preferred
# way to do that.
# TODO: Deprecate this variable's support for ssh and http/https.
: "${CROS_BINARY_URI:=}"
if [[ -n "${CROS_BINARY_URI}" ]]; then
	cros-binary_add_uri "${CROS_BINARY_URI}"
fi

# @ECLASS-VARIABLE: CROS_BINARY_LOCAL_URI_BASE
# @DESCRIPTION:
# Optional URI to override CROS_BINARY_URI location.  If this variable
# is used the filename from CROS_BINARY_URI will be used, but the path
# to the binary will be changed.
: "${CROS_BINARY_LOCAL_URI_BASE:=}"

# Check for EAPI 7+.
case "${EAPI:-0}" in
[0123456]) die "unsupported EAPI (${EAPI}) in eclass (${ECLASS})" ;;
esac

cros-binary_check_file() {
	cros-binary_dead_usage
}

cros-binary_fetch() {
	cros-binary_dead_usage
}

cros-binary_src_unpack() {
	cros-binary_dead_usage
}

cros-binary_src_install() {
	cros-binary_dead_usage
}

# Ordered lists of |-march=| flag values, which are used to optimize
# builds/libraries.
# Later microarchitectures are assumed to be backwards-compatible with earlier
# levels, with each generation's feature list being a superset building on
# prior generations. This assumption allows earlier march flags to be used as
# fallbacks if the requesting library doesn't support the later (more recent)
# microarchitecture.

# @ECLASS-VARIABLE: CROS_BINARY_PRIORITIZED_MARCHS_X86_64
# @INTERNAL
# @DESCRIPTION:
# Ordered list of x86-64 levels in terms of features.
# https://en.wikipedia.org/wiki/X86-64#Microarchitecture_levels
# Shellcheck can't understand namedrefs in the array of arrays usage.
# shellcheck disable=SC2034
CROS_BINARY_PRIORITIZED_MARCHS_X86_64=(
	"march_x86-64"
	"march_x86-64-v2"
	"march_x86-64-v3"
	"march_x86-64-v4"
)

# @ECLASS-VARIABLE: CROS_BINARY_PRIORITIZED_MARCHS_AMD
# @INTERNAL
# @DESCRIPTION: Ordered list of AMD microarchitectures.
# Shellcheck can't understand namedrefs in the array of arrays usage.
# shellcheck disable=SC2034
CROS_BINARY_PRIORITIZED_MARCHS_AMD=(
	"march_x86-64"
	"march_x86-64-v2"
	"march_x86-64-v3"
	"march_bdver4"
	"march_znver1"
	"march_znver2"
	"march_znver3"
	"march_x86-64-v4"
	"march_znver4"
)

# @ECLASS-VARIABLE: CROS_BINARY_PRIORITIZED_MARCHS_INTEL_ATOM
# @INTERNAL
# @DESCRIPTION: Ordered list of Intel Atom microarchitectures.
# This follows the LLVM hierarchy of Intel microarchitectures:
#   https://llvm.org/doxygen/X86TargetParser_8cpp_source.html
# Shellcheck can't understand namedrefs in the array of arrays usage.
# shellcheck disable=SC2034
CROS_BINARY_PRIORITIZED_MARCHS_INTEL_ATOM=(
	"march_x86-64"
	"march_x86-64-v2"
	"march_silvermont"
	"march_goldmont"
	"march_goldmont-plus"
	"march_tremont"
	"march_x86-64-v3"
	"march_alderlake"
	"march_meteorlake"
)

# @ECLASS-VARIABLE: CROS_BINARY_PRIORITIZED_MARCHS_INTEL_BIG
# @INTERNAL
# @DESCRIPTION: Ordered list of Intel microarchitectures big core.
# This follows the LLVM hierarchy of Intel microarchitectures:
#   https://llvm.org/doxygen/X86TargetParser_8cpp_source.html
# Shellcheck can't understand namedrefs in the array of arrays usage.
# shellcheck disable=SC2034
CROS_BINARY_PRIORITIZED_MARCHS_INTEL_BIG=(
	"march_x86-64"
	"march_x86-64-v2"
	"march_silvermont"
	"march_x86-64-v3"
	"march_skylake"
	"march_x86-64-v4"
)

# @ECLASS-VARIABLE: CROS_BINARY_PRIORITIZED_MARCHS_ARM
# @INTERNAL
# @DESCRIPTION: Ordered list of 32-bit ARM microarchitectures.
# Shellcheck can't understand namedrefs in the array of arrays usage.
# shellcheck disable=SC2034
CROS_BINARY_PRIORITIZED_MARCHS_ARM=(
	"march_armv7-a"
	"march_armv8-a"
)

# @ECLASS-VARIABLE: CROS_BINARY_PRIORITIZED_MARCHS_ARM64
# @INTERNAL
# @DESCRIPTION: Ordered list of 64-bit ARM microarchitectures.
# Shellcheck can't understand namedrefs in the array of arrays usage.
# shellcheck disable=SC2034
CROS_BINARY_PRIORITIZED_MARCHS_ARM64=(
	"march_armv8-a"
)

# @ECLASS-VARIABLE: CROS_BINARY_ALL_PRIORITIZED_MARCHS_NAMES
# @INTERNAL
# @DESCRIPTION: Ordered list of all supported microarchitectures.
# |CROS_BINARY_PRIORITIZED_MARCHS_X86_64| must be first, since those march values
# exist in the remaining x86 lists as well as fallbacks.
# TODO(go/cros-arm64-plan): Remove |CROS_BINARY_PRIORITIZED_MARCHS_ARM|
# once all boards have migrated to 64-bit user space.
# Shellcheck can't understand namedrefs as function arguments.
# shellcheck disable=SC2034
CROS_BINARY_ALL_PRIORITIZED_MARCHS_NAMES=(
	"CROS_BINARY_PRIORITIZED_MARCHS_X86_64"
	"CROS_BINARY_PRIORITIZED_MARCHS_AMD"
	"CROS_BINARY_PRIORITIZED_MARCHS_INTEL_ATOM"
	"CROS_BINARY_PRIORITIZED_MARCHS_INTEL_BIG"
	"CROS_BINARY_PRIORITIZED_MARCHS_ARM"
	"CROS_BINARY_PRIORITIZED_MARCHS_ARM64"
)

# @ECLASS-VARIABLE: CROS_BINARY_PRIORITIZED_MARCHS_ABI_AMD64
# @INTERNAL
# @DESCRIPTION: Ordered list of all amd64 supported microarchitectures.
# |CROS_BINARY_PRIORITIZED_MARCHS_X86_64| must be first, since those march values
# exist in the remaining x86 lists as well as fallbacks.
CROS_BINARY_PRIORITIZED_MARCHS_ABI_AMD64=(
	"CROS_BINARY_PRIORITIZED_MARCHS_X86_64"
	"CROS_BINARY_PRIORITIZED_MARCHS_AMD"
	"CROS_BINARY_PRIORITIZED_MARCHS_INTEL_ATOM"
	"CROS_BINARY_PRIORITIZED_MARCHS_INTEL_BIG"
)

# @ECLASS-VARIABLE: CROS_BINARY_PRIORITIZED_MARCHS_ABI_ARM64
# @INTERNAL
# @DESCRIPTION: Ordered list of all arm64 supported microarchitectures.
# TODO(go/cros-arm64-plan): Remove |CROS_BINARY_PRIORITIZED_MARCHS_ARM|
# once all boards have migrated to 64-bit user space.
CROS_BINARY_PRIORITIZED_MARCHS_ABI_ARM64=(
	"CROS_BINARY_PRIORITIZED_MARCHS_ARM64"
)

# @ECLASS-VARIABLE: CROS_BINARY_PRIORITIZED_MARCHS_ABI_ARM
# @INTERNAL
# @DESCRIPTION: Ordered list of all arm64 supported microarchitectures.
# TODO(go/cros-arm64-plan): Remove once all boards have migrated to 64-bit user space.
CROS_BINARY_PRIORITIZED_MARCHS_ABI_ARM=(
	"CROS_BINARY_PRIORITIZED_MARCHS_ARM"
)

# @ECLASS-FUNCTION: cros-binary_get_ordered_unique_marchs
# @DESCRIPTION:
# Get the unique |march_| flag values from the array of march arrays passed in,
# preserving order.
cros-binary_get_ordered_unique_marchs() {
	local -n cb_goum_ordered_marchs=$1
	local marches=()

	local march_list
	for march_list in "${cb_goum_ordered_marchs[@]}"; do
		# Skip march values that were already included.
		local -n curr_prioritized_marchs=${march_list}
		local curr_march
		for curr_march in "${curr_prioritized_marchs[@]}"; do
			if ! has "${curr_march}" "${marches[@]}"; then
				marches+=( "${curr_march}" )
			fi
		done
	done

	echo "${marches[@]}"
}

# @ECLASS-FUNCTION: cros-binary_get_required_marchs
# @DESCRIPTION:
# Get the full list of required USE |march_| flag values, without duplicates.
cros-binary_get_required_marchs() {
	cros-binary_get_ordered_unique_marchs CROS_BINARY_ALL_PRIORITIZED_MARCHS_NAMES
}

# @ECLASS-VARIABLE: CROS_BINARY_MARCHS_USE
# @DESCRIPTION: List of all march_ USE flags.
# Used by ebuilds importing this eclass.
# shellcheck disable=SC2034
CROS_BINARY_MARCHS_USE="$(cros-binary_get_required_marchs)"

# @ECLASS-VARIABLE: CROS_BINARY_MARCHS_REQUIRED_USE
# @DESCRIPTION: List of all exactly-one required march_ USE flags.
# Used by ebuilds importing this eclass.
# shellcheck disable=SC2034
CROS_BINARY_MARCHS_REQUIRED_USE="^^ ( $(cros-binary_get_required_marchs) )"

# @ECLASS-FUNCTION: cros-binary_get_required_marchs_for_abi
# @DESCRIPTION:
# Get the full list of required USE |march_| flag values, without duplicates, so the passed in ABI.
cros-binary_get_required_marchs_for_abi() {

	case $1 in
	amd64)
		cros-binary_get_ordered_unique_marchs CROS_BINARY_PRIORITIZED_MARCHS_ABI_AMD64
		;;
	arm64)
		cros-binary_get_ordered_unique_marchs CROS_BINARY_PRIORITIZED_MARCHS_ABI_ARM64
		;;
	# TODO(go/cros-arm64-plan): Remove once all boards have migrated to 64-bit user space.
	arm)
		cros-binary_get_ordered_unique_marchs CROS_BINARY_PRIORITIZED_MARCHS_ABI_ARM
		;;
	*)
		die "Unknown ABI '$1'"
		;;
	esac
}

# @ECLASS-FUNCTION: _cros-binary_find_march_fallback
# @INTERNAL
# @DESCRIPTION:
# Find the fallback for the passed in march value from the list of built marchs.
_cros-binary_find_march_fallback() {
	# Named variables need unique names; prefix with function name.
	local -n cb_fmf_march=$1
	local -n cb_fmf_library_marchs=$2
	local -n cb_fmf_prioritized_marchs=$3

	local found_march=false

	# Iterate in reverse order, to find the compatible march value with the
	# most features enabled.
	local i
	for (( i = ${#cb_fmf_prioritized_marchs[@]} - 1 ; i >= 0 ; i-- )) ; do
		local curr_prioritized_march="${cb_fmf_prioritized_marchs[${i}]}"
		# 1. Search the prioritized march list to find the exact march we would
		# use, if the library had generated it.
		if [[ "${curr_prioritized_march}" == "${cb_fmf_march}" ]]; then
			found_march=true
		# 2. Continue searching in reverse order to find the best fallback
		# march that the library generated.
		elif ${found_march}; then
			if has "${curr_prioritized_march}" "${cb_fmf_library_marchs[@]}"; then
				# TODO (b/300996135): Re-enable once we know the best way to
				# inform developers to update their libraries.
				# if [[ "${curr_prioritized_march}" == "march_x86-64" ]]; then
					# ewarn "${P}: '${cb_fmf_march}' is using the unoptimized 'march_x86-64' build. Please update the package to support additional architecture-specific binaries."
					# ewarn "${P}: To see affected boards: cros query boards -f '\"${cb_fmf_march}\" in use_flags'"
				# fi
				echo "${curr_prioritized_march}"
				return
			fi
		fi
	done
}

# @ECLASS-FUNCTION: cros-binary_get_compatible_march_for_abi
# @DESCRIPTION:
# Find the compatible march for the passed in value from the list of built marchs.
# This may be the exact same march, or a fallback value.
cros-binary_get_compatible_march_for_abi() {
	# Named variables need unique names; prefix with function name.
	local cb_gcm_abi=$1
	local -n cb_gcm_march=$2
	local -n cb_gcm_library_marchs=$3

	# If the passed in list of supported marchs matches directly, use it.
	if has "${cb_gcm_march}" "${cb_gcm_library_marchs[@]}"; then
		echo "${cb_gcm_march}"
		return
	fi

	local prioritized_marchs=()
	case ${cb_gcm_abi} in
	amd64)
		prioritized_marchs=( "${CROS_BINARY_PRIORITIZED_MARCHS_ABI_AMD64[@]}" )
		;;
	arm64)
		prioritized_marchs=( "${CROS_BINARY_PRIORITIZED_MARCHS_ABI_ARM64[@]}" )
		;;
	# TODO(go/cros-arm64-plan): Remove once all boards have migrated to 64-bit user space.
	arm)
		prioritized_marchs=( "${CROS_BINARY_PRIORITIZED_MARCHS_ABI_ARM[@]}" )
		;;
	*)
		die "Unknown ABI '${cb_gcm_abi}'"
		;;
	esac

	local i
	for i in "${prioritized_marchs[@]}"; do
		# Use namerefs to iterate over the array of arrays.
		# https://unix.stackexchange.com/a/546374
		local -n curr_prioritized_marchs=${i}
		if has "${cb_gcm_march}" "${curr_prioritized_marchs[@]}"; then
			_cros-binary_find_march_fallback cb_gcm_march cb_gcm_library_marchs curr_prioritized_marchs
			return
		fi
	done

	eerror "For ABI '${cb_gcm_abi}': unknown march value '${cb_gcm_march}', cb_gcm_library_marchs = '${cb_gcm_library_marchs[*]}'"
	die "Unknown march value"
}

# @ECLASS-FUNCTION: cros-binary_generate_src_uris
# @DESCRIPTION:
# Build up the list of source URIs for every known march value, falling back to
# the appropriate binary if the library doesn't have a matching build.
# The passed in map contains [key]=value: [march]=URI
# @EXAMPLE:
# To generate SRC_URI for a library, in the ebuild do:
# 1. Declare the necessary associative arrays (maps) of march_ -> URI.
# 2. Call cros-binary_generate_src_uris(), passing the associative arrays.
#
# @CODE
#  declare -A march_uris_amd64=(
#    ["march_x86-64"]="gs://chromeos-localmirror/distfiles/lib_amd64-${PV}.tar.gz"
#    ["march_skylake"]="gs://chromeos-localmirror/distfiles/lib_skylake-${PV}.tar.gz"
#    ["march_znver1"]="gs://chromeos-localmirror/distfiles/lib_znver1-${PV}.tar.gz"
#  )
#  declare -A march_uris_arm64=(
#    ["march_armv8-a"]="gs://chromeos-localmirror/distfiles/lib_arm64-${PV}.tar.gz"
#  )
#  declare -A march_uris_arm=(
#    ["march_armv7-a"]="gs://chromeos-localmirror/distfiles/lib_arm32-${PV}.tar.gz"
#    ["march_armv8-a"]="gs://chromeos-localmirror/distfiles/lib_arm32-${PV}.tar.gz"
#  )
#  SRC_URI="$(cros-binary_generate_src_uris march_uris_amd64 march_uris_arm64 march_uris_arm)"
# @CODE
cros-binary_generate_src_uris() {
	local -n cb_gsu_march_uris_amd64=$1
	local -n cb_gsu_march_uris_arm64=$2
	local -n cb_gsu_march_uris_arm=$3

	# Build up the SRC_URI list by mapping all known march_ values to the most
	# compatible (possibly fallback) march value, based on what the library
	# supports, grouped by ABI.
	# The generated SRC_URI uses nested |use?| flag checks to support the same
	# march flag being used with different ARCH (ABI) flags, requiring a
	# different build (URI) for each. An example SRC_URI:
	# 	SRC_URI='
	# 		amd64? (
	# 			march_x86-64? ( gs://.../libsoda_chromeos_amd64-0.0.45.tar.gz )
	# 			march_x86-64-v2? ( gs://.../libsoda_chromeos_amd64-0.0.45.tar.gz )
	#           ...
	# 			march_alderlake? ( gs://.../libsoda_chromeos_alderlake-0.0.45.tar.gz )
	# 			march_meteorlake? ( gs://.../libsoda_chromeos_alderlake-0.0.45.tar.gz )
	# 		)
	# 		arm64? (
	# 			march_armv8-a? ( gs://.../libsoda_chromeos_arm64-0.0.45.tar.gz )
	# 		)
	# 		arm? (
	# 			march_armv7-a? ( gs://.../libsoda_chromeos_arm32-0.0.45.tar.gz )
	# 			march_armv8-a? ( gs://.../libsoda_chromeos_arm32-0.0.45.tar.gz )
	# 		)
	# 	'

	# TODO: See if we can eliminate some of the code duplication by using named
	# refs to loop over the supported ARCH values (i.e., amd64, arm, arm64),
	# rather than duplicating the loops.

	# amd64
	local i
	local cb_gsu_library_marchs=( "${!cb_gsu_march_uris_amd64[@]}" )
	if [[ ${#cb_gsu_library_marchs[@]} -gt 0 ]]; then
		local src_uris="amd64? ( "
		for i in $(cros-binary_get_required_marchs_for_abi amd64); do
			# |curr_march| is passed as a named variable (nameref), which shellcheck can't recognize.
			# shellcheck disable=SC2034
			local curr_march="${i}"
			local compatible_march=$(cros-binary_get_compatible_march_for_abi amd64 curr_march cb_gsu_library_marchs)
			if [[ -n "${compatible_march}" ]]; then
				local uri=${cb_gsu_march_uris_amd64[${compatible_march}]}
				src_uris+=" ${i}? ( ${uri} )"
			fi
		done
		src_uris+=" ) "
	fi

	# arm64
	cb_gsu_library_marchs=( "${!cb_gsu_march_uris_arm64[@]}" )
	if [[ ${#cb_gsu_library_marchs[@]} -gt 0 ]]; then
		src_uris+="arm64? ( "
		for i in $(cros-binary_get_required_marchs_for_abi arm64); do
			# |curr_march| is passed as a named variable (nameref), which shellcheck can't recognize.
			# shellcheck disable=SC2034
			local curr_march="${i}"
			local compatible_march=$(cros-binary_get_compatible_march_for_abi arm64 curr_march cb_gsu_library_marchs)
			if [[ -n "${compatible_march}" ]]; then
				local uri=${cb_gsu_march_uris_arm64[${compatible_march}]}
				src_uris+=" ${i}? ( ${uri} )"
			fi
		done
		src_uris+=" ) "
	fi

	# arm (32-bit)
	# TODO(go/cros-arm64-plan): Remove once all boards have migrated to 64-bit user space.
	cb_gsu_library_marchs=( "${!cb_gsu_march_uris_arm[@]}" )
	if [[ ${#cb_gsu_library_marchs[@]} -gt 0 ]]; then
		src_uris+="arm? ( "
		for i in $(cros-binary_get_required_marchs_for_abi arm); do
			# |curr_march| is passed as a named variable (nameref), which shellcheck can't recognize.
			# shellcheck disable=SC2034
			local curr_march="${i}"
			local compatible_march=$(cros-binary_get_compatible_march_for_abi arm curr_march cb_gsu_library_marchs)
			if [[ -n "${compatible_march}" ]]; then
				local uri=${cb_gsu_march_uris_arm[${compatible_march}]}
				src_uris+=" ${i}? ( ${uri} )"
			fi
		done
		src_uris+=" ) "
	fi

	echo "${src_uris}"
}
