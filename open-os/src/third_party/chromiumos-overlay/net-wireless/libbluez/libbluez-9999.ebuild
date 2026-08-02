# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"
# To support choosing between current and next versions, two cros-workon
# projects are declared. During emerge, both project sources are copied to
# their respective destination directories, and one is chosen as the
# "working directory" in src_unpack() below based on bluez-next USE flag.
CROS_WORKON_LOCALNAME=("bluez/current" "bluez/next")
CROS_WORKON_PROJECT=("chromiumos/third_party/bluez" "chromiumos/third_party/bluez")
CROS_WORKON_OPTIONAL_CHECKOUT=(
	"use !bluez-next"
	"use bluez-next"
)
CROS_WORKON_DESTDIR=("${S}/bluez/current" "${S}/bluez/next")
CROS_WORKON_EGIT_BRANCH=("chromeos-5.54" "chromeos-5.54")

inherit autotools cros-sanitizers cros-workon flag-o-matic

DESCRIPTION="Bluetooth library for Linux"
HOMEPAGE="http://www.bluez.org/"
#SRC_URI not defined because we get our source locally

LICENSE="GPL-2 LGPL-2.1"
KEYWORDS="~*"
IUSE="asan bluez-next debug"
REQUIRED_USE="?? ( bluez-next )"

CDEPEND="
	>=dev-libs/glib-2.14:2=
	sys-apps/dbus:=
	virtual/libudev:=
"
DEPEND="${CDEPEND}"
RDEPEND="${CDEPEND}
	!<net-wireless/bluez-5.54-r1186
"
BDEPEND=""

DOCS=( AUTHORS ChangeLog README )

src_unpack() {
	cros-workon_src_unpack

	# Setting S has the effect of changing the temporary build directory
	# here onwards. Choose "bluez/next" or "bluez/current" subdir depending on
	# the USE flag.
	local checkout="bluez/$(usex bluez-next next current)"
	S+="/${checkout}"
	local version="$("${FILESDIR}"/chromeos-version.sh "${S}")"
	einfo "Using checkout ${checkout} (version ${version})"
}

src_prepare() {
	default

	eautoreconf
}

src_configure() {
	sanitizers-setup-env
	# Workaround a global-buffer-overflow warning in asan build.
	# See crbug.com/748216 for details.
	if use asan; then
		append-flags '-mllvm -asan-globals=0'
	fi

	econf \
		--disable-tools \
		--localstatedir=/var \
		--disable-cups \
		$(use_enable debug) \
		--disable-test \
		--enable-library \
		--disable-systemd \
		--disable-obex \
		--disable-sixaxis \
		--disable-network \
		--disable-datafiles \
		--disable-admin \
		--disable-fuzzer \
		--disable-hid2hci \
		--disable-deprecated \
		--disable-monitor \
		--disable-manpages \

}

src_compile() {
	emake lib/libbluetooth.la
}

src_install() {
	emake DESTDIR="${D}" install-libLTLIBRARIES install-pkgincludeHEADERS install-pkgconfigDATA
}
