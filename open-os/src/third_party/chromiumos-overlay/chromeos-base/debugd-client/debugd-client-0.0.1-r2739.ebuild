# Copyright 2015 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="da569a22673b3062a21647821a6e5b5b461c0da5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "4b8d3a8a1831dbbc1e56166555ae744e1d5e2844" "f42007e4a31278e678d51c10aeca399c103092d3" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk debugd/client debugd/dbus_bindings .gn"

PLATFORM_SUBDIR="debugd/client"
WANT_LIBCHROME="no"

inherit cros-workon platform

DESCRIPTION="Chrome OS debugd client library"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/debugd/"
SRC_URI=""

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

src_install() {
	platform_src_install
	# Install DBus client library.
	platform_install_dbus_client_lib "debugd"
}
