# Copyright 2011 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Chrome OS Firmware virtual package"
HOMEPAGE="http://src.chromium.org"

LICENSE="metapackage"
SLOT="0"
KEYWORDS="*"

IUSE="bootimage cros_ec zephyr_ec cros_ish zephyr_ish"

RDEPEND="!bootimage? ( chromeos-base/chromeos-firmware-null )
	bootimage? ( sys-boot/chromeos-bootimage )
	zephyr_ec? ( chromeos-base/chromeos-zephyr )
	zephyr_ish? ( chromeos-base/chromeos-zephyr-ish )
	cros_ish? ( chromeos-base/chromeos-ish )"

DEPEND=""
