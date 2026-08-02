# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "f1d257da620b69389bd2c06d7ce149ceec804d12" "b8033e453c7d9518619e90fb100d7d90d7b4026d" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk fbpreprocessor featured metrics .gn"

PLATFORM_SUBDIR="fbpreprocessor"

inherit cros-workon platform cros-protobuf user

DESCRIPTION="Pseudonymize low level debug firmware dumps for feedback reports"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/fbpreprocessord/README.md"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

COMMON_DEPEND="
	chromeos-base/debugd-client:=
	chromeos-base/featured:=
	chromeos-base/metrics:=
	chromeos-base/session_manager-client:=
	chromeos-base/system_api:=
"

ACCOUNT_DEPEND="
	acct-group/fbpreprocessor-user-access
	acct-group/fbpreprocessor
	acct-user/fbpreprocessor
"

RDEPEND="
	${ACCOUNT_DEPEND}
	${COMMON_DEPEND}
"

BDEPEND="
	${ACCOUNT_DEPEND}
	chromeos-base/chromeos-dbus-bindings
	chromeos-base/minijail
"

# Workaround to rebuild this package on the chromeos-dbus-bindings update.
# Please find the comment in chromeos-dbus-bindings for its background.
DEPEND="${COMMON_DEPEND}
	chromeos-base/chromeos-dbus-bindings:=
"

src_install() {
	platform_src_install

	# Set up cryptohome daemon mount store in daemon's mount
	# namespace.
	local daemon_store="/etc/daemon-store/fbpreprocessord"
	dodir "${daemon_store}"
	fperms 3770 "${daemon_store}"
	fowners fbpreprocessor:fbpreprocessor-user-access "${daemon_store}"
}

pkg_setup() {
	enewuser fbpreprocessor
	enewgroup fbpreprocessor
	enewgroup fbpreprocessor-user-access
}
