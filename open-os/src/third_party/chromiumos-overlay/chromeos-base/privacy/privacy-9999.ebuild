# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# A package for privacy in chromiumos/platform2

EAPI=7

CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_DESTDIR="${S}/platform2"
CROS_WORKON_SUBTREE="common-mk privacy .gn"

PLATFORM_SUBDIR="privacy"

inherit cros-workon platform

DESCRIPTION="Privacy related packages for chromiumos"
HOMEPAGE="https://source.chromium.org/chromiumos/chromiumos/codesearch/+/main:src/platform2/privacy/"
LICENSE="BSD-Google"
KEYWORDS="~*"

RDEPEND="
"

DEPEND="
${RDEPEND}
"

src_install() {
	platform_src_install
}
