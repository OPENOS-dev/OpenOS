# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "b8033e453c7d9518619e90fb100d7d90d7b4026d" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "5b4eea73c5eee4fc37a928185776c72de793f2d0" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
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
KEYWORDS="*"

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
