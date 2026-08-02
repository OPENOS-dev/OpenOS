# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "b9456d2f3facf22d130c0d8aa5110c4962620af7" "5e69788a6380c17829c093b9a794fb0248def844" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk libpmt pmt_tool .gn"

PLATFORM_NATIVE_TEST="yes"
PLATFORM_SUBDIR="pmt_tool"

WANT_LIBBRILLO="yes"
WANT_LIBCHROME="yes"

inherit cros-workon platform

DESCRIPTION="Command-line utility to sample and decode the Intel PMT telemetry data"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/pmt_tool"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="test"

COMMON_DEPEND="
	chromeos-base/libpmt:=
	dev-libs/protobuf:=
	dev-cpp/abseil-cpp:=
"

RDEPEND="
	${COMMON_DEPEND}
"

DEPEND="
	${COMMON_DEPEND}
	net-analyzer/netdata:=
"
src_install() {
	platform_src_install

	insinto /usr/local/etc/netdata/
	doins "${FILESDIR}"/pmt_netdata.conf
}

pkg_postinst() {
	# Append pmt_tool's specific netdata plugin config to netdata.conf.
	# netdata.conf is owned and already installed by the netdata pkg,
	# it is not part of pmt_tool's installation, hence we're editing it in $ROOT.
	# $ROOT is not accessible during the src_install phase,
	# can only be accessed during pkg_ phases.
	local pmt_netdata_conf="${ROOT}/usr/local/etc/netdata/pmt_netdata.conf"
	local netdata_conf="${ROOT}/usr/local/etc/netdata/netdata.conf"
	local plugin_list="${ROOT}/usr/local/etc/netdata/.user_plugins.lst"

	# Appending netdata.conf with pmt_plugin specific config if not there already.
	if ! grep -Fxq "intel_pmt" "${plugin_list}"; then
	echo -e "\nintel_pmt" >> "${plugin_list}" || die
	cat "${pmt_netdata_conf}" >> "${netdata_conf}" || die
	fi

	rm -rf "${pmt_netdata_conf}"
}
