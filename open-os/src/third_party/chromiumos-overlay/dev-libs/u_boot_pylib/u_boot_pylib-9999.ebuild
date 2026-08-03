# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_PROJECT="chromiumos/third_party/u-boot"
CROS_WORKON_LOCALNAME="u-boot/files"
CROS_WORKON_SUBTREE="tools/u_boot_pylib"
CROS_WORKON_EGIT_BRANCH="chromeos-v2023.10-next"

DISTUTILS_USE_PEP517="setuptools"
PYTHON_COMPAT=( python3_{8..12} )

inherit cros-workon distutils-r1

DESCRIPTION="U-Boot python library"
HOMEPAGE="https://www.denx.de/wiki/U-Boot"

LICENSE="GPL-2"
SLOT="0/0"
KEYWORDS="~*"
IUSE=""

src_unpack() {
	cros-workon_src_unpack

	S+=/tools/u_boot_pylib
}

python_prepare_all() {
	# setup.py is redundant to pyproject.toml.
	rm "setup.py" || die
	distutils-r1_python_prepare_all
}
