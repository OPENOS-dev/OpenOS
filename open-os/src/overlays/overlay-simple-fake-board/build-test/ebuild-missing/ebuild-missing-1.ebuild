# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# sudo emerge build-test/ebuild-missing
EAPI=7

DESCRIPTION="Depends on a package that does not exist."
HOMEPAGE="http://www.chromium.org"

LICENSE="BSD-Google"
SLOT=0
KEYWORDS="*"
IUSE=""

RDEPEND="build-test/does-not-exist"
DEPEND="${RDEPEND}"
