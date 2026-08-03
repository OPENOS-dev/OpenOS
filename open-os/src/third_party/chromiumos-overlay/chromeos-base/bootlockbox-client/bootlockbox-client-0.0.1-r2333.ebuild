# Copyright 2018 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "22caa0fa0f150cde0869e0bafe74dd0da66cb9b4" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk bootlockbox .gn"

PLATFORM_SUBDIR="bootlockbox/client"

inherit cros-workon platform cros-protobuf

DESCRIPTION="BootLockbox DBus client library for Chromium OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/bootlockbox/client/"

LICENSE="BSD-Google"
KEYWORDS="*"
# Disable unittesting for client bindings.
RESTRICT="test"

RDEPEND="
	chromeos-base/system_api:=
"

# Workaround to rebuild this package on the chromeos-dbus-bindings update.
# Please find the comment in chromeos-dbus-bindings for its background.
# Workaround to rebuild this package when protoc is upgraded.
DEPEND="
	${RDEPEND}
	chromeos-base/chromeos-dbus-bindings:=
"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
"

src_install() {
	platform_src_install

	# Export neccessary header files:
	insinto /usr/include/bootlockbox-client/bootlockbox
	doins ../boot_lockbox_client.h

	# Export necessary for crytphome header files:
	insinto /usr/include/bootlockbox
	doins "${OUT}"/gen/include/bootlockbox/*.h

	dolib.a "${OUT}"/libbootlockbox-proto.a
	# Install libbootlockbox-client.so:
	dolib.so "${OUT}"/lib/libbootlockbox-client.so

	# Install DBus client library.
	platform_install_dbus_client_lib "bootlockbox"
}
