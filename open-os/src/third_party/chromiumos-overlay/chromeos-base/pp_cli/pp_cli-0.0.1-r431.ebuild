# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="441ec608672b400c9b8c2f1369f8f3e1410e2d7c"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "ea3729d55039425988d9fd82c1b07061f7b9a041" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk patchpanel .gn"

PLATFORM_SUBDIR="patchpanel/pp_cli"

inherit cros-workon libchrome platform

DESCRIPTION="CLI to control patchpanel internal state through DBus"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/patchpanel/pp_cli/"
LICENSE="BSD-Google"
KEYWORDS="*"
# This package has no unittests.
RESTRICT="test"

COMMON_DEPEND="
	chromeos-base/patchpanel-client:=
"

RDEPEND="
	${COMMON_DEPEND}
"

DEPEND="
	${COMMON_DEPEND}
"
