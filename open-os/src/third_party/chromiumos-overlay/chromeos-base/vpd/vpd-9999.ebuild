# Copyright 2012 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_PROJECT=("chromiumos/platform2" "chromiumos/platform/vpd")
CROS_WORKON_LOCALNAME=("platform2" "platform/vpd")
CROS_WORKON_DESTDIR=("${S}/platform2" "${S}/platform2/vpd")
CROS_WORKON_SUBTREE=("common-mk .gn" "")

PLATFORM_SUBDIR="vpd"

WANT_LIBCHROME="no"
WANT_LIBBRILLO="no"

inherit cros-workon platform

DESCRIPTION="ChromeOS vital product data utilities"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/vpd/"
SRC_URI="gs://chromeos-localmirror/distfiles/${PN}-testdata-0.0.3.tar.xz"

LICENSE="BSD-Google"
KEYWORDS="~*"
IUSE="cros_host test"

# util-linux is for libuuid.
DEPEND="
	dev-cpp/abseil-cpp:=
	sys-apps/flashmap:=
	sys-apps/flashrom:=
	sys-apps/util-linux:=
"

# chromeos-activate-date for ActivateDate upstart and script.
RDEPEND="
	${DEPEND}
	test? (
		app-alternatives/tar
		chromeos-base/libbrillo
		chromeos-base/libchrome
		sys-apps/coreutils
	)
	fuzzer? (
		chromeos-base/libbrillo
		chromeos-base/libchrome
	)
	!cros_host? (
		virtual/chromeos-activate-date
	)
"

src_unpack() {
	platform_src_unpack
	cd "${S}" || die
	unpack "${A}"
}

src_install() {
	platform_src_install

	# ChromeOS > Platform > Enablement > Firmware > BIOS
	local component="167186"
	local fuzzer
	for fuzzer in cache cache_file decoder raw_decoder; do
		platform_fuzzer_install "${S}"/OWNERS \
			"${OUT}/vpd_${fuzzer}_fuzzer" --comp "${component}"
	done
}

platform_pkg_test() {
	platform test_all

	# This is not a gtest binary; avoid platform_test appending
	# gtest-specific args.
	PLATFORM_PARALLEL_GTEST_TEST="no" \
		platform_test run "tests/run_tests"
}
