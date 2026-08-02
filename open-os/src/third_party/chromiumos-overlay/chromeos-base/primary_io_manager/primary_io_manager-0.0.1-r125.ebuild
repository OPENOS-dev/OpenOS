# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="1b7a3e54d8c660cddc38d7869af656d801d22b19"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "16839d1d0fcecb39e52d1bc3a65aa02d62c1ee6d" "4fb08c80d2897450152dd9777a491e0d9c0e2091" "b8033e453c7d9518619e90fb100d7d90d7b4026d" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk chromeos-config primary_io_manager featured .gn"

PLATFORM_NATIVE_TEST="yes"
PLATFORM_SUBDIR="primary_io_manager"

# Do not run test parallelly until unit tests are fixed.
# shellcheck disable=SC2034
PLATFORM_PARALLEL_GTEST_TEST="no"

inherit cros-workon platform udev user

DESCRIPTION="Primary IO Manager for Chromium OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/primary_io_manager/"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="cfm_enabled_device"

COMMMON_DEPEND="
	chromeos-base/chromeos-config-tools:=
	chromeos-base/featured:=
	chromeos-base/net-base:=
	chromeos-base/patchpanel-client:=
	chromeos-base/system_api:=
	dev-cpp/abseil-cpp:=
	sys-apps/dbus:=
	virtual/libusb:1
	virtual/libudev:=
"

RDEPEND="
	${COMMMON_DEPEND}
	acct-group/iomanager
	acct-user/iomanager
	virtual/udev
"
DEPEND="${COMMMON_DEPEND}
	sys-kernel/linux-headers:=
"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
"

src_install() {
	platform_src_install

	dobin "${OUT}"/primary_io_manager

	# Install upstart configuration
	insinto /etc/init
	doins primary_io_manager.conf

	# DBus configuration
	insinto /etc/dbus-1/system.d
	doins dbus/org.chromium.PrimaryIoManager.conf
}

platform_pkg_test() {
	local tests=(
		primary_io_manager_test
	)

	local test_bin
	for test_bin in "${tests[@]}"; do
		platform_test "run" "${OUT}/${test_bin}"
	done
}

pkg_preinst() {
	enewuser "iomanager"
	enewgroup "iomanager"
}
