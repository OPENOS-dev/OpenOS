# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="38272f61162fa7fa1b6dfa7a6afc1ac8d2f9650b"
CROS_WORKON_TREE=("95d4df7f9212a6560d43b5e6eee5c905a64eefd8" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "4934b6b332f2a3db7a26bad9f888607a4f12b440" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_USE_VCSID="1"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="biod common-mk libec .gn"

PLATFORM_SUBDIR="libec"

inherit cros-workon platform

DESCRIPTION="Embedded Controller Library for Chromium OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/libec"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="cros_host"

COMMON_DEPEND="
	chromeos-base/chromeos-ec-headers:=
	!cros_host? ( chromeos-base/power_manager-client:= )
	dev-cpp/abseil-cpp:=
	virtual/libusb:1=
	sys-apps/flashmap:=
"

RDEPEND="
	${COMMON_DEPEND}
	"

DEPEND="
	${COMMON_DEPEND}
	chromeos-base/system_api
"

src_install() {
	platform_src_install

	# Install fuzzers.
	local fuzzer_component_id="782045"
	local fuzz_targets=(
		"libec_ec_panicinfo_fuzzer"
	)
	local fuzz_target
	for fuzz_target in "${fuzz_targets[@]}"; do
		platform_fuzzer_install "${S}"/OWNERS "${OUT}"/"${fuzz_target}" \
			--comp "${fuzzer_component_id}"
	done
}
