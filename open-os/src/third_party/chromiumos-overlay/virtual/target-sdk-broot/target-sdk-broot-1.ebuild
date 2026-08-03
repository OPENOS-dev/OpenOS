# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

DESCRIPTION="
	List of build-time packages that are needed for building boards.
	These are built on a per-board basis instead of part of the SDK.
	Generally first party projects that are updated frequently should be here.
	'broot' is short for 'build root'; see the portage BROOT variable.
"
HOMEPAGE="https://dev.chromium.org/"

LICENSE="metapackage"
SLOT="0"
KEYWORDS="*"
IUSE=""

RDEPEND="virtual/target-chromium-os-sdk-broot"
