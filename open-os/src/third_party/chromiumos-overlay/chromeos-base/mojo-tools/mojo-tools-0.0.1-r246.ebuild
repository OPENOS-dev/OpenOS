# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="79edaa8ab831a2a2a1f48604d8805126ecfd1a1b"
CROS_WORKON_TREE="f6ab8b3535eef7601daa8666e0e6485dfd6246b6"
CROS_WORKON_PROJECT=("chromiumos/platform/libchrome")
CROS_WORKON_LOCALNAME=("platform/libchrome")
CROS_WORKON_EGIT_BRANCH=("main")
CROS_WORKON_SUBTREE=("mojo/public/tools")
PYTHON_COMPAT=( python3_{6..11} )

inherit cros-workon python-r1

DESCRIPTION="Mojo python tools for binding generating."
HOMEPAGE="https://chromium.googlesource.com/chromium/src/+/HEAD/mojo/public/tools/mojom/"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

RDEPEND="${PYTHON_DEPS}
	dev-python/jinja2[${PYTHON_USEDEP}]
"
DEPEND="${PYTHON_DEPS}"

src_install() {
	install_python() {
		python_domodule mojo/public/tools/mojom/mojom
		python_domodule mojo/public/tools/bindings/generators
	}
	python_foreach_impl install_python
}
