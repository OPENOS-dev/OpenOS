# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="7c7201e0ed0edcca7f6a064db47655aa44cfd9f4"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "c3bbb4d4f47be60af4a0d38478a92f038e3f4fc6" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_DESTDIR="${S}/platform2"
CROS_WORKON_SUBTREE="common-mk lorgnette .gn"

PLATFORM_SUBDIR="lorgnette"

inherit cros-workon platform cros-protobuf

LICENSE="BSD-Google"
KEYWORDS="*"
SLOT="0/0"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
"

RDEPEND="
	chromeos-base/lorgnette
	chromeos-base/lorgnette_cli
	dev-libs/re2:=
"

DEPEND="${RDEPEND}
	chromeos-base/permission_broker-client:=
	chromeos-base/system_api:=
	dev-cpp/abseil-cpp:=
	media-libs/libjpeg-turbo:=
	media-libs/libpng:=
	media-gfx/sane-backends:=
	sys-apps/util-linux:=
	virtual/jpeg:0=
	virtual/libusb:1
"

src_install() {
	# platform_src_install omitted, to avoid conflicts with
	# chromeos-base/lorgnette.

	use compilation_database && platform_install_compilation_database

	dobin "${OUT}"/sane_backend_wwcb_tests
}
