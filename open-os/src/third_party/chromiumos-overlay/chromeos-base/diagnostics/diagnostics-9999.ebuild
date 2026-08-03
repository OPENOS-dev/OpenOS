# Copyright 2018 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_DESTDIR="${S}/platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_SUBTREE="common-mk diagnostics .gn"

PLATFORM_SUBDIR="diagnostics"

inherit cros-sanitizers cros-workon cros-unibuild platform cros-protobuf udev user

DESCRIPTION="Device telemetry and diagnostics for Chrome OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/diagnostics"

LICENSE="BSD-Google"
KEYWORDS="~*"
IUSE="fuzzer mesa_reven diagnostics dlc"

COMMON_DEPEND="
	acct-user/cros_healthd
	acct-group/cros_healthd
	chromeos-base/bootstat:=
	chromeos-base/chromeos-config-tools:=
	chromeos-base/libec:=
	chromeos-base/metrics:=
	chromeos-base/minijail:=
	chromeos-base/missive:=
	chromeos-base/mojo_service_manager:=
	chromeos-base/spaced:=
	chromeos-base/vboot_reference:=
	dev-libs/glib:=
	dev-libs/libevdev:=
	dev-libs/openssl:=
	dev-libs/re2:=
	net-misc/curl:=
	virtual/libudev:=
	sys-apps/pciutils:=
	virtual/libusb:1=
	virtual/opengles:=
	sys-apps/fwupd:=
	sys-apps/rootdev:=
	sys-apps/util-linux:=
	x11-libs/libdrm:=
"

DEPEND="
	${COMMON_DEPEND}
	chromeos-base/attestation-client:=
	chromeos-base/chromeos-ec-headers:=
	chromeos-base/concierge-client:=
	chromeos-base/cros-camera-libs:=
	chromeos-base/debugd-client:=
	chromeos-base/dlcservice-client:=
	chromeos-base/libiioservice_ipc:=
	chromeos-base/power_manager-client:=
	chromeos-base/session_manager-client:=
	chromeos-base/system_api:=[fuzzer?]
	chromeos-base/tpm_manager-client:=
	media-libs/libcras:=
	net-analyzer/ndt7-client-cc:=
	x11-drivers/opengles-headers:=
"

# TODO(b/271544868): Remove net-wireless/iw once we find alternatives.
RDEPEND="
	${COMMON_DEPEND}
	chromeos-base/crash-reporter
	chromeos-base/debugd
	chromeos-base/iioservice
	dev-util/stressapptest
	net-wireless/iw
	dlc? (
		chromeos-base/fio-dlc
	)
"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
	chromeos-base/minijail
"

pkg_preinst() {
	enewgroup cros_ec-access
	enewgroup fpdev
	enewuser healthd_ec
	enewgroup healthd_ec
	enewuser healthd_fp
	enewgroup healthd_fp
	enewuser healthd_evdev
	enewgroup healthd_evdev
	enewuser healthd_psr
	enewgroup healthd_psr
	enewgroup mei-access
}

src_install() {
	platform_src_install

	# Install udev rules.
	udev_dorules udev/99-mei_driver_files.rules

	# Install fuzzers.
	local fuzzer_component_id="982097"
	platform_fuzzer_install "${S}"/OWNERS "${OUT}"/fetch_system_info_fuzzer \
		--comp "${fuzzer_component_id}"
	platform_fuzzer_install "${S}"/OWNERS "${OUT}"/crash_events_uploads_log_parser_fuzzer \
		--comp "${fuzzer_component_id}"

	# TODO(b/299052079): Remove mojom file after completing hotline migration.
	insinto /usr/include/cros_healthd-client/diagnostics/mojom/public
	doins "${S}"/mojom/public/cros_healthd_probe.mojom
	doins "${OUT}"/gen/include/diagnostics/mojom/public/cros_healthd_probe.mojom-module
}
