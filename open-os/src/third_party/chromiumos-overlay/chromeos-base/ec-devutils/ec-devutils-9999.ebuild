# Copyright 2014 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"
CROS_WORKON_PROJECT="chromiumos/platform/ec"
CROS_WORKON_LOCALNAME="platform/ec"
PYTHON_COMPAT=( python3_{8..11} )

# This ebuild is upreved via PuPR, so disable the normal uprev process for
# cros-workon ebuilds.
#
# To uprev manually, run:
#    cros_mark_as_stable --force --overlay-type public --packages \
#     chromeos-base/ec-devutils commit
CROS_WORKON_MANUAL_UPREV="1"

inherit cros-workon distutils-r1 libchrome cros-sanitizers

DESCRIPTION="Host development utilities for Chromium OS EC"
HOMEPAGE="https://www.chromium.org/chromium-os/ec-development"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="~*"
IUSE="hammerd"

DEPEND="virtual/libusb:1=
	sys-apps/flashmap:=
	dev-embedded/libftdi:=
	dev-libs/boringssl:=
	chromeos-base/libec:=
	"
RDEPEND="
	${DEPEND}
	app-mobilephone/dfu-util
	sys-firmware/servo-firmware
	sys-apps/flashrom
	!<chromeos-base/ec-utils-0.0.1-r6153
	chromeos-base/ec-utils
	chromeos-base/zephyr-npcx-monitor
	chromeos-base/zephyr-rtk-flame
	>=dev-python/pyusb-1.0.2[${PYTHON_USEDEP}]
	"
BDEPEND="
	dev-python/setuptools[${PYTHON_USEDEP}]
	virtual/pkgconfig
	"
# b/274791539: gtest is required because libec includes a libchrome header that
# requires gtest to be installed when building.
DEPEND+="
	dev-cpp/gtest
"

set_board() {
	# No need to be board specific, no tools below build code that is
	# EC specific. However, the EC build system must ensure that all
	# utilities in this package are built for a given board. We ensure this
	# for the host board.
	export BOARD="host"
}

src_configure() {
	sanitizers-setup-env
	distutils-r1_src_configure
}

src_compile() {
	tc-export AR CC PKG_CONFIG RANLIB
	# In platform/ec Makefile, it uses "CC" to specify target chipset and
	# "HOSTCC" to compile the utility program because it assumes developers
	# want to run the utility from same host (build machine).
	# In this ebuild file, we only build utility
	# and we may want to build it so it can
	# be executed on target devices (i.e., arm/x86/amd64), not the build
	# host (BUILDCC, amd64). So we need to override HOSTCC by target "CC".
	export HOSTCC="${CC}"
	set_board

	# b/247791129: EC expects HOST_PKG_CONFIG to be the pkg-config targeting the
	# platform that the EC is running on top of (e.g., the Chromebook's AP).
	# That platform corresponds to the ChromeOS "$BOARD" and the pkg-config for
	# the "$BOARD" being built is specified by tc-getPKG_CONFIG.
	export HOST_PKG_CONFIG
	HOST_PKG_CONFIG=$(tc-getPKG_CONFIG)

	# EC expects BUILD_PKG_CONFIG to be the pkg-config targeting the build
	# machine (the machine doing the compilation).
	export BUILD_PKG_CONFIG
	BUILD_PKG_CONFIG=$(tc-getBUILD_PKG_CONFIG)

	# When building for the host, EC ignores CFLAGS. Instead, EC expects
	# HOST_CFLAGS to be the CFLAGS targeting the platform that the EC
	# is running on top of (i.e., the Chromebook's AP).
	export HOST_CFLAGS
	HOST_CFLAGS="${CFLAGS}"

	# When building for the host, EC ignores LDFLAGS. Instead, EC expects
	# HOST_LDFLAGS to be the LDFLAGS targeting the platform that the EC
	# is running on top of (i.e., the Chromebook's AP).
	export HOST_LDFLAGS
	HOST_LDFLAGS="${LDFLAGS}"

	emake CC="${CC}" utils-host
	# Add usb_updater2 for servo or hammer updates.
	emake -C extra/usb_updater usb_updater2
	if use hammerd; then
		# Add touchpad_updater for TP update on hammer.
		emake -C extra/touchpad_updater touchpad_updater
	fi
	distutils-r1_src_compile
}

src_test() {
	emake BOARD=host utils-test out="${WORKDIR}/build/host"
}

src_install() {
	set_board
	dobin "build/${BOARD}/util/stm32mon"
	dobin "build/${BOARD}/util/ec_parse_panicinfo"
	dobin "build/${BOARD}/util/uartupdatetool"
	dobin "build/${BOARD}/util/iteflash"
	dobin "build/${BOARD}/util/itecomdbgr"
	dobin "build/${BOARD}/util/rtkupdate"

	# Add usb_updater2 for servo or hammer updates.
	dosbin "extra/usb_updater/usb_updater2"
	if use hammerd; then
		# Add touchpad_updater for TP update on hammer.
		newsbin "extra/touchpad_updater/touchpad_updater" ec_touchpad_updater
	fi

	dobin "util/flash_ec"
	dobin "util/uart_stress_tester.py"
	insinto /usr/share/ec-devutils
	doins util/openocd/*

	distutils-r1_src_install
}
