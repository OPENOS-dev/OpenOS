# Copyright 2016 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="1b7a3e54d8c660cddc38d7869af656d801d22b19"
CROS_WORKON_TREE="16839d1d0fcecb39e52d1bc3a65aa02d62c1ee6d"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_SUBTREE="chromeos-config"

PYTHON_COMPAT=( python3_{6..11} )

inherit cros-workon distutils-r1

DESCRIPTION="Chrome OS configuration host tools"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/chromeos-config"

LICENSE="BSD-Google"
SLOT=0
KEYWORDS="*"

RDEPEND="
	>=sys-fs/squashfs-tools-4.3
	dev-python/jinja2[${PYTHON_USEDEP}]
	dev-python/jsonschema[${PYTHON_USEDEP}]
	dev-python/pyyaml[${PYTHON_USEDEP}]
	dev-python/setuptools[${PYTHON_USEDEP}]
	!<chromeos-base/chromeos-config-tools-0.0.4
"

DEPEND="
	${RDEPEND}
	dev-python/setuptools[${PYTHON_USEDEP}]
"

# Needed for cros_config_proto_converter.
RDEPEND+="
  chromeos-base/cros-config-api
"

REQUIRED_USE="${PYTHON_REQUIRED_USE}"

src_unpack() {
	cros-workon_src_unpack
	S+="/chromeos-config"
}
