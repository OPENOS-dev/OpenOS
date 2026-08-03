# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_SUBTREE="common-mk chromeos-config modemloggerd .gn"

PLATFORM_SUBDIR="modemloggerd"

inherit cros-workon cros-protobuf platform user

DESCRIPTION="Chrome OS modem logger"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/modemloggerd"

LICENSE="BSD-Google"
KEYWORDS="~*"
IUSE=""

DEPEND="
	chromeos-base/chromeos-config-tools:=
	"

RDEPEND="
	${DEPEND}
	net-misc/fibocom-tools:=
	net-misc/qlog:=
	"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
"

src_install() {
	platform_src_install
	dobin "${OUT}"/modemloggerd

	# Install DBus config.
	insinto /etc/dbus-1/system.d
	doins dbus/org.chromium.Modemloggerd.conf

	# Install DBus interface.
	insinto /usr/share/dbus-1/interfaces
	doins dbus_bindings/org.chromium.Modemloggerd.Manager.xml
	doins dbus_bindings/org.chromium.Modemloggerd.Modem.xml

	# Install helper scripts.
	exeinto /usr/local/libexec/modemloggerd
	doexe scripts/rw101_set_usbmode.sh

	insinto /usr/share/modemloggerd
	doins helper_manifest.textproto
}
