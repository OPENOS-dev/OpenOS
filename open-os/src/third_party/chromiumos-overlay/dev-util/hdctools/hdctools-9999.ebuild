# Copyright 2011 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_PROJECT="chromiumos/third_party/hdctools"
PYTHON_COMPAT=( python3_11 )

CROS_PROTOBUF_APPLY_DEFAULT_DEPS=0
inherit cros-workon distutils-r1 cros-toolchain-funcs udev cros-sanitizers cros-protobuf

DESCRIPTION="Software to communicate with servo/miniservo debug boards"
HOMEPAGE="https://www.chromium.org/chromium-os/servo"

LICENSE="BSD-Google"
KEYWORDS="~*"
IUSE="cros_host test"

COMMON_DEPEND="
	>=dev-embedded/libftdi-0.18:=
	dev-python/numpy[${PYTHON_USEDEP}]
	>=dev-python/pexpect-3.0[${PYTHON_USEDEP}]
	dev-python/pyserial[${PYTHON_USEDEP}]
	>=dev-python/pyusb-1.0.2[${PYTHON_USEDEP}]
	dev-python/packaging[${PYTHON_USEDEP}]
	virtual/libusb:1=
	chromeos-base/ec-devutils:=[${PYTHON_USEDEP}]
	chromeos-base/pigweed-utils[${PYTHON_USEDEP}]
	dev-python/backoff[${PYTHON_USEDEP}]
"

RDEPEND="${COMMON_DEPEND}
	!<chromeos-base/ec-devutils-0.0.3
	virtual/servo-config-dut-usb3:*
"

DEPEND="${COMMON_DEPEND}
"

BDEPEND="
	${CROS_PROTOC_DEPS}
	dev-python/grpcio-tools[${PYTHON_USEDEP}]
	test? ( dev-python/pytest[${PYTHON_USEDEP}] )
"

DIRS="servo servo_updater ec3po usbkm232 measurement_tools"

src_prepare() {
	for dir in ${DIRS}; do
		(cd "${dir}" || die ; distutils-r1_src_prepare)
	done
}

src_configure() {
	for dir in ${DIRS}; do
		(cd "${dir}" || die ; sanitizers-setup-env)
		(cd "${dir}" || die ; distutils-r1_src_configure)
	done
}

src_compile() {
	tc-export CC PKG_CONFIG
	local makeargs=( $(usex cros_host '' EXTRA_DIRS=) )
	emake "${makeargs[@]}"
	for dir in ${DIRS}; do
		(cd "${dir}" || die ; distutils-r1_src_compile)
	done
}


src_install() {
	local makeargs=(
		$(usex cros_host '' EXTRA_DIRS=)
		DESTDIR="${D}"
		LIBDIR="/usr/$(get_libdir)"
		UDEV_DEST="${D}$(get_udevdir)/rules.d"
		install
	)
	emake "${makeargs[@]}"
	for dir in ${DIRS}; do
		(cd "${dir}" || die ; distutils-r1_src_install)
	done
}
