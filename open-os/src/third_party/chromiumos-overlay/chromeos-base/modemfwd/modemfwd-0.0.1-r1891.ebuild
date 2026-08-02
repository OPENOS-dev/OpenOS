# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "16839d1d0fcecb39e52d1bc3a65aa02d62c1ee6d" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "e3245342910d5a13571938d94b0cee15bc77f927" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk chromeos-config metrics modemfwd .gn"

PLATFORM_SUBDIR="modemfwd"

inherit cros-workon platform cros-protobuf tmpfiles udev user

DESCRIPTION="Modem firmware updater daemon"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/modemfwd"

LICENSE="BSD-Google"
SLOT="0/0"
KEYWORDS="*"
IUSE="fuzzer"

COMMON_DEPEND="
	app-arch/xz-utils:=
	chromeos-base/chromeos-config-tools:=
	chromeos-base/dlcservice-client:=
	chromeos-base/metrics:=
	chromeos-base/patchmaker:=
	net-misc/modemmanager-next:=
"

RDEPEND="${COMMON_DEPEND}"

DEPEND="${COMMON_DEPEND}
	chromeos-base/chromeos-dbus-bindings:=
	chromeos-base/minijail:=
	chromeos-base/shill-client:=
	chromeos-base/system_api:=[fuzzer?]
	dev-libs/re2:=
	fuzzer? ( dev-libs/libprotobuf-mutator:= )
"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
	chromeos-base/minijail
"

src_install() {
	platform_src_install

	dotmpfiles tmpfiles.d/*.conf

	# udev scripts and rules.
	exeinto "$(get_udevdir)"
	doexe udev/*.sh

	local fuzzer_component_id="167157"
	platform_fuzzer_install "${S}"/OWNERS "${OUT}"/firmware_manifest_v2_fuzzer \
		--comp "${fuzzer_component_id}"

	insinto /usr/share/policy
	newins "seccomp/modemfwd-mbimcli-seccomp-${ARCH}.policy" modemfwd-mbimcli-seccomp.policy
}
