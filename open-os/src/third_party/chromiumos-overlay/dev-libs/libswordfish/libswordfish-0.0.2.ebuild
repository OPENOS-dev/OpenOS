# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

inherit dlc cros-binary

DESCRIPTION="Google swordfish library for ChromeOS"

# ABI march flag -> URI mappings
# amd64
# Shellcheck can't understand namedrefs as function arguments.
# shellcheck disable=SC2034
declare -A march_uris_amd64=(
	["march_alderlake"]="gs://chromeos-localmirror/distfiles/libswordfish_chromeos_alderlake-${PV}.tar.gz"
)

SRC_URI="
	$(cros-binary_generate_src_uris march_uris_amd64)
"

RESTRICT="mirror"

LICENSE="BSD-Google Apache-2.0 MPL-2.0 icu-58"
SLOT="0"
KEYWORDS="*"

IUSE="
	dlc
"

# All possible march USE flags.
# Declared in cros-binary.eclass.
# shellcheck disable=SC2154
IUSE+="${CROS_BINARY_MARCHS_USE}"

# Swordfish will not be supported on rootfs and only be supported through DLC.
REQUIRED_USE="
	dlc
"

# Exactly one march flag is required.
# Declared in cros-binary.eclass.
# shellcheck disable=SC2154
REQUIRED_USE+="${CROS_BINARY_MARCHS_REQUIRED_USE}"

S="${WORKDIR}"

# The storage space for this dlc. This sets up the upper limit of this dlc to be
# DLC_PREALLOC_BLOCKS * 4KB = 25MB for now.
DLC_PREALLOC_BLOCKS="6400"
# Preload DLC data on test images.
DLC_PRELOAD=true

# Enabled scaled design.
DLC_SCALED=true

src_install() {
	local swordfishlib_path="$(dlc_add_path /)"

	insinto "${swordfishlib_path}"
	# Install the shared library.
	insopts -m0755
	newins "libswordfish.so" "libswordfish.so"
	insopts -m0644

	dlc_src_install
}
