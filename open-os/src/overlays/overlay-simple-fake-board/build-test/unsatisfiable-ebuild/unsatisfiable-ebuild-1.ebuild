# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# sudo emerge build-test/unsatisfiable-ebuild
EAPI=7

DESCRIPTION="No ebuild exists that satisfies the dependency."
HOMEPAGE="http://www.chromium.org"

LICENSE="GPL-2"
SLOT=0
KEYWORDS="*"
IUSE=""

RDEPEND="<build-test/base-pkg-1"
DEPEND="${RDEPEND}"
