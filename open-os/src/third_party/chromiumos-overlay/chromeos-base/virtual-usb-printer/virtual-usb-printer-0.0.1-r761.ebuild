# Copyright 2019 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT=("e67643c64a105f6f744b007eb857f381ace07e8e" "ab7d4776a61e945cf9ed97c763d667e1b113ef42")
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "bb39b5688504c6fcdc0ea296de7a23e79f808a0a")
CROS_WORKON_LOCALNAME=("platform2" "third_party/virtual-usb-printer")
CROS_WORKON_PROJECT=("chromiumos/platform2" "chromiumos/third_party/virtual-usb-printer")
CROS_WORKON_EGIT_BRANCH=("main" "virtual-printer")
CROS_WORKON_DESTDIR=("${S}/platform2" "${S}/platform2/virtual-usb-printer")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_SUBTREE=("common-mk .gn" "")

PLATFORM_SUBDIR="virtual-usb-printer"

inherit cros-workon platform cros-protobuf

DESCRIPTION="Used with USBIP to provide a virtual USB printer for testing."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/third_party/virtual-usb-printer/"

LICENSE="GPL-2"
KEYWORDS="*"

IUSE=""
# Do not run UsbIpManagerTest.* test parallelly until unit tests are fixed.
#TODO(b/337117666): Fix unittest failure with gtest-parallel.
# shellcheck disable=SC2034
PLATFORM_PARALLEL_GTEST_TEST="no"

RDEPEND="
	chromeos-base/libipp:=
	dev-libs/libxml2:=
	net-misc/usbip:=
	virtual/jpeg:0=
"

DEPEND="${RDEPEND}"

src_install() {
	platform_src_install

	insinto /usr/local/etc/virtual-usb-printer
	doins config/escl_capabilities.json
	doins config/escl_capabilities_large_paper_sizes.json
	doins config/escl_capabilities_left_justified.json
	doins config/escl_capabilities_center_justified.json
	doins config/escl_capabilities_right_justified.json
	doins config/ipp_attributes.json
	doins config/ipp_attributes_pwgraster.json
	doins config/ippusb_printer.json
	doins config/ippusb_backflip_printer.json
	doins config/ippusb_printer_plus_storage.json
	doins config/stub_usb_fujitsu_scanner.json
	doins config/usb_printer.json

	# Install upstart files into rootfs, since upstart won't look in
	# /usr/local/etc.
	insinto /etc/init
	doins init/virtual-usb-printer.conf
}
