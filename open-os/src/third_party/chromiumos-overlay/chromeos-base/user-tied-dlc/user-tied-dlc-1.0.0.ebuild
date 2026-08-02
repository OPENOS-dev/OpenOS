# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# This is a sample DLC used for testing new features in DLC. It does not really
# build anything. This DLC is tied to the user and will be installed into the
# user's cryptohome.

EAPI="7"

inherit dlc

DESCRIPTION="A sample user-tied DLC"
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
DLC_NAME="Sample user-tied DLC"

# Only use this variable if you have integration tests running against the DLC.
DLC_PRELOAD=true

# The DLC is tied to the user.
DLC_USER_TIED=true

# The user-tied DLC is scaled and will not update with the OS.
DLC_SCALED=true

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
