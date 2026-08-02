# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2
#
# Shellcheck doesn't understand our CONFIG_FRAGMENTS expansion.
# shellcheck disable=SC2034

# @ECLASS: coreboot-private-files-amd.eclass
# @MAINTAINER:
# The ChromiumOS Authors <chromium-os-dev@chromium.org>
# @BUGREPORTS:
# Please report bugs via
# https://issuetracker.google.com/issues/new?component=169878
# @VCSURL: https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/eclass/@ECLASS@
# @BLURB: Helper eclass for copying AMD prebuilts from an internal overlay.
# @DESCRIPTION:
# Common ebuild used during platform bringup to facilitate the copying of
# prebuilt blobs from an internal overlay into the staging directory.

# The inherit guard ensures that an eclass can be inherited multiple times, with
# its functions and variables being defined only once.
if [[ -z "${_ECLASS_COREBOOT_PF_AMD}" ]]; then
_ECLASS_COREBOOT_PF_AMD=1

# Check for EAPI 7+.
case "${EAPI:-0}" in
0|1|2|3|4|5|6) die "Unsupported EAPI=${EAPI:-0} (too old) for ${ECLASS}" ;;
7) ;;
esac

# @ECLASS-VARIABLE: COREBOOT_PF_AMD_SOC_NAME
# @PRE_INHERIT
# @DESCRIPTION:
# SOC name for this platform.  This variable is used to specify the path to copy
# the appropriate SOC blobs to the corresponding build directory.
[[ -n "${COREBOOT_PF_AMD_SOC_NAME-}" ]] || die "COREBOOT_PF_AMD_SOC_NAME (${COREBOOT_PF_AMD_SOC_NAME}) must designate the SOC name for this platform"

# @ECLASS-VARIABLE: COREBOOT_PF_AMD_CRB_NAME
# @PRE_INHERIT
# @DESCRIPTION:
# CRB name for this platform.  This variable is used to specify the path to copy
# platform blobs into the corresponding working directories for the CRB.
[[ -n "${COREBOOT_PF_AMD_CRB_NAME-}" ]] || die "COREBOOT_PF_AMD_CRB_NAME (${COREBOOT_PF_AMD_CRB_NAME}) must designate the CRB name associated with this platform"

# @ECLASS-VARIABLE: COREBOOT_PF_AMD_PROG_CODENAME
# @PRE_INHERIT
# @DESCRIPTION:
# Codename for this platform.  This variable is used to specify the path to copy
# platform blobs into the corresponding working directories for the mainboard.
[[ -n "${COREBOOT_PF_AMD_PROG_CODENAME-}" ]] || die "COREBOOT_PF_AMD_PROG_CODENAME (${COREBOOT_PF_AMD_PROG_CODENAME}) must designate the codename name associated with this platform"

# @FUNCTION: coreboot-private-files-amd_src_install
# @DESCRIPTION:
# Copies prebuilts from a private overlay into a staging directory.
coreboot-private-files-amd_src_install() {
	local build_dir="/firmware/coreboot-private/3rdparty/"
	local src="${FILESDIR}/3rdparty/amd_blobs/${COREBOOT_PF_AMD_SOC_NAME}"
	local dest="${build_dir}/amd_blobs/${COREBOOT_PF_AMD_SOC_NAME}"
	local prog_dest="${build_dir}/blobs/mainboard/google/${COREBOOT_PF_AMD_PROG_CODENAME}"
	local crb_src="${FILESDIR}/3rdparty/blobs/mainboard/amd/${COREBOOT_PF_AMD_CRB_NAME}"
	local crb_dest="${build_dir}/blobs/mainboard/amd/${COREBOOT_PF_AMD_CRB_NAME}"

	# Pull in VBIOS binary. Exclude release notes.
	insinto "${dest}"
	doins "${src}"/*.bin

	# Pull in amdfw.cfg and other binary blobs. Exclude release notes.
	insinto "${dest}/psp"
	doins "${src}/psp/amdfw.cfg"
	doins "${src}/psp"/*.{sbin,csbin,stkn,tkn,bin}

	# Pull in APCB.
	insinto "${prog_dest}"
	doins "${src}/psp"/APCB_*.bin

	# Pull in EC and APCB for CRB.
	insinto "${crb_dest}"
	doins "${crb_src}"/*.bin
}

fi  # _ECLASS_COREBOOT_PF_AMD

EXPORT_FUNCTIONS src_install
