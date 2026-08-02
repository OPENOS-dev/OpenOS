# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

inherit cros-camera dlc-prebuilt

DESCRIPTION="Package for super resolution library as a DLC"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE="dlc"
REQUIRED_USE="dlc"

S="${WORKDIR}"

# The size of the compressed super resolution library is about 5.5 MB. Therefore,
# considering the future growth, we should reserve 5.5 * 130% ~= 8 MB.
DLC_PREALLOC_BLOCKS="$((8 * 256))"

# Preload the DLC for test images.
DLC_PRELOAD=true


# The prebuilt file is generated via
# cros_generate_dlc_artifacts \
#  --upload \
#  --src-dir ${ABI}-${MARCH} \
#  --license ~/chromiumos/src/third_party/chromiumos-overlay/licenses/BSD-Google \
#  --id=cros-camera-super-res-dlc \
#  --preallocated-blocks=$((8 * 256)) \
#  --version=${PV}_${ABI}-${MARCH} \
#  --preload
uri_prefix_for() {
	local abi="$1"
	local march="$2"
	echo "gs://chromeos-localmirror/dlc-images/${PN}/package/${PV}_${abi}-${march}"
}

uri_for() {
	local abi="$1"
	local march="$2"
	local prefix=$(uri_prefix_for "${abi}" "${march}")
	echo "${prefix}/${DLC_META_ARTIFACT} -> ${PV}-${abi}-${march}-${DLC_META_ARTIFACT}"
	echo "${prefix}/dlc.img -> ${PV}-${abi}-${march}-dlc.img"
}

# ABI march flag -> URI[prefix]? mappings
# amd64
# Shellcheck can't understand namedrefs as function arguments.
# shellcheck disable=SC2034
declare -A march_uris_amd64=(
	["march_x86-64"]="$(uri_for amd64 march_x86-64)"
)
# arm64
# Shellcheck can't understand namedrefs as function arguments.
# shellcheck disable=SC2034
declare -A march_uris_arm64=(
	["march_armv8-a"]="$(uri_for arm64 march_armv8-a)"
)
# arm
# TODO(go/cros-arm64-plan): Remove once all boards have migrated to 64-bit user space.
# Shellcheck can't understand namedrefs as function arguments.
# shellcheck disable=SC2034
declare -A march_uris_arm=(
	["march_armv7-a"]="$(uri_for arm march_armv7-a)"
	["march_armv8-a"]="$(uri_for arm march_armv8-a)"
)

SRC_URI="
	$(cros-binary_generate_src_uris march_uris_amd64 march_uris_arm64 march_uris_arm)
	gs://chromeos-localmirror/distfiles/${P}.tar.gz
"

# For accessing dlc-images
RESTRICT="mirror"

src_prepare() {
	default
	if use amd64; then
		DLC_SRC_URI_PREFIX="$(uri_prefix_for amd64 march_x86-64)"
	elif use arm64; then
		DLC_SRC_URI_PREFIX="$(uri_prefix_for arm64 march_armv8-a)"
	elif use arm; then
		DLC_SRC_URI_PREFIX="$(uri_prefix_for arm march_armv8-a)"
		if use march_armv7-a; then
			DLC_SRC_URI_PREFIX="$(uri_prefix_for arm march_armv7-a)"
		fi
	fi
}

src_unpack() {
	default
	# Don't bother using the dlc-prebuilt unpack function here.
	unpacker "${DISTDIR}/"*"${DLC_META_ARTIFACT}"
}

src_install() {
	insinto /usr/include/cros-camera/libupsample/
	doins "${WORKDIR}"/upsample_wrapper_bindings.h
	doins "${WORKDIR}"/upsample_wrapper_types.h

	insinto "/${DLC_META_ARTIFACT_BUILD_DIR}/${DLC_ID}/${DLC_PACKAGE}"
	newins "${DISTDIR}/"*-dlc.img dlc.img

	dlc-prebuilt_src_install
}
