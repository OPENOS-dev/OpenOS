# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

# This ebuild only cares about its own FILESDIR and ebuild file, so it tracks
# the canonical empty project.
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-workon cros-fwupd

DESCRIPTION="Installs UEFI dbx update files used by fwupd."
HOMEPAGE="https://fwupd.org/lvfs/devices/"

FILENAMES=(
	# com.microsoft.dbx.x64.firmware version 20241101
	"d661d4a0aaca09dfa9e56967ca2467b0575fc07cb704d182fa8c68225452957f-DBXUpdate-20241101-x64.cab"

	# org.linuxfoundation.dbx.ia32.firmware version 89
	"f2f984b66f262801f4b3d25d719b64c99c0869bc653c33c6691fb5c604b955c5-DBXUpdate-20230509-ia32.cab"
)
SRC_URI="${FILENAMES[*]/#/${CROS_FWUPD_URL}/}"
LICENSE="LVFS-Vendor-Agreement-v1"

KEYWORDS="~*"

DEPEND=""
RDEPEND="sys-apps/fwupd"
