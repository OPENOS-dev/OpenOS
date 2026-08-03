# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

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
KEYWORDS="~*"
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
