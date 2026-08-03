# Copyright 2019 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# This is a sample DLC used for testing new features in DLC. It does not really
# build anything, it just creates a DLC image with random content.

EAPI="7"

inherit dlc

DESCRIPTION="A sample DLC"
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
DLC_NAME="Sample DLC"

# Only use this variable if you have integration tests running against the DLC.
DLC_PRELOAD=true

# DO NOT USE this variable, unless it was discussed with @chromeos-core-services.
DLC_FACTORY_INSTALL=true

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
