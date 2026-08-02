# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_INCREMENTAL_BUILD="1"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk featured metrics regmon .gn"

PLATFORM_SUBDIR="regmon"

inherit cros-protobuf cros-workon platform user
DESCRIPTION="Daemon to report policy violations of first-party network traffic."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/regmon/"
LICENSE="BSD-Google"
SLOT=0/0
KEYWORDS="~*"

RDEPEND="
	chromeos-base/featured:=
	chromeos-base/metrics:=
	chromeos-base/minijail:=
"

DEPEND="
	${RDEPEND}
	chromeos-base/system_api:=
"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
	chromeos-base/minijail
"

pkg_setup() {
	# Use pkgsetup because src_install needs regmond user and group
	enewuser regmond
	enewgroup regmond
	enewgroup regmond_senders
	cros-workon_pkg_setup
}

src_install() {
	platform_src_install

	local daemon_store="/etc/daemon-store/regmond"
	dodir "${daemon_store}"
	fperms 0700 "${daemon_store}"
	fowners regmond:regmond "${daemon_store}"
}
