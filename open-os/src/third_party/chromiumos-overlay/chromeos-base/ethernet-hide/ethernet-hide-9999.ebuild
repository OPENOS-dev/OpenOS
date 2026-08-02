# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_SUBTREE="ethernet-hide"

PYTHON_COMPAT=( python3_{7..11} )

inherit cros-workon python-single-r1

DESCRIPTION="A tool that hides Ethernet interfaces while still enabling the SSH connection."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/ethernet-hide/"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="~*"
IUSE=""

RDEPEND="${PYTHON_DEPS}
	$(python_gen_cond_dep 'dev-python/dbus-python[${PYTHON_USEDEP}]')
	chromeos-base/shill
	net-misc/dhcp
	net-misc/socat
	net-misc/openssh
	sys-apps/iproute2"

src_compile() {
	# We only install scripts here, so no need to compile.
	:
}

src_install() {
	python_domodule ethernet-hide/ehide
	python_newscript ethernet-hide/main.py ehide
}
