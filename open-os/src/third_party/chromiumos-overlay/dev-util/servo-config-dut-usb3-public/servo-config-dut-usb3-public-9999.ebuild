# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the BSD license.

EAPI="7"

# This ebuild only cares about its own FILESDIR and ebuild file, so it tracks
# the canonical empty project.
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-workon

DESCRIPTION="List DUT USB 3 capability of Servo devices."
LICENSE="BSD-Google"
KEYWORDS="~*"

RDEPEND="!dev-util/servo-config-dut-usb3-private"

src_install() {
	insinto /usr/share/servo
	doins "${FILESDIR}"/data/*

	insinto /etc/servo
	doins "${FILESDIR}"/sysconf/dut_usb3.no
	doins "${FILESDIR}"/sysconf/dut_usb3.yes
}
