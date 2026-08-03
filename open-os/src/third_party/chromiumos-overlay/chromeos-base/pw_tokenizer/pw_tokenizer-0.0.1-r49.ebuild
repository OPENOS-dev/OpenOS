# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2.

EAPI=7

CROS_WORKON_COMMIT="c25046dc5cedfdf67e06e62b566a4ed4a6b36b2e"
CROS_WORKON_TREE="97bf7c4d831669c1d2647fc1cfb1a32892ee28f9"
CROS_WORKON_PROJECT="chromiumos/third_party/pigweed/pigweed"
CROS_WORKON_LOCALNAME="third_party/pigweed"
CROS_WORKON_SUBTREE="pw_tokenizer/py"
CROS_WORKON_EGIT_BRANCH="upstream/main"

PYTHON_COMPAT=( python3_{8..11} )

inherit cros-workon distutils-r1

DESCRIPTION="Pigweed tokenizer"
HOMEPAGE="https://pigweed.dev/"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE=""

BDEPEND="dev-python/setuptools[${PYTHON_USEDEP}]"

REQUIRED_USE="${PYTHON_REQUIRED_USE}"

src_unpack() {
	cros-workon_src_unpack
	S+="/pw_tokenizer/py"

	cat > "${S}/setup.py" <<- EOF || die
		from setuptools import setup
		setup()
	EOF
}
