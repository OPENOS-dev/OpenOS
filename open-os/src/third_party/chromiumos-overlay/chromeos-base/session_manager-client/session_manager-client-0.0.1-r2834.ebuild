# Copyright 2015 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "81b98a37471549e915e85559acff067cc6a9590d" "431e8602137055191734ac82485233855e1950a6" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk login_manager/dbus_bindings login_manager/session_manager-client .gn"

PLATFORM_NATIVE_TEST="yes"
PLATFORM_SUBDIR="login_manager/session_manager-client"
WANT_LIBCHROME="no"

inherit cros-workon platform

DESCRIPTION="Session manager (chromeos-login) DBus client library for Chromium OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/login_manager/"

LICENSE="BSD-Google"
KEYWORDS="*"
# Disable unittesting for client bindings.
RESTRICT="test"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
"

# Workaround to rebuild this package on the chromeos-dbus-bindings update.
# Please find the comment in chromeos-dbus-bindings for its background.
DEPEND="
	chromeos-base/chromeos-dbus-bindings:=
"

RDEPEND="
	!<chromeos-base/chromeos-login-0.0.2
"

src_install() {
	platform_src_install

	# Install DBus client library.
	platform_install_dbus_client_lib "session_manager"
}
