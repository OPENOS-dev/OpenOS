# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="114c05cb05e742094c8a4d913d00531b04908c72"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "9576f262a57fd7a5bff4991afa8dcb9c6650c635" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk net-base .gn"

PLATFORM_SUBDIR="net-base"
# Do not run test in parallel for net-base by default. There doesn't seem to be
# too much benefit on speed now but affects the debugging experience.
# shellcheck disable=SC2034
PLATFORM_PARALLEL_GTEST_TEST="no"

inherit cros-workon libchrome platform

DESCRIPTION="Networking primitive library"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/net-base/"
LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="fuzzer test"

COMMON_DEPEND="
	chromeos-base/minijail:=
	dev-cpp/abseil-cpp:=
	test? ( dev-libs/re2:= )
	net-dns/c-ares:=
"
DEPEND="
	${COMMON_DEPEND}
"
RDEPEND="
	${COMMON_DEPEND}
"

src_install() {
	platform_src_install

	local platform_network_component_id="1493959"
	platform_fuzzer_install "${S}"/OWNERS "${OUT}/http_url_fuzzer" \
		--comp "${platform_network_component_id}"
	platform_fuzzer_install "${S}"/OWNERS "${OUT}/netlink_attribute_list_fuzzer" \
		--comp "${platform_network_component_id}"
	platform_fuzzer_install "${S}"/OWNERS "${OUT}/rtnl_handler_fuzzer" \
		--comp "${platform_network_component_id}"
}
