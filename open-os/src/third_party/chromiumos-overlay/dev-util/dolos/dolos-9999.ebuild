# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2.

EAPI="7"

CROS_WORKON_PROJECT="chromiumos/platform/dolos"
CROS_WORKON_LOCALNAME="../platform/dolos"

PYTHON_COMPAT=( python3_{8..11} )

inherit cros-workon distutils-r1 cros-toolchain-funcs cros-sanitizers cros-protobuf

DESCRIPTION="Install dolos test tools, a virtual battery replacement."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/dolos/"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="~*"


DIRS="tools/doloscmd tools/dolosbattery"

src_prepare() {
	for dir in ${DIRS}; do
		(cd "${dir}" || die ; distutils-r1_src_prepare)
	done
}

src_configure() {
	for dir in ${DIRS}; do
		(cd "${dir}" || die ; sanitizers-setup-env)
		(cd "${dir}" || die ; distutils-r1_src_configure)
	done
}

src_compile() {
	for dir in ${DIRS}; do
		(cd "${dir}" || die ; distutils-r1_src_compile)
	done
}

src_install() {
	for dir in ${DIRS}; do
		(cd "${dir}" || die ; distutils-r1_src_install)
	done
}
