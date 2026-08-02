# Copyright 2015 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_OUTOFTREE_BUILD=1

CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_SUBTREE="common-mk update_engine .gn"

PLATFORM_NATIVE_TEST="yes"
PLATFORM_SUBDIR="update_engine/client-headers"

inherit cros-debug cros-workon platform

DESCRIPTION="ChromeOS Update Engine client library"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/update_engine/"
SRC_URI=""

LICENSE="BSD-Google"
KEYWORDS="~*"
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
	!<chromeos-base/update_engine-0.0.3
"

src_install() {
	platform_src_install

	# Install DBus client library.
	platform_install_dbus_client_lib "update_engine"
}
