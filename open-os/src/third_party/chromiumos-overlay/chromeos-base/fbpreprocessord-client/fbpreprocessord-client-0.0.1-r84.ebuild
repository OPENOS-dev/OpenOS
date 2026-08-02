# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "f1d257da620b69389bd2c06d7ce149ceec804d12" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk fbpreprocessor .gn"

PLATFORM_SUBDIR="fbpreprocessor/client"

inherit cros-workon platform

DESCRIPTION="Chrome OS fbpreprocessord client library"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/fbpreprocessor/"
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
	platform_install_dbus_client_lib "fbpreprocessor"
}
