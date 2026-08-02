# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

# please keep this ebuild at EAPI 7 -- sys-apps/portage dep
EAPI=7

DISTUTILS_USE_PEP517=flit
PYTHON_COMPAT=( python3_{8..12} )

inherit distutils-r1

DESCRIPTION="Backport of pathlib-compatible object wrapper for zip files"
HOMEPAGE="https://github.com/jaraco/zipp/"
SRC_URI="mirror://pypi/${PN:0:1}/${PN}/${P}.tar.gz"

LICENSE="MIT"
SLOT="0"
KEYWORDS="*"

BDEPEND="
	test? (
		dev-python/jaraco-functools[${PYTHON_USEDEP}]
		dev-python/jaraco-itertools[${PYTHON_USEDEP}]
		dev-python/more-itertools[${PYTHON_USEDEP}]
	)
"

distutils_enable_tests pytest

EPYTEST_DESELECT=(
	# speed tests are flaky by design
	tests/test_zipp.py::TestPath::test_implied_dirs_performance
)

src_prepare() {
	# dep of disabled test
	sed -i -e '/func_timeout/d' tests/test_zipp.py || die
	distutils-r1_src_prepare
}

src_configure() {
	grep -q 'build-backend = "setuptools' pyproject.toml ||
		die "Upstream changed build-backend, recheck"
	# write a custom pyproject.toml to ease setuptools bootstrap
	cat > pyproject.toml <<-EOF || die
		[build-system]
		requires = ["flit_core >=3.2,<4"]
		build-backend = "flit_core.buildapi"

		[project]
		name = "zipp"
		version = "${PV}"
		description = "Backport of pathlib-compatible object wrapper for zip files"
	EOF
}
