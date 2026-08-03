# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk dns-proxy metrics shill/dbus/client .gn"

PLATFORM_SUBDIR="dns-proxy"

inherit cros-workon platform cros-protobuf user

DESCRIPTION="A daemon that provides DNS proxying services."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/dns-proxy/"

LICENSE="BSD-Google"
SLOT="0/0"
KEYWORDS="~*"

COMMON_DEPEND="
	chromeos-base/metrics:=
	chromeos-base/minijail:=
	chromeos-base/net-base:=
	chromeos-base/patchpanel:=
	chromeos-base/patchpanel-client:=
	chromeos-base/session_manager-client:=
	chromeos-base/shill-client:=
	chromeos-base/shill-dbus-client:=
	chromeos-base/system_api:=[fuzzer?]
	sys-apps/dbus:=
	net-dns/c-ares:=
	net-misc/curl:=
"
RDEPEND="
	${COMMON_DEPEND}
	!<chromeos-base/shill-0.0.6
"

DEPEND="
	${COMMON_DEPEND}
	chromeos-base/permission_broker-client:=
"

BDEPEND="
	chromeos-base/minijail
"

pkg_preinst() {
	enewuser "dns-proxy"
	enewgroup "dns-proxy"
	enewuser "dns-proxy-system"
	enewgroup "dns-proxy-system"
	enewuser "dns-proxy-user"
	enewgroup "dns-proxy-user"
}

src_install() {
	platform_src_install

	dosym /run/dns-proxy/resolv.conf /etc/resolv.conf

	local fuzzer_component_id="1493959"
	platform_fuzzer_install "${S}"/OWNERS "${OUT}"/ares_client_fuzzer \
		--comp "${fuzzer_component_id}"
	platform_fuzzer_install "${S}"/OWNERS "${OUT}"/doh_curl_client_fuzzer \
		--comp "${fuzzer_component_id}"
	platform_fuzzer_install "${S}"/OWNERS "${OUT}"/resolver_fuzzer \
		--comp "${fuzzer_component_id}"
}
