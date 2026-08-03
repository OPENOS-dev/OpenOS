# Copyright 2014 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "d86bcd4d3ff251e26927460520d65dde23124f54" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD="1"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk debugd metrics .gn"

PLATFORM_SUBDIR="debugd"

inherit cros-workon platform cros-protobuf user

DESCRIPTION="Chrome OS debugging service"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/debugd/"
LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="arcvm cellular fbpreprocessord iwlwifi_dump nvme sata ti50_onboard tpm ufs"

COMMON_DEPEND="
	app-arch/xz-utils:=
	chromeos-base/chromeos-login:=
	chromeos-base/cryptohome-client:=
	chromeos-base/net-base:=
	chromeos-base/minijail:=
	chromeos-base/metrics:=
	chromeos-base/power_manager-client:=
	chromeos-base/shill-client:=
	chromeos-base/system_api:=
	chromeos-base/vboot_reference:=
	chromeos-base/vpd:=
	dev-libs/re2:=
	net-libs/libpcap:=
	net-misc/curl:=
	net-wireless/iw:=
	sys-apps/rootdev:=
	sys-libs/libcap:=
	sata? ( sys-apps/smartmontools:= )
"
RDEPEND="${COMMON_DEPEND}
	iwlwifi_dump? ( chromeos-base/intel-wifi-fw-dump )
	nvme? ( sys-apps/nvme-cli )
	ufs? (
		sys-apps/sg3_utils
		sys-apps/ufs-utils
	)
	chromeos-base/chromeos-ssh-testkeys
	chromeos-base/chromeos-sshd-init
	chromeos-base/libsegmentation
	chromeos-base/runtime_hwid_utils
	!chromeos-base/workarounds
	sys-apps/iproute2
	sys-apps/memtester
"
DEPEND="${COMMON_DEPEND}
	chromeos-base/debugd-client:=
	chromeos-base/fbpreprocessord-client:=
	sys-apps/dbus:="

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
	chromeos-base/minijail
"

pkg_setup() {
	# Has to be done in pkg_setup() instead of pkg_preinst() since
	# src_install() needs debugd.
	enewuser "debugd"
	enewgroup "debugd"

	cros-workon_pkg_setup
}

pkg_preinst() {
	enewuser "debugd-logs"
	enewgroup "debugd-logs"

	enewgroup "daemon-store"
	enewgroup "logs-access"
}

src_install() {
	platform_src_install

	local debugd_seccomp_dir="src/helpers/seccomp"
	# Install seccomp policies.
	insinto /usr/share/policy
	local policy
	for policy in "${debugd_seccomp_dir}"/*-"${ARCH}".policy; do
		local policy_basename="${policy##*/}"
		local policy_name="${policy_basename/-${ARCH}}"
		newins "${policy}" "${policy_name}"
	done

	# Install DBus configuration.
	insinto /etc/dbus-1/system.d
	doins share/org.chromium.debugd.conf

	insinto /etc/init
	doins share/debugd.conf share/kernel-features.json

	insinto /etc/perf_commands
	doins -r share/perf_commands/*

	local daemon_store="/etc/daemon-store/debugd"
	dodir "${daemon_store}"
	fperms 0750 "${daemon_store}"
	fowners debugd:debugd "${daemon_store}"

	local fuzzer_component_id="960619"
	platform_fuzzer_install "${S}"/OWNERS \
			"${OUT}"/debugd_cups_uri_helper_utils_fuzzer \
			--comp "${fuzzer_component_id}"
}
