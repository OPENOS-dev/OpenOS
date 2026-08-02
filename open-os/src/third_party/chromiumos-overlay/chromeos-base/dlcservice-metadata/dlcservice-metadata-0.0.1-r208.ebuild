# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "5f14740aa045a65cb4e1b813ef0657f5e03ecb3c" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk dlcservice .gn"

PLATFORM_SUBDIR="dlcservice/metadata"

inherit cros-workon platform

DESCRIPTION="DLC metadata library for ChromiumOS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/dlcservice/metadata"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"

# File libdlcservice-metadata.so moved from dlcservice.
RDEPEND="
	!<=chromeos-base/dlcservice-0.0.1-r1073
"

DEPEND="
	${RDEPEND}
	chromeos-base/system_api:=
"
