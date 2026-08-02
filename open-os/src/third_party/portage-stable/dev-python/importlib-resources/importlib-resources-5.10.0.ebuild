# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

# please keep this ebuild at EAPI 7 -- sys-apps/portage dep
EAPI=7

DISTUTILS_USE_PEP517=flit
# This is a backport of Python 3.9's importlib.resources
PYTHON_COMPAT=( python3_{8..12} )

inherit distutils-r1

DESCRIPTION="Read resources from Python packages"
HOMEPAGE="
	https://github.com/python/importlib_resources/
	https://pypi.org/project/importlib-resources/
"
SRC_URI="
	https://github.com/python/importlib_resources/archive/v${PV}.tar.gz
		-> importlib_resources-${PV}.gh.tar.gz
"

LICENSE="Apache-2.0"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	!dev-python/importlib_resources
	$(python_gen_cond_dep '
		>=dev-python/zipp-3.7.0-r1[${PYTHON_USEDEP}]
	' 3.8 3.9)
"

distutils_enable_tests unittest

src_unpack() {
	default_src_unpack
	mv importlib_resources-${PV} importlib-resources-${PV}
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
		name = "importlib_resources"
		version = "${PV}"
		description = "Read resources from Python packages"
	EOF
}
