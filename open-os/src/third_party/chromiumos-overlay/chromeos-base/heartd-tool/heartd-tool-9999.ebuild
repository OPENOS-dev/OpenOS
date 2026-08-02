# Copyright 2023 The Chromium OS Authors. All rights reserved.
# Distributed under the terms of the GNU General Public License v2

EAPI=7

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
KEYWORDS="~*"
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
