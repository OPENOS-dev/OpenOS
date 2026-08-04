# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

DESCRIPTION="Coreboot files for Skywalker baseboard"
KEYWORDS="~*"
LICENSE="BSD-Google"

# This ebuild only cares about its own FILESDIR and ebuild file, so it tracks
# the canonical empty project.
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"

# No need to inherit coreboot-private-files as there are no files in ${FILESDIR}
inherit cros-workon

DEPEND="
	chromeos-base/chromeos-config:=
	=sys-firmware/realtek-rts5453-GOOG0B00-firmware-16.14.4
	=sys-firmware/realtek-rts5453-GOOG0B01-firmware-16.14.4
	=sys-firmware/realtek-rts5453-GOOG0B02-firmware-16.14.4
	=sys-firmware/realtek-rts5453-GOOG0B03-firmware-16.14.4
	=sys-firmware/realtek-rts5453-GOOG0B04-firmware-16.14.4
	=sys-firmware/realtek-rts5453-GOOG0C00-firmware-16.14.4
	=sys-firmware/realtek-rts5453-GOOG0D00-firmware-16.14.4
	=sys-firmware/realtek-rts5453-GOOG0E00-firmware-16.14.4
	=sys-firmware/realtek-rts5453vb-GOOG0U00-firmware-16.14.4
	=sys-firmware/realtek-rts5453vb-GOOG0W00-firmware-16.14.4
	=sys-firmware/realtek-rts5453vb-GOOG0X00-firmware-16.14.4
	=sys-firmware/ti-tps6699x-GOOG0K00-firmware-19.32.28
"
RDEPEND="${DEPEND}"

src_install() {
	(cros_config_host "get-firmware-build-combinations" depthcharge \
		|| die "Failed to run cros_config_host") |
	while read -r name && read -r depthcharge; do
		local fw_names=()
		case "${depthcharge}" in
			anakin|baze|padme|tarkin)
				fw_names+=(
					"rts5453vb_GOOG0U00"
					"rts5453_GOOG0B00"
					"tps6699x_GOOG0K00"
				)
				;;
			dooku)
				fw_names+=("rts5453vb_GOOG0U00")
				;;
			grogu)
				fw_names+=("rts5453_GOOG0E00")
				;;
			jaina)
				fw_names+=("rts5453vb_GOOG0X00")
				;;
			obiwan)
				fw_names+=(
					"rts5453_GOOG0D00"
					"rts5453_GOOG0B02"
					"rts5453vb_GOOG0U00"
					"rts5453vb_GOOG0W00"
				)
				;;
			r2d2)
				fw_names+=("rts5453_GOOG0B04")
				;;
			sheev)
				fw_names+=("rts5453_GOOG0E00")
				;;
			skywalker)
				fw_names+=("rts5453_GOOG0C00" "rts5453_GOOG0B00")
				;;
			vader)
				fw_names+=("rts5453_GOOG0B00" "rts5453vb_GOOG0U00")
				;;
			yoda)
				fw_names+=(
				"rts5453_GOOG0D00"
				"rts5453_GOOG0B01"
				"rts5453vb_GOOG0U00"
				"rts5453vb_GOOG0W00"
				)
				;;
		esac

		# Silently ignore models that don't need PDC Firmware.
		[[ "${#fw_names[@]}" -eq 0 ]] && continue
		insinto "/firmware/cbfs-rw-compress-override/${name}"
		for fw_name in "${fw_names[@]}"; do
			local fw_dir

			case "${fw_name}" in
				rts5453*)
					fw_dir="${SYSROOT}/firmware/rts5453"
					;;
				tps6699x*)
					fw_dir="${SYSROOT}/firmware/tps6699x"
					;;
				*)
					die "Unknown firmware type: ${fw_name}"
					;;
			esac

			local fw_bin="${fw_dir}/${fw_name}.bin"
			local fw_hash="${fw_dir}/${fw_name}.hash"
			doins "${fw_bin}"
			doins "${fw_hash}"
		done
	done
}
