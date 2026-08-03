# Copyright 2014 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "16839d1d0fcecb39e52d1bc3a65aa02d62c1ee6d" "b8033e453c7d9518619e90fb100d7d90d7b4026d" "1cd1013057a51ae459cfe5a0b07deaf4793e68e7" "4934b6b332f2a3db7a26bad9f888607a4f12b440" "21be3d94228981b109f53d043d8a9971ef38b54e" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "43eb4f30218ee6fc055f185786d914bccd668086" "d68677a0f92fd54fc25bab9a6d1a996b14f98257" "ec984bfe5341a0fbdf425858feba901268f174b0" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_USE_VCSID="1"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
# TODO(crbug.com/809389): Avoid directly including headers from other packages.
CROS_WORKON_SUBTREE="common-mk chromeos-config featured iioservice libec libsar metrics mojo_service_manager power_manager shill/dbus/client .gn"

PLATFORM_NATIVE_TEST="yes"
PLATFORM_SUBDIR="power_manager"

inherit tmpfiles cros-workon cros-unibuild platform cros-protobuf systemd udev user

DESCRIPTION="Power Manager for Chromium OS"
HOMEPAGE="http://dev.chromium.org/chromium-os/packages/power_manager"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="-als cellular +cras cros_embedded +display_backlight fuzzer -has_keyboard_backlight iioservice iioservice_proximity -keyboard_includes_side_buttons keyboard_convertible_no_side_buttons -legacy_power_button -powerd_manual_eventlog_add +powerknobs systemd +touchpad_wakeup -touchscreen_wakeup unibuild wilco qrtr"
REQUIRED_USE="
	?? ( keyboard_includes_side_buttons keyboard_convertible_no_side_buttons )
	unibuild
"

COMMON_DEPEND="
	chromeos-base/chromeos-config-tools:=
	chromeos-base/featured:=
	chromeos-base/libec:=
	chromeos-base/libiioservice_ipc:=
	chromeos-base/libsar:=
	>=chromeos-base/metrics-0.0.1-r3152:=
	chromeos-base/ml-client:=
	chromeos-base/mojo_service_manager:=
	chromeos-base/power_manager-client:=
	chromeos-base/shill-dbus-client:=
	chromeos-base/tpm_manager-client:=
	dev-cpp/abseil-cpp:=
	dev-libs/libnl:=
	dev-libs/re2:=
	cras? ( media-libs/libcras:= )
	sys-fs/udev:=
	virtual/libusb:=
	virtual/udev
	cellular? ( net-misc/modemmanager-next:= )
	sys-apps/dbus:="

RDEPEND="${COMMON_DEPEND}
	chromeos-base/libiioservice_ipc:=
	powerd_manual_eventlog_add? ( sys-apps/coreboot-utils )
	qrtr? ( net-libs/libqrtr:= )
"

DEPEND="${COMMON_DEPEND}
	chromeos-base/chromeos-ec-headers:=
	chromeos-base/system_api:=[fuzzer?]
	qrtr? ( sys-apps/upstart:= )
"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
"

pkg_setup() {
	# Create the 'power' user and group here in pkg_setup as src_install needs
	# them to change the ownership of power manager files.
	enewuser "power"
	enewgroup "power"
	# Ensure that this group exists so that power_manager can access
	# /dev/cros_ec.
	enewgroup "cros_ec-access"
	cros-workon_pkg_setup
}

src_install() {
	platform_src_install

	# Binaries for production
	fowners root:power /usr/bin/powerd_setuid_helper
	fperms 4750 /usr/bin/powerd_setuid_helper

	# Preferences
	insinto /usr/share/power_manager
	doins default_prefs/*
	use als && doins optional_prefs/has_ambient_light_sensor
	use cras && doins optional_prefs/use_cras
	use display_backlight || doins optional_prefs/external_display_only
	use has_keyboard_backlight && doins optional_prefs/has_keyboard_backlight
	use legacy_power_button && doins optional_prefs/legacy_power_button
	use powerd_manual_eventlog_add && doins optional_prefs/manual_eventlog_add

	# udev scripts and rules.
	exeinto "$(get_udevdir)"
	doexe udev/*.sh
	udev_dorules udev/*.rules

	if use powerknobs; then
		udev/gen_autosuspend_rules.py > "${T}"/98-autosuspend.rules || die
		udev_dorules "${T}"/98-autosuspend.rules
		udev_dorules udev/optional/98-powerknobs.rules
		dobin udev/optional/set_blkdev_pm
	fi
	if use keyboard_includes_side_buttons; then
		udev_dorules udev/optional/93-powerd-tags-keyboard-side-buttons.rules
	elif use keyboard_convertible_no_side_buttons; then
		udev_dorules udev/optional/93-powerd-tags-keyboard-convertible.rules
	fi

	if ! use touchpad_wakeup; then
		udev_dorules udev/optional/93-powerd-tags-no-touchpad-wakeup.rules
	else
		udev_dorules udev/optional/93-powerd-tags-unibuild-touchpad-wakeup.rules
	fi

	if use touchscreen_wakeup; then
		udev_dorules udev/optional/93-powerd-tags-touchscreen-wakeup.rules
	else
		udev_dorules udev/optional/93-powerd-tags-unibuild-touchscreen-wakeup.rules
	fi

	if use wilco; then
		udev_dorules udev/optional/93-powerd-wilco-ec-files.rules

		exeinto /usr/share/cros/init/optional
		doexe init/shared/optional/powerd-pre-start-wilco.sh
	fi

	# Init scripts
	if use systemd; then
		systemd_dounit init/systemd/*.service
		systemd_enable_service boot-services.target powerd.service
		systemd_enable_service system-services.target report-power-metrics.service
		dotmpfiles init/systemd/powerd_directories.conf
	else
		insinto /etc/init
		insopts -m0644
		doins init/upstart/*.conf
	fi

	# Install fuzz targets.
	local fuzzer
	for fuzzer in "${OUT}"/*_fuzzer; do
		local fuzzer_component_id="167191"
		platform_fuzzer_install "${S}"/OWNERS "${fuzzer}" \
			--comp "${fuzzer_component_id}"
	done
}
