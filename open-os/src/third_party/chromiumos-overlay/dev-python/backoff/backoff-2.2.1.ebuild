# Copyright 2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_SETUPTOOLS=rdepend
PYTHON_COMPAT=( python3_{8..12} )
inherit distutils-r1

DESCRIPTION="Function decoration for backoff and retry"
HOMEPAGE="https://github.com/litl/backoff"
SRC_URI="mirror://pypi/${PN:0:1}/${PN}/${P}.tar.gz"

LICENSE="Apache-2.0"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	dev-python/setuptools[${PYTHON_USEDEP}]
"
