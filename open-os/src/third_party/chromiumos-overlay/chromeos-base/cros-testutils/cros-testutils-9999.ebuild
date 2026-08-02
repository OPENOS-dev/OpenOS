# Copyright 2011 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

# This ebuild only cares about its own FILESDIR and ebuild file, so it tracks
# the canonical empty project.
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-workon

DESCRIPTION="Host test utilities for ChromiumOS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/crostestutils/"

LICENSE="BSD-Google"
KEYWORDS="~*"

RDEPEND=""

# These are all either bash / python scripts.  No actual builds DEPS.
DEPEND=""

src_install() {
	dodir /usr/bin
	local f
	for f in cros_run_bvt test_that performance/bootperf-bin/{bootperf,showbootdata}; do
		dosym "${CHROOT_SOURCE_ROOT}/src/platform/crostestutils/${f}" "/usr/bin/${f##*/}"
	done
}
