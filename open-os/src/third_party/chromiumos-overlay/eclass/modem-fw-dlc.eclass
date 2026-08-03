# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# @ECLASS: modem-fw-dlc.eclass
# @MAINTAINER:
# cros-cellular-core@, andrewlassalle@chromium.org
# @BUGREPORTS:
# Please report bugs via
# https://issuetracker.google.com/issues/new?component=167157
# @VCSURL: https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/eclass/@ECLASS@
# @BLURB: helper eclass for building modem FW DLCs
# @DESCRIPTION:
# Common settings use by most modem FW DLCs.

if [[ -z "${_ECLASS_MODEM_FW_DLC}" ]]; then

# Multiple inclusion protection.
_ECLASS_MODEM_FW_DLC=1

inherit cros-cellular dlc estack

# @ECLASS-VARIABLE: MODEM_FW_DLC_FIRMWARE_VARIANT
# @INTERNAL
# @DEFAULT_UNSET
# @DESCRIPTION:
# A value to indicate which firmware-variants use this DLC. This value will be
# used in DLC_ATTRIBUTES.
# This value can be set to filter DLCs when installing them on the device.


# @ECLASS-VARIABLE: MODEM_FW_DLC_PREALLOC_SIZE_MB
# @DEFAULT_UNSET
# @DESCRIPTION:
# The DLC size in MiB.
# This value can be used to define the DLC preallocatoin size in MiB instead of
# directly using DLC_PREALLOC_BLOCKS.

# @ECLASS-VARIABLE: MODEM_FW_DLC_FM101_DEFAULT_SIZE_3FW
# @INTERNAL
# @DESCRIPTION:
# The default preallocation size for FM101 modem DLCs.
# This value should never increase, since there is no guarantee that the user
# will have enough space left to accommodate the increase in size.
# We reserve enough space to fit:
# uncompressed => 3 Main FWs = ~89MiB * 3 =  267MiB
# compressed => 3 Main FWs = ~64MiB * 3 =  192MiB
# Total = ~192MiB => 200MiB to be safe
export readonly MODEM_FW_DLC_FM101_DEFAULT_SIZE_3FW=200

# @ECLASS-VARIABLE: MODEM_FW_DLC_FM350_DEFAULT_SIZE_3FW
# @INTERNAL
# @DESCRIPTION:
# The default preallocation size for Fibocom FM350 modem DLCs.
# This value should never increase, since there is no guarantee that the user
# will have enough space left to accommodate the increase in size.
# We reserve enough space to fit:
# uncompressed => 3 Main FWs = ~77MiB * 3 =  231MiB
# compressed => 3 Main FWs = ~50MiB * 3 =  150MiB
# Total = ~150 MiB => 156MiB to be safe
export readonly MODEM_FW_DLC_FM350_DEFAULT_SIZE_3FW=156

# @ECLASS-VARIABLE: MODEM_FW_DLC_L850_DEFAULT_SIZE_3FW
# @INTERNAL
# @DESCRIPTION:
# The default preallocation size for Fibocom L850 modem DLCs.
# This value should never increase, since there is no guarantee that the user
# will have enough space left to accommodate the increase in size.
# We reserve enough space to fit:
# 3 Main FWs = ~11MiB * 3 (All L850 files are already compressed)
# 1 OEM FW = 125KiB
# 1 OEM carrier pack = 2MiB
# Total = ~36 MiB => 39MiB to be safe
export readonly MODEM_FW_DLC_L850_DEFAULT_SIZE_3FW=39

# @ECLASS-VARIABLE: MODEM_FW_DLC_EM060_DEFAULT_SIZE
# @INTERNAL
# @DESCRIPTION:
# The default preallocation size for Quectel EM060 modem DLCs.
# This value should never increase, since there is no guarantee that the user
# will have enough space left to accommodate the increase in size. As per
# Quectel's commitment, We reserve enough space to fit:
# 2 Main FWs = ~29MiB * 2
# 2 Carrier FWs = ~3MiB * 2
# 1 OEM FW = ~63MiB
# Total = ~127MiB uncompressed, ~93MiB compressed => 100MiB to be safe
export readonly MODEM_FW_DLC_EM060_DEFAULT_SIZE=100

# @ECLASS-VARIABLE: MODEM_FW_DLC_LCUK54_DEFAULT_SIZE
# @INTERNAL
# @DESCRIPTION:
# The default preallocation size for NetPrisma LCUK54 modem DLCs.
# This value should never increase, since there is no guarantee that the user
# will have enough space left to accommodate the increase in size.
# While AVL should assert a 100MiB maximum firmware size, let's set a
# temporary 2x allocation until we have confirmation from NetPrisma.
# TODO(b/350560043): Add math here when we have more details.
export readonly MODEM_FW_DLC_LCUK54_DEFAULT_SIZE=200

# @ECLASS-VARIABLE: MODEM_FW_DLC_RW101_DEFAULT_SIZE_3FW
# @INTERNAL
# @DESCRIPTION:
# The default preallocation size for RW101 modem DLCs.
# This value should never increase, since there is no guarantee that the user
# will have enough space left to accommodate the increase in size.
# We reserve enough space to fit:
# uncompressed => 3 Main FWs = ~89MiB * 3 =  267MiB
# compressed => 3 Main FWs = ~64MiB * 3 =  192MiB
# Total = ~192MiB => 200MiB to be safe
export readonly MODEM_FW_DLC_RW101_DEFAULT_SIZE_3FW=200

# @ECLASS-VARIABLE: MODEM_FW_DLC_RW135_DEFAULT_SIZE_3FW
# @INTERNAL
# @DESCRIPTION:
# The default preallocation size for RW135 modem DLCs.
# This value should never increase, since there is no guarantee that the user
# will have enough space left to accommodate the increase in size.
# We reserve enough space to fit:
# uncompressed => 3 Main FWs = ~89MiB * 3 =  267MiB
# compressed => 3 Main FWs = ~64MiB * 3 =  192MiB
# Total = ~192MiB => 200MiB to be safe
export readonly MODEM_FW_DLC_RW135_DEFAULT_SIZE_3FW=200

# @ECLASS-VARIABLE: MODEM_FW_DLC_RW350_DEFAULT_SIZE_3FW
# @INTERNAL
# @DESCRIPTION:
# The default preallocation size for Rolling RW350 modem DLCs.
# This value should never increase, since there is no guarantee that the user
# will have enough space left to accommodate the increase in size.
# We reserve enough space to fit:
# uncompressed => 3 Main FWs = ~77MiB * 3 =  231MiB
# compressed => 3 Main FWs = ~50MiB * 3 =  150MiB
# Total = ~150 MiB => 156MiB to be safe
export readonly MODEM_FW_DLC_RW350_DEFAULT_SIZE_3FW=156

# Installs the DLC during FSI.
DLC_FACTORY_INSTALL=true

# Preload on test images
DLC_PRELOAD=true

# Always update with the OS
DLC_CRITICAL_UPDATE=true

# Keep DLC on powerwash
DLC_POWERWASH_SAFE=true

# Trusted dm-verity digest through LoadPin.
DLC_LOADPIN_VERITY_DIGEST=true

# DLC will use logical volume. Needed for powerwash survival.
DLC_USE_LOGICAL_VOLUME=true

# Use the scaled infrastructure for fetching DLCs.
DLC_FORCE_OTA=true

# @FUNCTION: modem_fw_dlc_generate_patches
# @DESCRIPTION:
# Moves firmware files and manifest into a common working
# directory, and processes the files into patches for DLC.
modem_fw_dlc_generate_patches() {
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

# @FUNCTION: _modem_fw_dlc_validate_contents
# @DESCRIPTION:
# Ensures that contents of the files placed into DLC meet our expectations.
# Specifically, firmware for only a single modem should be present, and there
# should be a non-zero number of files present (i.e. the DLC shouldn't be empty)
_modem_fw_dlc_validate_contents() {
	local fw_dir="${1}"

	num_modem_dirs=$(find "${fw_dir}" -maxdepth 1 -mindepth 1 -type d | wc -l)
	[[ ${num_modem_dirs} -eq 1 ]] || die "${num_modem_dirs} modem FWs found in DLC"
}

# @FUNCTION: modem_fw_dlc_src_install
# @DESCRIPTION:
# Convenience function to create a Modem FW DLC. The function does some basic
# validation, packages the modem FW files in the correct directories and
# creates the DLC.
modem_fw_dlc_src_install() {
	# Only set DLC_PREALLOC_BLOCKS if MODEM_FW_DLC_PREALLOC_SIZE_MB was set in the ebuild.
	# This allows for the use of either flag in the ebuild.
	if [[ -n "${MODEM_FW_DLC_PREALLOC_SIZE_MB}" ]]; then
		# 256 blocks = 1MiB/4Kib = 1024*1024/4096.
		DLC_PREALLOC_BLOCKS="$((MODEM_FW_DLC_PREALLOC_SIZE_MB * 256))"
	fi

	# Set the `modem` and model name in the DLC attribute
	DLC_ATTRIBUTES="${DLC_ATTRIBUTES} modem"
	if [[ -n "${MODEM_FW_DLC_FIRMWARE_VARIANT}" ]]; then
		DLC_ATTRIBUTES="${DLC_ATTRIBUTES} ${MODEM_FW_DLC_FIRMWARE_VARIANT}"
	fi

	# If patches have been made, use them. Else, pull in firmware files directly
	local patch_output_path="$(_cellular_get_local_patchdir)"
	if [ -d "${patch_output_path}" ]; then
		_modem_fw_dlc_validate_contents "${patch_output_path}"

		insinto "$(dlc_add_path "/")"
		doins -r "${patch_output_path}"/*
	else
		local fw_path="${WORKDIR}/modem_fw"
		cellular_gather_firmware_to_directory "${fw_path}"
		_modem_fw_dlc_validate_contents "${fw_path}"

		insinto "$(dlc_add_path "/")"
		doins -r "${fw_path}"/*
	fi

	dlc_src_install
}
fi
