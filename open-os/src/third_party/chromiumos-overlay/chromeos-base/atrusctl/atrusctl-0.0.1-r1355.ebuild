# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT=("e67643c64a105f6f744b007eb857f381ace07e8e" "218e56e138d3178127c10d8644bc5f3193850f2f")
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "472fbf45e8514a009b637c6a7ffbd5088e83b880")
CROS_WORKON_LOCALNAME=("platform2" "third_party/atrusctl")
CROS_WORKON_PROJECT=("chromiumos/platform2" "chromiumos/third_party/atrusctl")
CROS_WORKON_DESTDIR=("${S}/platform2" "${S}/platform2/atrusctl")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_SUBTREE=("common-mk .gn" "")

PLATFORM_SUBDIR="atrusctl"

inherit cros-workon platform udev user

DESCRIPTION="CrOS daemon for the Atrus speakerphone"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/third_party/atrusctl/"

LICENSE="BSD-Google"
KEYWORDS="*"

DEPEND="
	dev-cpp/abseil-cpp:0=
	virtual/libusb:1=
	virtual/libudev:0=
"
RDEPEND="
	${DEPEND}
	!sys-apps/atrusctl
"

src_install() {
	platform_src_install

	dosbin "${OUT}/atrusd"

	insinto /etc/rsyslog.d
	newins conf/rsyslog-atrus.conf atrus.conf

	udev_newrules conf/udev-atrus.rules 99-atrus.rules

	insinto /etc/init
	doins init/atrusd.conf

	insinto /etc/dbus-1/system.d
	doins dbus_permissions/org.chromium.Atrusctl.conf
}

pkg_preinst() {
	enewuser atrus
	enewgroup atrus
}
