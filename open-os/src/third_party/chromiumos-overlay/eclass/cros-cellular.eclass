# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

inherit estack

# @ECLASS: cros-cellular.eclass
# @MAINTAINER:
# ejcaruso@chromium.org
# @BUGREPORTS:
# Please report bugs via https://crbug.com/new
# @VCSURL: https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/eclass/@ECLASS@
# @BLURB: helper eclass for building cellular helpers for modemfwd
# @DESCRIPTION:
# modemfwd expects a directory containing helpers and a manifest detailing
# which ones are included. This ebuild makes it easier to put everything in
# the right place.

# The modemfwd helper directory.
_cellular_get_helperdir() {
	echo /opt/google/modemfwd-helpers
}

# The modemfwd firmware directory (until CUS handles the firmware package).
_cellular_get_firmwaredir() {
	echo /opt/google/modemfwd-firmware
}

# @FUNCTION: cellular_dohelper
# @DESCRIPTION:
# Installs a helper binary to the modemfwd helper directory
# using doexe.
cellular_dohelper() {
	(
		exeinto "$(_cellular_get_helperdir)"
		doexe "${@}"
	)
}

# @FUNCTION: cellular_newhelper
# @DESCRIPTION:
# Installs a helper binary to the modemfwd helper directory
# using newexe.
cellular_newhelper() {
	(
		exeinto "$(_cellular_get_helperdir)"
		newexe "${@}"
	)
}

# @FUNCTION: cellular_domanifest
# @DESCRIPTION:
# Installs the compiled proto manifest into the modemfwd helper
# directory using doins.
cellular_domanifest() {
	(
		insinto "$(_cellular_get_helperdir)"
		doins "${@}"
	)
}

# @FUNCTION: cellular_dofirmware
# @DESCRIPTION:
# Installs the firmware blob(s) into the firmware directory.
cellular_dofirmware() {
	(
		insinto "$(_cellular_get_firmwaredir)"
		doins "${@}"
	)
}

# @FUNCTION: cellular_gather_firmware_to_directory
# @DESCRIPTION:
# Gathers the firmware files from the gs packages into
# the directory passed as an argument.
cellular_gather_firmware_to_directory() {
	local dest_dir="${1}"
	local modem_name

	# Gather firmware files and manifest
	for modem_name in "em060" "fm101" "fm350" "l850" "lcuk54" "nl668" "rw101" "rw135" "rw350"; do
		eshopts_push -s nullglob
		local modem_dirs=("${WORKDIR}"/cellular-firmware-{fibocom,netprisma,quectel,rolling}-"${modem_name}"-*)
		eshopts_pop

		if [[ "${#modem_dirs[@]}" -ne 0 ]]; then
			# Create subdirectory in the workspace if it doesn't exist already.
			mkdir -p "${dest_dir}/${modem_name}" || die

			local f
			for f in "${modem_dirs[@]}"; do
				cp -r "${f}"/* "${dest_dir}/${modem_name}" \
					|| die "Failed to cp modem firmware into squashfs"
			done
		fi
	done
}

# @FUNCTION: _cellular_get_local_patchdir
# @DESCRIPTION:
# Returns the local working directory that will be used as a
# destination for patch processing, and as input to squashfs
# creation.
_cellular_get_local_patchdir() {
	echo "${WORKDIR}/patch_dir"
}

# @FUNCTION: cellular_generate_firmware_patches
# @DESCRIPTION:
# Moves firmware files and manifest into a common working
# directory, and processes the files into patches.
cellular_generate_firmware_patches() {
	local patch_input_path="${WORKDIR}/patch_dir_input"
	local input_manifest_path="${FILESDIR}/patch_manifest.textproto"
	local patch_output_path="$(_cellular_get_local_patchdir)"
	local optional_manifest_flag=""

	# Move all firmware files into working directory
	cellular_gather_firmware_to_directory "${patch_input_path}"

	# Extract any existing firmware payloads (L850) that use xz
	find "${patch_input_path}" -name "*.xz"  -exec unxz {} \;

	# Use pre-computed patch manifest if available
	if [ -f "${input_manifest_path}" ]; then
		optional_manifest_flag="--input_manifest=${input_manifest_path}"
	fi
	mkdir -p "${patch_output_path}" || die
	patchmaker --encode --src_path="${patch_input_path}" \
		--dest_path="${patch_output_path}" \
		"${optional_manifest_flag}" \
		|| die "Failed to generate firmware patches"
}

# @FUNCTION: cellular_create_squashfs_bundle
# @DESCRIPTION:
# Moves firmware files and manifest into a common working
# directory, creates, and installs a squashfs.
cellular_create_squashfs_bundle() {
	# Directory we'll generate the squashfs from
	local fw_path="${WORKDIR}/modem_fw"
	mkdir -p "${fw_path}" || die

	local patch_output_path="$(_cellular_get_local_patchdir)"

	# If patches have been made, use them. Else, pull in firmware files directly
	if [ -d "${patch_output_path}" ]; then
		cp -r "${patch_output_path}"/* "${fw_path}" \
			|| die "Failed to cp modem firmware into squashfs"
	else
		cellular_gather_firmware_to_directory "${fw_path}"
	fi

	cp "${FILESDIR}/firmware_manifest.textproto" "${fw_path}" \
		|| die "Failed to cp firmware_manifest into squashfs"

	# Make sure permissions are correct.
	find "${fw_path}" -type f -exec chmod 644 {} + || die
	find "${fw_path}" -type d -exec chmod 755 {} + || die

	# Create and install the squashfs file.
	mksquashfs "${fw_path}" "${WORKDIR}/modem_fw.squash" \
		-all-root -noappend -no-recovery -no-exports -exit-on-error \
		-comp zstd -Xcompression-level 22 -b 1M -4k-align -root-mode 0755 \
		-no-progress \
		|| die "Failed to create modem FW squashfs"

	insinto "$(_cellular_get_firmwaredir)"
	doins "${WORKDIR}/modem_fw.squash"
}
