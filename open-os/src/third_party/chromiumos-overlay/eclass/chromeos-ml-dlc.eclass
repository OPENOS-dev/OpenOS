# Copyright 2025 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2.

# @ECLASS: chromeos-ml-dlc.eclass
# @MAINTAINER:
# ChromeOS ODML Foundations Eng Team
# @BUGREPORTS:
# Please report bugs via
# https://issuetracker.google.com/issues/new?component=1445284
# @VCSURL: https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/eclass/@ECLASS@
# @BLURB: helper eclass for building ChromeOS ML DLC packages
# @DESCRIPTION:
# A lot of ChromeOS ML prebuilt DLC packages are managed in the same way.
# This automates a lot of that common stuff in one place.
#
# The 9999 version of these packages will only be used for:
# 1. Local testing.
# 2. Building the prebuilt DLC for release.
#
# The local testing flow these packages:
# 1. Upload the source data and put the link in CHROMEOS_ML_DLC_9999_SRC_URI.
# 2. Change the content of chromeos-ml-dlc-9999_* functions when needed.
# 3. Build the 9999 version of it.
# 4. Deploy the 9999 version to the device for testing.
#
# The prebuilt DLC release flow these packages:
# 1. Upload the source data and put the link in CHROMEOS_ML_DLC_9999_SRC_URI.
# 2. Change the content of chromeos-ml-dlc-9999_* functions when needed.
# 3. Update/Bump the CHROMEOS_ML_VERSION.
# 4. Build the 9999 version of it with different USE flags combinations.
# 5. Use cros_generate_dlc_artifacts to upload the artifacts of 9999 version.
# 6. Update the Manifest for the new prebuilt DLC artifacts.
# 7. Uprev the stable version of ebuild for release.

if [[ -z "${_ECLASS_CHROMEOS_ML_DLC}" ]]; then
_ECLASS_CHROMEOS_ML_DLC=1

case ${EAPI:-0} in
[0123456]) die "Unsupported EAPI=${EAPI:-0} (too old) for ${ECLASS}" ;;
esac

inherit dlc-prebuilt unpacker

# @ECLASS-VARIABLE: CHROMEOS_ML_VERSION
# @DESCRIPTION:
# Specify the version of this chromeos ML DLC package.
# This will be used for the prebuilt DLC version.
# This number should be bumped when the content of prebuilt DLC changed.
: "${CHROMEOS_ML_VERSION:=${PV}}"

# @ECLASS-VARIABLE: CHROMEOS_ML_DLC_9999_SRC_URI
# @DESCRIPTION:
# The SRC_URI for the "9999 repack for prebuilt DLC" stage of this package.
: "${CHROMEOS_ML_DLC_9999_SRC_URI:=}"

# @FUNCTION: chromeos-ml-dlc-9999_src_unpack
# @DESCRIPTION:
# The src_unpack for the "9999 repack for prebuilt DLC" stage of this package.

# @FUNCTION: chromeos-ml-dlc-9999_src_install
# @DESCRIPTION:
# The src_install for the "9999 repack for prebuilt DLC" stage of this package.

# @ECLASS-VARIABLE: CHROMEOS_ML_BUCKET
# @DEFAULT_UNSET
# @DESCRIPTION:
# The DLC bucket prefix to pull from, developers can override this.
: "${CHROMEOS_ML_BUCKET:=gs://chromeos-localmirror-private/dlc-images/${PN}/package}"

# @FUNCTION: chromeos-ml-dlc-dlc_description
# @DESCRIPTION:
# Should be used to generate the DLC_DESCRIPTION in the DLC package.
chromeos-ml-dlc-dlc_description() {
	echo "${DESCRIPTION} ${CHROMEOS_ML_VERSION}"
}

# @FUNCTION: chromeos-ml-dlc-expend_flag
# @DESCRIPTION:
# Enumerate all possible USE flag combinations, and generate the SRC_URI mapping for them.
chromeos-ml-dlc-expend_flag() {
	# shellcheck disable=SC2206   # Expand on whitespace.
	local iuse_array=(${IUSE})
	local i=$1
	local prefix=$2
	local append=$3
	local suffix=$4
	local bucket=${CHROMEOS_ML_BUCKET}/${CHROMEOS_ML_VERSION}-${append}

	[[ -z "${DLC_META_ARTIFACT}" ]] && die "DLC_META_ARTIFACT undefined"

	if [[ ${i} -ge ${#iuse_array[@]} ]]; then
		echo "${prefix} ${bucket}/${DLC_META_ARTIFACT} -> ${PN}-${CHROMEOS_ML_VERSION}-${append}-${DLC_META_ARTIFACT} ${suffix}"
		echo "${prefix} ${bucket}/dlc.img -> ${PN}-${CHROMEOS_ML_VERSION}-${append}-dlc.img ${suffix}"
		return
	fi

	local flag=${iuse_array[i]}
	flag=${flag#[-+]}

	chromeos-ml-dlc-expend_flag $((i+1)) "${prefix} ${flag}? (" "${append}P${flag}" "${suffix} )"
	chromeos-ml-dlc-expend_flag $((i+1)) "${prefix} !${flag}? (" "${append}N${flag}" "${suffix} )"
}

# @FUNCTION: chromeos-ml-dlc-get_src_uri
# @DESCRIPTION:
# Generate the SRC_URI for the ChromeOS ML DLC package.
# It will install the DLC prepare SRC_URI for unstable package.
# Or use the prebuilt DLC for stable package.
chromeos-ml-dlc-get_src_uri() {
	if [[ "${PV}" == "9999" ]]; then
		echo "${CHROMEOS_ML_DLC_9999_SRC_URI}"
	else
		[[ -z "${DLC_META_ARTIFACT}" ]] && die "DLC_META_ARTIFACT undefined"
		chromeos-ml-dlc-expend_flag 0 "" "" ""
	fi
}

# @FUNCTION: chromeos-ml-dlc_src_unpack
# @DESCRIPTION:
# The src_unpack function for the ChromeOS ML DLC package.
# It will run the chromeos-ml-dlc-9999 unpack function in the ebuild file for unstable package.
# Or unpack the prebuilt DLC for stable package.
chromeos-ml-dlc_src_unpack() {
	if [[ "${PV}" == "9999" ]]; then
		[[ "$(type -t chromeos-ml-dlc-9999_src_unpack)" != "function" ]] && die "chromeos-ml-dlc-9999_src_unpack undefined"
		chromeos-ml-dlc-9999_src_unpack
	else
		[[ -z "${DLC_META_ARTIFACT}" ]] && die "DLC_META_ARTIFACT undefined"
		unpacker "${DISTDIR}/"*"${DLC_META_ARTIFACT}"
	fi
}

# @FUNCTION: chromeos-ml-dlc_src_install
# @DESCRIPTION:
# The src_install function for the ChromeOS ML DLC package.
# It will run the chromeos-ml-dlc-9999 install function in the ebuild file for unstable package.
# Or install the prebuilt DLC for stable package.
chromeos-ml-dlc_src_install() {
	if [[ "${PV}" == "9999" ]]; then
		[[ "$(type -t chromeos-ml-dlc-9999_src_install)" != "function" ]] && die "chromeos-ml-dlc-9999_src_install undefined"
		chromeos-ml-dlc-9999_src_install
	else
		insinto "/${DLC_META_ARTIFACT_BUILD_DIR}/${DLC_ID}/${DLC_PACKAGE}"
		newins "${DISTDIR}/"*-dlc.img dlc.img

		# shellcheck disable=SC2206   # Expand on whitespace.
		local iuse_array=(${IUSE})
		local append=""
		for flag in "${iuse_array[@]#[-+]}"; do
			if [[ "${flag}" == "cros_host" || "${flag}" == "cros_workon_tree"* ]]; then
				continue
			fi
			append="${append}$(usex "${flag}" "P${flag}" "N${flag}")"
		done

		DLC_SRC_URI_PREFIX="${CHROMEOS_ML_BUCKET}/${CHROMEOS_ML_VERSION}-${append}" \
		DLC_MIRROR_BUCKET="gs://chromeos-localmirror-private" \
			dlc-prebuilt_src_install
	fi
}

fi

EXPORT_FUNCTIONS src_unpack src_install
