# Copyright 2018 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2.

EAPI="7"

PYTHON_COMPAT=( python3_{8..12} )
DISTUTILS_USE_PEP517=setuptools

inherit distutils-r1

DESCRIPTION="Google Cloud API client core library"
SRC_URI="mirror://pypi/${PN:0:1}/${PN}/${P}.tar.gz"

LICENSE="Apache-2.0"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	>=dev-python/google-api-core-1.19.0[${PYTHON_USEDEP}]
	>=dev-python/grpcio-1.8.2[${PYTHON_USEDEP}]"
DEPEND="${RDEPEND}"

python_compile() {
	distutils-r1_python_compile
	find "${BUILD_DIR}" -name '*.pth' -delete || die
}
