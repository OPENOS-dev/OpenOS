# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# A package for privacy in chromiumos/platform2

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "76b687903dc9e082c2e7ac4e8787946b248b9c63" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
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
KEYWORDS="*"

RDEPEND="
"

DEPEND="
${RDEPEND}
"

src_install() {
	platform_src_install
}
