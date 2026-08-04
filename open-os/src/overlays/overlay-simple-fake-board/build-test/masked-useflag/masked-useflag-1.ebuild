# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# sudo emerge build-test/masked-useflag
EAPI=7

DESCRIPTION="Masked Package"
HOMEPAGE="http://www.chromium.org"

LICENSE="BSD-Google"
SLOT="${PN}/0"
KEYWORDS="*"
IUSE="masked_useflag"

RDEPEND="=build-test/base-pkg-1[masked_useflag]"
DEPEND=""
