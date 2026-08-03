# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# This is a sample DLC (LVM) used for testing new features in DLC.

EAPI="7"

inherit dlc

DESCRIPTION="A sample DLC (LVM)"
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
DLC_NAME="Sample DLC (LVM)"

# Only use this variable if you have integration tests running against the DLC.
DLC_PRELOAD=true

# Force usage of logical volumes for the DLC.
DLC_USE_LOGICAL_VOLUME=true

src_unpack() {
	# Because we are not pulling in any sources, we need to have an empty
	# source directory to satisfy the build success.
	S="${WORKDIR}"
}

src_install() {
	local seed="${PF}"
	insinto "$(dlc_add_path /opt)"
	echo "${seed}" | newins - seed
	dlc_src_install
}
