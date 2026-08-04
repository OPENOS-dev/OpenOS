# Copyright 2025 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

# This ebuild only cares about its own FILESDIR and ebuild file, so it tracks
# the canonical empty project.
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"

# coreboot-private-files is needed for coreboot-private-files_src_install
inherit coreboot-private-files cros-workon

DESCRIPTION="coreboot files for corsola."
KEYWORDS="~*"

RDEPEND="
	chromeos-base/chromeos-config
	sys-firmware/analogix-anx3447-firmware
"
DEPEND="${RDEPEND}"

# This package requires that no license information to be displayed (not even
# Google-TOS). Do not use this license without a contractual reason.
LICENSE="Proprietary-Binary"

ANX3447_BOARDS=(
	kingler
	kyogre
	steelix
	voltorb
	ponyta
	chinchou
	squirtle
)

src_install() {
	# Install anx3447 firmware for Kingler boards and empty firmware for
	# others.
	local anx3447_bin="${SYSROOT}/firmware/anx3447/anx3447_ocm.bin"
	local anx3447_hash="${anx3447_bin/%.bin/.hash}"
	local empty_bin="$(basename "${anx3447_bin}")"
	local empty_hash="${empty_bin/%.bin/.hash}"

	touch "${empty_bin}"
	echo -n -e "\\x0\\x0" > "${empty_hash}"

	local fields="zephyr-ec"
	local cmd="get-firmware-build-combinations"
	(cros_config_host "${cmd}" "${fields}" || die) |
	while read -r name; do
		read -r zephyr_ec
		local firmware_bin="${empty_bin}"
		local firmware_hash="${empty_hash}"
		for board in "${ANX3447_BOARDS[@]}"; do
			if [[ "${zephyr_ec}" == "${board}" ]]; then
				firmware_bin="${anx3447_bin}"
				firmware_hash="${anx3447_hash}"
				break
			fi
		done

		insinto "/firmware/cbfs-rw-compress/${name}"
		doins "${firmware_bin}"

		insinto "/firmware/cbfs-rw-raw/${name}"
		doins "${firmware_hash}"
	done
}
