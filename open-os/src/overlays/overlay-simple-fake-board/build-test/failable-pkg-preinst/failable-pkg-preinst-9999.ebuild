# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"

inherit cros-workon

DESCRIPTION="cros-workon package that optionally fails pkg_preinst."
HOMEPAGE="http://www.chromium.org"

LICENSE="GPL-2"
KEYWORDS="~*"
IUSE=""

RDEPEND=""
DEPEND=""

pkg_preinst() {
	if [[ -e /tmp/FAIL_PKG_PREINST ]]; then
		die "/tmp/FAIL_PKG_PREINST exists, failed pkg_preinst."
	fi
}
