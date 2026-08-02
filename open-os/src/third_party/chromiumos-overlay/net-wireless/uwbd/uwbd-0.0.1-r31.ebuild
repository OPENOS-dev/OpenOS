# Copyright 2022 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT=("9e797743280a9707a4ae38e2d331d202883c1115" "f8d6678bc1261194d25829450c4ccc623e4d820d")
CROS_WORKON_TREE=("438e3c7adbf0b3e95a8a5548979979dcbd86107d" "13b0650948f7604a6abd3c724597399417a17a1b")
CROS_WORKON_PROJECT=("chromiumos/platform2" "aosp/platform/hardware/nxp/uwb")
CROS_WORKON_LOCALNAME=("../platform2" "../aosp/hardware/nxp/uwb")
CROS_WORKON_DESTDIR=("${S}" "${S}/uwbd/nxp_hal/")
CROS_WORKON_SUBTREE=("uwbd" "")

CROS_WORKON_INCREMENTAL_BUILD=1

inherit flag-o-matic cros-workon cros-rust cros-protobuf udev user libchrome

DESCRIPTION="The UWB D-Bus daemon"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/uwbd"

LICENSE="BSD-Google"
SLOT="0/0"
KEYWORDS="*"
IUSE="uwbd_client"

DEPEND="
	dev-rust/third-party-crates-src:=
	dev-rust/chromeos-dbus-bindings:=
	dev-rust/libchromeos:=
	net-wireless/uwb_core:=
	chromeos-base/libbrillo:=
	sys-apps/dbus:=
"
RDEPEND="${DEPEND}"

pkg_preinst() {
	# Create user and group for uwbd
	enewuser "uwbd"
	enewgroup "uwbd"
}

src_configure() {
	append-cppflags -DBASE_VER="$(libchrome_ver)"
	cros-rust_src_configure
}

src_install() {
	# Install the uwbd binary.
	dobin "$(cros-rust_get_build_dir)/uwbd"
	if use uwbd_client; then
		dobin "$(cros-rust_get_build_dir)/uwbd_client"
	fi

	# Install udev rule
	udev_dorules "${S}/udev/99-uwb.rules"

	# Install the DBus config.
	insinto /etc/dbus-1/system.d
	doins dbus/org.chromium.uwbd.conf

	# Install the upstart config.
	insinto /etc/init
	doins upstart/uwbd.conf

	# Install HAL config.
	insinto /etc/uwb
	doins "${S}/nxp_hal/halimpl/config/SR1XX/libuwb-nxp-type2gc-es1.conf"

	# Install the seccomp filter.
	insinto /usr/share/policy
	doins upstart/seccomp/uwbd-seccomp.policy
}
