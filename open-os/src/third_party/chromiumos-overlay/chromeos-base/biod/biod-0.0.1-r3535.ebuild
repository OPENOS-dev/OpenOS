# Copyright 2016 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="38272f61162fa7fa1b6dfa7a6afc1ac8d2f9650b"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "95d4df7f9212a6560d43b5e6eee5c905a64eefd8" "16839d1d0fcecb39e52d1bc3a65aa02d62c1ee6d" "b8033e453c7d9518619e90fb100d7d90d7b4026d" "4934b6b332f2a3db7a26bad9f888607a4f12b440" "6a9a79d4054ab44a049d1756dc7e3cce63bc2cf8" "b8c09b0737d26e92e8c1543f785a92a112de09cc" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_USE_VCSID="1"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk biod chromeos-config featured libec libhwsec libhwsec-foundation metrics .gn"

PLATFORM_SUBDIR="biod"

inherit cros-fuzzer cros-sanitizers cros-workon cros-unibuild platform \
	cros-protobuf tmpfiles udev user

DESCRIPTION="Biometrics Daemon for Chromium OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/biod/README.md"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="
	factory_branch
	fp_on_power_button
	fpmcu_firmware_bloonchipper
	fpmcu_firmware_buccaneer
	fpmcu_firmware_chobienia
	fpmcu_firmware_chudow
	fpmcu_firmware_dartmonkey
	fpmcu_firmware_gwendolin
	fpmcu_firmware_helipilot
	fpmcu_firmware_nami
	fpmcu_firmware_niedzica
	fpmcu_firmware_nocturne
	fpmcu_firmware_rosalia
	fpmcu_firmware_sanok
	fpmcu_firmware_srebrna
	fpmcu_firmware_stobnica
	fuzzer
	test
"
# We must depend on libusb because libec headers make use of libusb.
COMMON_DEPEND="
	chromeos-base/chromeos-config-tools:=
	chromeos-base/featured:=
	chromeos-base/libec:=
	chromeos-base/libhwsec:=[test?]
	chromeos-base/libhwsec-foundation:=
	>=chromeos-base/metrics-0.0.1-r3152:=
	chromeos-base/vboot_reference:=
	sys-apps/flashmap:=
	virtual/libusb:1=
"

# For biod_client_tool. The biod_proxy library will be built on all boards but
# biod_client_tool will be built only on boards with biod.
COMMON_DEPEND+="
	chromeos-base/biod_proxy:=
"

# The crosec-legacy-drv package is a pinned version of flashrom
# for production firmware updates.
RDEPEND="
	${COMMON_DEPEND}
	sys-apps/crosec-legacy-drv:=
	!factory_branch? ( virtual/chromeos-firmware-fpmcu )
	"

# Release branch firmware.
# The USE flags below come from USE_EXPAND variables.
# See third_party/chromiumos-overlay/profiles/base/make.defaults.
RDEPEND+="
	!factory_branch? (
		fpmcu_firmware_bloonchipper? (
			sys-firmware/chromeos-fpmcu-release-bloonchipper
		)
		fpmcu_firmware_buccaneer? ( sys-firmware/chromeos-fpmcu-release-buccaneer )
		fpmcu_firmware_chobienia? ( sys-firmware/chromeos-fpmcu-release-chobienia )
		fpmcu_firmware_chudow? ( sys-firmware/chromeos-fpmcu-release-chudow )
		fpmcu_firmware_dartmonkey? ( sys-firmware/chromeos-fpmcu-release-dartmonkey )
		fpmcu_firmware_gwendolin? ( sys-firmware/chromeos-fpmcu-release-gwendolin )
		fpmcu_firmware_helipilot? ( sys-firmware/chromeos-fpmcu-release-helipilot )
		fpmcu_firmware_nami? ( sys-firmware/chromeos-fpmcu-release-nami )
		fpmcu_firmware_niedzica? ( sys-firmware/chromeos-fpmcu-release-niedzica )
		fpmcu_firmware_nocturne? ( sys-firmware/chromeos-fpmcu-release-nocturne )
		fpmcu_firmware_rosalia? ( sys-firmware/chromeos-fpmcu-release-rosalia )
		fpmcu_firmware_sanok? ( sys-firmware/chromeos-fpmcu-release-sanok )
		fpmcu_firmware_srebrna? ( sys-firmware/chromeos-fpmcu-release-srebrna )
		fpmcu_firmware_stobnica? ( sys-firmware/chromeos-fpmcu-release-stobnica )
	)
"

DEPEND="
	${COMMON_DEPEND}
	chromeos-base/chromeos-ec-headers:=
	chromeos-base/power_manager-client:=
	chromeos-base/system_api:=[fuzzer?]
	dev-libs/openssl:=
"

BDEPEND="
	chromeos-base/minijail
"

pkg_setup() {
	enewuser biod
	enewgroup biod
	enewgroup fpdev
}

src_install() {
	platform_src_install

	udev_dorules udev/99-biod.rules

	dotmpfiles tmpfiles.d/*.conf

	# Set up cryptohome daemon mount store in daemon's mount
	# namespace.
	local daemon_store="/etc/daemon-store/biod"
	dodir "${daemon_store}"
	fperms 0700 "${daemon_store}"
	fowners biod:biod "${daemon_store}"

	local fuzzer_component_id="782045"
	platform_fuzzer_install "${S}/OWNERS" "${OUT}"/biod_storage_fuzzer --comp "${fuzzer_component_id}"

	platform_fuzzer_install "${S}/OWNERS" "${OUT}"/biod_crypto_validation_value_fuzzer --comp "${fuzzer_component_id}"
}
