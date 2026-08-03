# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

inherit cros-binary cros-constants dlc-prebuilt

DESCRIPTION="Downloads the assets needed for ml-core"

LICENSE="BSD-Google"
KEYWORDS="-* amd64 arm64"
# All possible march USE flags declared in cros-binary.eclass.
# shellcheck disable=SC2154
IUSE="camera_feature_effects local_ml_core_internal dlc ${CROS_BINARY_MARCHS_USE}"
SLOT=0

# TODO(b/271513675): This is pulled in by some boards which build with -dlc.
# So, temporarily don't require "dlc" IUSE flag.
# Exactly one march flag is required, which is declared in cros-binary.eclass.
# shellcheck disable=SC2154
REQUIRED_USE="${CROS_BINARY_MARCHS_REQUIRED_USE}"

# Since we are moving the files from chromeos-base/ml-core-internal,
# we can not have both packages together.
# See https://www.chromium.org/chromium-os/developer-library/guides/portage/ebuild-faq/#moving-files-between-packages
RDEPEND="!chromeos-base/ml-core-internal"

S="${WORKDIR}"

# Upper limit for this DLC. 15000 * 4KB = 60MB.
DLC_PREALLOC_BLOCKS="15000"
# Preload DLC data on test images.
DLC_PRELOAD=true

# Get ml-core-dlc SRC_URI prefix
generate_ml_core_dlc_SRC_URI_prefix() {
	local arch="$1"
	echo "gs://chromeos-localmirror/dlc-images/${PN}/package/${arch}-${PV}"
}

# Generate ml-core-dlc SRC_URI by arch
generate_ml_core_dlc_SRC_URI_for() {
	local arch="$1"
	local prefix="$(generate_ml_core_dlc_SRC_URI_prefix "${arch}")"
	echo "${prefix}/${DLC_META_ARTIFACT} -> ${PV}-${arch}-${DLC_META_ARTIFACT}"
	echo "${prefix}/dlc.img -> ${PV}-${arch}-dlc.img"
}

# Generate ml-core-dlc SRC_URI by arch
generate_ml_core_dlc_SRC_URI() {
	# ABI march flag -> URI[prefix]? mappings
	# amd64
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_amd64=(
		["march_x86-64"]="$(generate_ml_core_dlc_SRC_URI_for amd64)"
	)

	# arm64
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_arm64=(
		["march_armv8-a"]="$(generate_ml_core_dlc_SRC_URI_for arm64)"
	)

	# arm
	# Required for cros-binary_generate_src_uris. Keep this empty.
	# Shellcheck can't understand namedrefs as function arguments.
	# shellcheck disable=SC2034
	local -A march_uris_arm=()

	cros-binary_generate_src_uris march_uris_amd64 march_uris_arm64 march_uris_arm
}

# For accessing dlc-images
RESTRICT="mirror"
SRC_URI="
	amd64? (
		gs://chromeos-localmirror/distfiles/ml-core-lib-${PV}-amd64.tar.xz
	)
	arm64? (
		gs://chromeos-localmirror/distfiles/ml-core-lib-${PV}-arm64.tar.xz
	)
	$(generate_ml_core_dlc_SRC_URI)
"

src_prepare() {
	default
	if use amd64; then
		DLC_SRC_URI_PREFIX="$(generate_ml_core_dlc_SRC_URI_prefix amd64)"
	elif use arm64; then
		DLC_SRC_URI_PREFIX="$(generate_ml_core_dlc_SRC_URI_prefix arm64)"
	else
		die "Unsupported arch"
	fi
}

src_unpack() {
	default
	# Don't bother using the dlc-prebuilt unpack functions here.
	unpacker "${DISTDIR}/"*"${DLC_META_ARTIFACT}"
}

src_install() {
	# Don't install / set up the actual DLC unless camera effects are enabled
	if ! use camera_feature_effects; then
		return
	fi

	insinto "/${DLC_META_ARTIFACT_BUILD_DIR}/${DLC_ID}/${DLC_PACKAGE}"
	newins "${DISTDIR}/"*-dlc.img dlc.img

	dlc-prebuilt_src_install

	insopts -m0555

	if ! use local_ml_core_internal; then
		# Install ICA headers, proto, and test files needed by ml-core and clients.
		insinto "/usr/include/ml_core"
		doins "interface.h"
		doins "ica.proto"
		doins "raid.proto"
		doins "raid_interface.h"
		insinto "/build/share/ml_core/"
		doins "cat_and_dog.webp"
		doins "moon_big.jpg"
		doins "libcros_ml_core_internal.so"
	fi
}
