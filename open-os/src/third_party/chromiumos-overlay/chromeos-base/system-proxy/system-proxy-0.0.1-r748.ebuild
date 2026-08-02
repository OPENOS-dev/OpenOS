# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "55903d6d672e8bdad40486765d522b08aa485668" "795934cc82a4d8c623985bb6fab7c4c6544c8f83" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk libpasswordprovider system-proxy .gn"

PLATFORM_SUBDIR="system-proxy"
# Do not run test parallelly for system-proxy. See b/293997430.
# shellcheck disable=SC2034
PLATFORM_PARALLEL_GTEST_TEST="no"

inherit cros-workon platform cros-protobuf user

DESCRIPTION="A daemon that provides authentication support for system services
and ARC apps behind an authenticated web proxy."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/system-proxy/"

LICENSE="BSD-Google"
SLOT="0/0"
KEYWORDS="*"
IUSE="fuzzer"

COMMON_DEPEND="
	chromeos-base/libpasswordprovider:=
	chromeos-base/minijail:=
	chromeos-base/net-base:=
	chromeos-base/patchpanel:=
	chromeos-base/patchpanel-client:=
	sys-apps/dbus:=
	net-misc/curl:=
"
RDEPEND="${COMMON_DEPEND}"
DEPEND="
	${COMMON_DEPEND}
	chromeos-base/permission_broker-client:=
	fuzzer? ( dev-libs/libprotobuf-mutator:= )
"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
	chromeos-base/minijail
"

pkg_preinst() {
	enewuser "system-proxy"
	enewgroup "system-proxy"
}

src_install() {
	platform_src_install

	if use fuzzer; then
		local fuzzer_component_id="156085"
		platform_fuzzer_install "${S}"/OWNERS "${OUT}"/system_proxy_connect_headers_parser_fuzzer \
			--comp "${fuzzer_component_id}"
		platform_fuzzer_install "${S}"/OWNERS "${OUT}"/system_proxy_worker_config_fuzzer \
			--comp "${fuzzer_component_id}"
		platform_fuzzer_install "${S}"/OWNERS "${OUT}"/system_proxy_http_util_fuzzer \
			--comp "${fuzzer_component_id}"
	fi
}
