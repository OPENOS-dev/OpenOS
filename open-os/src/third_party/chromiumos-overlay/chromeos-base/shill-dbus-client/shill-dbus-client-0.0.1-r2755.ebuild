# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="abd90d55ee21d870cbd5cafa975bf9a5bab4e759"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "a7dc04e623704dd76c550cbc8ebf93b3fb54e321" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_SUBTREE="common-mk shill .gn"

PLATFORM_SUBDIR="shill/dbus/client"

inherit cros-workon platform

DESCRIPTION="Shill DBus client interface library"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/shill/dbus/client"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

DEPEND="
	chromeos-base/net-base:=
	chromeos-base/system_api:=
	chromeos-base/shill-client:=
"
RDEPEND="${DEPEND}"

src_install() {
	platform_src_install

	# Install libshill-dbus-client library.
	insinto "/usr/$(get_libdir)/pkgconfig"
	local v="$(libchrome_ver)"
	./preinstall.sh "${OUT}" "${v}"
	dolib.so "${OUT}/lib/libshill-dbus-client.so"
	doins "${OUT}/lib/libshill-dbus-client.pc"

	# Install header files from libshill-dbus-client.
	insinto /usr/include/shill/dbus/client
	doins ./*.h
}

platform_pkg_test() {
	platform_test "run" "${OUT}/libshill-dbus-client_test"
}
