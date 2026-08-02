# Copyright 2023 The Chromium OS Authors. All rights reserved.
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "a614296718b76bc1343020a08c1077f9267ee695" "43eb4f30218ee6fc055f185786d914bccd668086" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_SUBTREE="common-mk heartd mojo_service_manager .gn"

PLATFORM_SUBDIR="heartd/tools"

inherit cros-workon platform user

DESCRIPTION="ChromeOS heartd tool."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/heartd"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""
# This package has no unittests.
RESTRICT="test"

COMMON_DEPEND="
	chromeos-base/mojo_service_manager:=
"

DEPEND="
	${COMMON_DEPEND}
	chromeos-base/system_api:=
"

RDEPEND="
	${COMMON_DEPEND}
"
