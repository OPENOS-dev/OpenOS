# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the BSD license.

EAPI="7"

CROS_WORKON_COMMIT="f539782e96b97c11975c4a38fb1542a5a6042805"
CROS_WORKON_TREE="2f6922f871991e9052697c0c1b53deaa802293fb"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_SUBTREE="chromeos-nvt-tcon-updater"

inherit cros-workon user

DESCRIPTION="Shell library for integrating the Novatek TCON Firmware updater"

LICENSE="BSD-Google"
SLOT="0/0"
KEYWORDS="*"

RDEPEND="
	chromeos-base/chromeos-init
	chromeos-base/common-assets
	chromeos-base/minijail
	sys-apps/novatek-tcon-fw-update-tool
"

pkg_preinst() {
	enewgroup fwupdate-drm_dp_aux-i2c
	enewuser fwupdate-drm_dp_aux-i2c
}

src_install() {
	insinto "/opt/google/tcon/policies"
	doins chromeos-nvt-tcon-updater/policies/"${ARCH}"/nvt-tcon-fw-updater.update.policy

	insinto "/opt/google/tcon/scripts"
	doins chromeos-nvt-tcon-updater/scripts/chromeos-nvt-tcon-firmware-update.sh
}
