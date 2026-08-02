# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# An ext2 based DLC, can be used for verification/testing/etc.

EAPI="7"

inherit dlc

DESCRIPTION="An ext2 based DLC"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/dlcservice"
SRC_URI=""

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE="dlc"
REQUIRED_USE="dlc"

# Required
DLC_PREALLOC_BLOCKS="1024"

# Optional, reference design doc for all other optional DLC variables.
DLC_NAME="EXT2 DLC"

# Enabled scaled design.
DLC_SCALED=true

# Set the filesystem type to 'ext2'.
DLC_FS_TYPE="ext2"

src_unpack() {
	# Because we are not pulling in any sources, we need to have an empty
	# source directory to satisfy the build success.
	S="${WORKDIR}"
}

src_install() {
	# Create a few files with random content. The contents of these files
	# are not important. Update the `seed` to change randomness.
	local seed="${PF}"

	# Setup DLC paths.
	insinto "$(dlc_add_path /opt)"

	echo "${seed}" | newins - seed

	dlc_src_install
}
