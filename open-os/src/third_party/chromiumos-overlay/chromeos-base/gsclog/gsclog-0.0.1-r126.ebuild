# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "cff6cf2750490c8e88289a79fe728d1cdc21a35b" "f182925db23c77d96440e4dade69bf4225393f47" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk gsclog trunks .gn"

PLATFORM_SUBDIR="gsclog"

inherit cros-workon platform user

DESCRIPTION="GSC log concatenator for Chromium OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/gsclog/"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="ti50_onboard"
# Only Ti50 supports the get logs command, so fail builds for other chips.
REQUIRED_USE="ti50_onboard"

RDEPEND="
	chromeos-base/minijail:=
	chromeos-base/trunks:=
"

DEPEND="${RDEPEND}"

BDEPEND="
	chromeos-base/minijail
"

pkg_setup() {
	enewuser "gsclog"
	enewgroup "gsclog"

	cros-workon_pkg_setup
}
