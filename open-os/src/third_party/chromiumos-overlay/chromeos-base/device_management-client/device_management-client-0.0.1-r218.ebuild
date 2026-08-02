# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="c7b71803ab8bd779a8d24d41660997e328fa4cd9"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "41a12df837a393cabb15aa3e428efaf95b170a6f" "b8c09b0737d26e92e8c1543f785a92a112de09cc" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk device_management libhwsec-foundation .gn"

PLATFORM_SUBDIR="device_management/client"

inherit cros-workon platform

DESCRIPTION="Device Management D-Bus client library for ChromiumOS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/device_management/client"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""
# Disable unittesting for client bindings.
RESTRICT="test"

DEPEND="
	chromeos-base/system_api:=
"

RDEPEND="${DEPEND}"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
"

src_install() {
	platform_src_install

	# Install D-Bus client library.
	platform_install_dbus_client_lib "device_management"
}
