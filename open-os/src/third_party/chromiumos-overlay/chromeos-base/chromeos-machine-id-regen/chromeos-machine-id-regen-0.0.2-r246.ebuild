# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "91bfb632df4a19b2f72993c8564f0b7f944121ac" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk machine-id-regen metrics .gn"

PLATFORM_SUBDIR="machine-id-regen"

inherit cros-workon platform systemd

DESCRIPTION="Utility to periodically update machine-id"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/machine-id-regen/"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE="systemd"

DEPEND="
	chromeos-base/metrics:=
	sys-apps/dbus:=
	sys-apps/upstart:=
"

RDEPEND="${DEPEND}"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
"

src_install() {
	platform_src_install

	# Install init scripts for systemd the ones for upstart are installd via
	# BUILD.gn.
	if use systemd; then
		systemd_dounit init/machine-id-regen-network.service
		systemd_dounit init/machine-id-regen-periodic.service
		systemd_enable_service shill-disconnected.target machine-id-regen-network.service
		systemd_dounit init/machine-id-regen-periodic.timer
		systemd_enable_service system-services.target machine-id-regen-periodic.timer
	fi
}
