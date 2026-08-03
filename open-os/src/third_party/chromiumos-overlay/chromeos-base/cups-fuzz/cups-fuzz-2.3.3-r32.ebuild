# Copyright 2018 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="489a619ce5576ef4a2d62e97610fcc261345756e"
CROS_WORKON_TREE=("e0d938d434ef1d71ab5f5adb1471acf5c9dfaa0a" "5338dadb72025d79f498b026ea8d0bc8395fdcfa")
CROS_WORKON_LOCALNAME="third_party/cups"
CROS_WORKON_PROJECT="chromiumos/third_party/cups"
CROS_WORKON_EGIT_BRANCH="chromeos"
CROS_WORKON_SUBTREE="fuzzers DIR_METADATA"
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-fuzzer cros-sanitizers cros-workon flag-o-matic libchrome cros-toolchain-funcs

DESCRIPTION="Fuzzer for PPD and IPP functions in CUPS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/third_party/cups/+/HEAD/fuzzers/"
SRC_URI=""

LICENSE="BSD-Google"
SLOT="0/0"
KEYWORDS="*"
IUSE="asan fuzzer"

COMMON_DEPEND="net-print/cups:=[fuzzer]"
RDEPEND="${COMMON_DEPEND}"
DEPEND="${COMMON_DEPEND}"

# We really don't want to be building this otherwise.
REQUIRED_USE="fuzzer"

src_unpack() {
	cros-workon_src_unpack
}

src_configure() {
	sanitizers-setup-env || die
	fuzzer-setup-binary || die
	append-ldflags "$("${CHOST}"-cups-config --libs)"
	append-ldflags "$($(tc-getPKG_CONFIG) --libs libchrome)"
	append-cppflags "$($(tc-getPKG_CONFIG) --cflags libchrome)"
	append-cxxflags -std=gnu++23
}

src_compile() {
	local build_dir="$(cros-workon_get_build_dir)"
	VPATH="${S}"/fuzzers emake -C "${build_dir}" cups_ppdopen_fuzzer
	VPATH="${S}"/fuzzers emake -C "${build_dir}" cups_ippreadio_fuzzer
	VPATH="${S}"/fuzzers emake -C "${build_dir}" cups_ipp_t_fuzzer
}

src_install() {
	local build_dir="$(cros-workon_get_build_dir)"
	local fuzzer_component_id="167231"
	fuzzer_install "${S}"/fuzzers/OWNERS "${build_dir}"/cups_ppdopen_fuzzer \
		--comp "${fuzzer_component_id}"
	fuzzer_install "${S}"/fuzzers/OWNERS "${build_dir}"/cups_ippreadio_fuzzer \
		--comp "${fuzzer_component_id}"
	fuzzer_install "${S}"/fuzzers/OWNERS "${build_dir}"/cups_ipp_t_fuzzer \
		--comp "${fuzzer_component_id}"
}
