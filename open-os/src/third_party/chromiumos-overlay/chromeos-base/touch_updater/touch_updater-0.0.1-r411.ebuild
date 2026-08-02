# Copyright 2013 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT=("b7fdd0bbfdb1203f0a4096355bb2522039a68186" "e1c7b519e0b62fd5e2a5577aa4a86828e98562c1")
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "a18d91b4124226997ac768d3db69d32413d93345" "bc44483ff8607ad5a0507c6057d018160ebad5b0" "860e1fbf234535850b83dba215e56d18b086bb1d" "8a0c7ad61340f54f4d6b2a3c97816b5dc9907b39")
CROS_WORKON_LOCALNAME=("platform2" "platform/touch_updater")
CROS_WORKON_PROJECT=("chromiumos/platform2" "chromiumos/platform/touch_updater")
CROS_WORKON_SUBTREE=("common-mk .gn" "policies scripts BUILD.gn install_touch_updater_seccomp.gni")
CROS_WORKON_DESTDIR=("${S}/platform2" "${S}/platform2/touch_updater")

PLATFORM_SUBDIR="touch_updater"

inherit cros-workon platform user

DESCRIPTION="Touch firmware and config updater"
HOMEPAGE="https://www.chromium.org/chromium-os"
SRC_URI=""

LICENSE="BSD-Google"
SLOT="0/0"
KEYWORDS="*"
IUSE="
	input_devices_synaptics
	input_devices_wacom
	input_devices_etphidiap
	input_devices_st_touchscreen
	input_devices_weida
	input_devices_goodix
	input_devices_sis
	input_devices_pixart
	input_devices_g2touch
	input_devices_cirque
	input_devices_elan_i2chid
	input_devices_melfas
	input_devices_emright
	input_devices_eps2pstiap
	input_devices_zinitix
	input_devices_himax
	input_devices_nvt_ts
	input_devices_ilitek_tddi
	input_devices_ilitek_its
	input_devices_paradetech
	input_devices_focaltech
"

# Third party firmware updaters usually belong in sys-apps/.  If you just
# checked in a new one to chromeos-base/, please move it to sys-apps/ before
# adding it as a dependency here.
RDEPEND="
	chromeos-base/chromeos-touch-common
	input_devices_synaptics? ( chromeos-base/rmi4utils )
	input_devices_wacom? ( chromeos-base/wacom_fw_flash )
	input_devices_etphidiap? ( chromeos-base/chromeos-touch-etphidiap )
	input_devices_st_touchscreen? ( chromeos-base/chromeos-touch-stupdate )
	input_devices_weida? ( chromeos-base/weida_wdt_util )
	input_devices_goodix? ( chromeos-base/gdix_hid_firmware_update )
	input_devices_sis? ( chromeos-base/sisConsoletool )
	input_devices_pixart? ( chromeos-base/pixart_tpfwup )
	input_devices_g2touch? ( chromeos-base/g2update_tool )
	input_devices_cirque? ( sys-apps/cirque_fw_update )
	input_devices_elan_i2chid? ( chromeos-base/elan_i2chid_tools )
	input_devices_melfas? ( chromeos-base/mfs-console-tool )
	input_devices_emright? ( chromeos-base/emright_fw_updater )
	input_devices_eps2pstiap? ( chromeos-base/epstps2iap )
	input_devices_zinitix? ( chromeos-base/zinitix_fw_updater )
	input_devices_himax? ( chromeos-base/hx_hid_util )
	input_devices_nvt_ts? ( chromeos-base/chromeos-nvt-touch-updater )
	input_devices_ilitek_tddi? ( chromeos-base/ilitek_tddi_tool )
	input_devices_ilitek_its? ( chromeos-base/ilitek_ld_tool )
	input_devices_paradetech? ( sys-apps/paradetech-updater )
	input_devices_focaltech? ( sys-apps/ftphid_ezupg_ap )
"

BDEPEND="
	chromeos-base/minijail
"

pkg_preinst() {
	if use input_devices_cirque || use input_devices_elan_i2chid || use input_devices_melfas || use input_devices_emright || use input_devices_zinitix || use input_devices_nvt_ts || use input_devices_himax || use input_devices_ilitek_tddi || use input_devices_ilitek_its || use input_devices_paradetech || use input_devices_focaltech; then
		enewgroup fwupdate-hidraw
		enewuser fwupdate-hidraw
	fi
	if use input_devices_sis; then
		enewgroup sisfwupdate
		enewuser sisfwupdate
	fi
	if use input_devices_pixart; then
		enewgroup pixfwupdate
		enewuser pixfwupdate
	fi
	if use input_devices_g2touch; then
		enewgroup g2touch
		enewuser g2touch
	fi
	if use input_devices_goodix; then
		enewgroup goodixfwupdate
		enewuser goodixfwupdate
	fi
	if use input_devices_eps2pstiap; then
		enewgroup fwupdate-serio
		enewuser fwupdate-serio
	fi
	if use input_devices_himax; then
		enewgroup fwupdate-i2c
		enewuser fwupdate-i2c
	fi
}
