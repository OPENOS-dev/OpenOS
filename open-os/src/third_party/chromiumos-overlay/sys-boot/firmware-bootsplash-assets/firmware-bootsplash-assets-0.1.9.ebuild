# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

# This ebuild reads the firmware bootsplash assets from public mirror and installs
# them in coreboot-private path.

# Instructions for updating the firmware bootsplash assets referenced below:
# 1. Create a versioned bootsplash assets archive file where version format matches
#    the ebuild version (eg. 0.0.n). Pick current version + 1 while updating.
#    * mkdir firmware-bootsplash-assets-${version}
#    * cp <all the files> firmware-bootsplash-assets-${version}
#    * tar -Ipixz -cf firmware-bootsplash-assets-${version}.tar.xz firmware-bootsplash-assets-${version}/
#
# 2.  Upload the archive file to chromeos-localmirror Google Storage (GS) bucket
#    * Refer: https://chromium.googlesource.com/chromiumos/docs/+/HEAD/archive_mirrors.md#Getting-files-onto-localmirror
#    * Note: Do not delete or overwrite the existing archives. This might break some firmware branches.
#
# 3. Bump/Uprev the ebuild version such that it matches the archive file version
# 4. In a cros_sdk chroot, generate the updated manifest
#    * Refer: https://chromium.googlesource.com/chromiumos/docs/+/HEAD/archive_mirrors.md#Updating-Manifest-files
#
# 5. Commit the CL with ebuild & Manifest files
SRC_URI="gs://chromeos-localmirror/distfiles/${P}.tar.xz"

DESCRIPTION="Firmware bootsplash assets applicable to multiple boards"
SLOT="0"
KEYWORDS="*"

LICENSE="BSD-Google"

S="${WORKDIR}"

# Ensure meeting the format while creating `firmware-bootsplash-assets-${version}` directory
#
# firmware-bootsplash-assets-${version}.tar.xz
# ├── chromebook_plus_100_percent.bmp
# ├── chromebook_plus_200_percent.bmp
# ├── chromeos_100_percent.bmp
# ├── chromeos_200_percent.bmp
# ├── footer
# │   ├── footer_100_percent.bmp
# │   └── footer_200_percent.bmp
# ├── low_battery_100_percent.bmp
# ├── low_battery_200_percent.bmp
# ├── oem_main_logo
# ├──────[ID]
# │        ├── logo_100_percent.bmp
# │        └── logo_200_percent.bmp
# │        └── logo_300_percent.bmp (optional)
#

src_install() {
	# Pull in Boot Splash Screens
	insinto /firmware/coreboot-private/bootsplash_assets

	# Install direct assets
	doins "${P}"/chromebook_plus_100_percent.bmp
	doins "${P}"/chromebook_plus_200_percent.bmp
	doins "${P}"/chromeos_100_percent.bmp
	doins "${P}"/chromeos_200_percent.bmp
	doins "${P}"/low_battery_100_percent.bmp
	doins "${P}"/low_battery_200_percent.bmp
	doins "${P}"/battery_low_100_percent.bmp
	doins "${P}"/battery_low_150_percent.bmp
	doins "${P}"/battery_low_200_percent.bmp
	doins "${P}"/battery_low_300_percent.bmp
	doins "${P}"/battery_charging_100_percent.bmp
	doins "${P}"/battery_charging_150_percent.bmp
	doins "${P}"/battery_charging_200_percent.bmp
	doins "${P}"/battery_charging_300_percent.bmp

	# Install assets from subdirectories

	# For bootsplash footer
	insinto /firmware/coreboot-private/bootsplash_assets/footer
	doins "${P}"/footer/footer_100_percent.bmp
	doins "${P}"/footer/footer_200_percent.bmp

	# For bootsplash center (main)
	# To ensure hardware-to-vendor abstraction, assets are organized
	# by a 'Secure ID'.
	#
	# Deployment Pattern:
	# insinto /firmware/coreboot-private/bootsplash_assets/oem_main_logo/[ID]
	# doins "${P}"/oem_main_logo/[ID]/logo_100_percent.bmp
	# doins "${P}"/oem_main_logo/[ID]/logo_200_percent.bmp
	# doins "${P}"/oem_main_logo/[ID]/logo_300_percent.bmp (optional)
	#
	# NOTE: [ID] is the first 8 characters (32-bit) of the Secure ID created
	# based on the OEM name.
	# Please consult README.txt for the secure_id-to-OEM manifest.

# ---------------- Add new OEM bootsplash logo path below ----------------------
	local -a oems=(
		"d3556d78"
		"f9a1262d"
		"137b7852"
		"90886c12"
		"76f62137"
		"d1490956"
	)

# ------------------------- Don't modify code below ----------------------------
	for oem in "${oems[@]}"; do
		insinto "/firmware/coreboot-private/bootsplash_assets/oem_main_logo/${oem}"
		doins "${P}/oem_main_logo/${oem}/logo_100_percent.bmp"
		doins "${P}/oem_main_logo/${oem}/logo_200_percent.bmp"

		# Optional: Only check and install 300% logo if it exists in the source
		if [[ -f "${P}/oem_main_logo/${oem}/logo_300_percent.bmp" ]]; then
			doins "${P}/oem_main_logo/${oem}/logo_300_percent.bmp"
		fi

		# Optional: Only check and install 150% logo if it exists in the source
		if [[ -f "${P}/oem_main_logo/${oem}/logo_150_percent.bmp" ]]; then
			doins "${P}/oem_main_logo/${oem}/logo_150_percent.bmp"
		fi
	done
}
