# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

if [[ -z "${_ECLASS_COREBOOT_SDK}" ]]; then
_ECLASS_COREBOOT_SDK=1

# Check for EAPI 7+.
case "${EAPI:-0}" in
[0123456]) die "unsupported EAPI (${EAPI}) in eclass (${ECLASS})" ;;
esac

# @ECLASS-VARIABLE: COREBOOT_SDK_PREFIX
# @DESCRIPTION:
#   Path where the coreboot SDK can be found.
COREBOOT_SDK_PREFIX=/opt/coreboot-sdk

# @ECLASS-VARIABLE: COREBOOT_SDK_PREFIX_arm
# @DESCRIPTION:
#   Prefix of coreboot SDK binaries for 32-bit arm.
COREBOOT_SDK_PREFIX_arm=${COREBOOT_SDK_PREFIX}/bin/arm-eabi-

# @ECLASS-VARIABLE: COREBOOT_SDK_PREFIX_arm64
# @DESCRIPTION:
#   Prefix of coreboot SDK binaries for 64-bit arm.
COREBOOT_SDK_PREFIX_arm64=${COREBOOT_SDK_PREFIX}/bin/aarch64-elf-

# @ECLASS-VARIABLE: COREBOOT_SDK_PREFIX_nds32
# @DESCRIPTION:
#   Prefix of coreboot SDK binaries for NDS32.
COREBOOT_SDK_PREFIX_nds32=${COREBOOT_SDK_PREFIX}/bin/nds32le-elf-

# @ECLASS-VARIABLE: COREBOOT_SDK_PREFIX_riscv
# @DESCRIPTION:
#   Prefix of coreboot SDK binaries for RISC-V.
COREBOOT_SDK_PREFIX_riscv=${COREBOOT_SDK_PREFIX}/bin/riscv64-elf-

# @ECLASS-VARIABLE: COREBOOT_SDK_PREFIX_x86_32
# @DESCRIPTION:
#   Prefix of coreboot SDK binaries for 32-bit x86.
COREBOOT_SDK_PREFIX_x86_32=${COREBOOT_SDK_PREFIX}/bin/i386-elf-

# @ECLASS-VARIABLE: COREBOOT_SDK_PREFIX_x86_64
# @DESCRIPTION:
#   Prefix of coreboot SDK binaries for 64-bit x86.
COREBOOT_SDK_PREFIX_x86_64=${COREBOOT_SDK_PREFIX}/bin/x86_64-elf-

# @ECLASS-VARIABLE: COREBOOT_SDK_ENABLED_TOOLCHAINS
# @DESCRIPTION:
# An array containing "arch:filename" for each enabled toolchain.  This variable
# normally gets populated by calling coreboot-sdk_enable.  Normally we'd use an
# associative array, but Portage serializes the environment into
# ${T}/environment, and does not successfully serialize associative arrays.
COREBOOT_SDK_ENABLED_TOOLCHAINS=()

# @FUNCTION: coreboot-sdk_enable
# @USAGE: <architecture version> [use_flags...]
# @DESCRIPTION:
# Enable a toolchain package.  This appends variables to SRC_URI and updates
# COREBOOT_SDK_ENABLED_TOOLCHAINS.
# version should be in the format "${PVR}/${HASH}", and gets used to create the
# URL: gs://chromiumos-sdk/toolchains/coreboot-sdk-${arch}/${version}.tar.zst.
# use_flags may optionally be specified to enable for only a given set of flags.
# For example, appending the arguments "amd64" "!coreboot_x86_64" will cause the
# toolchain to only be installed if amd64 is set, and coreboot_x86_64 is unset.
coreboot-sdk_enable() {
	local arch="$1"
	shift 1
	if [[ -z "${!COREBOOT_SDK_VERSIONS[@]}" ]]; then
		die "COREBOOT_SDK_VERSIONS not defined"
	fi
	if [[ -z "${COREBOOT_SDK_VERSIONS["${arch}"]+x}" ]]; then
		die "COREBOOT_SDK_VERSIONS not defined for ${arch}"
	fi

	local gcs_bucket="chromiumos-sdk"
	local version="${COREBOOT_SDK_VERSIONS["${arch}"]}"
	local version_hash="${version##*/}"
	local toolchain_version="${version%%/*}"
	local dest="coreboot-sdk-${arch}-${toolchain_version}-${version_hash}.tar.zst"
	local toolchain
	if ! has "${arch}:${dest}" "${COREBOOT_SDK_ENABLED_TOOLCHAINS[@]}"; then
		for toolchain in "${COREBOOT_SDK_ENABLED_TOOLCHAINS[@]}"; do
			if [[ "${toolchain}" == "${arch}":* ]]; then
				die "Conflicting coreboot-sdk versions for ${arch} is not permitted."
			fi
		done
		COREBOOT_SDK_ENABLED_TOOLCHAINS+=( "${arch}:${dest}" )
	fi

	if [[ "${#COREBOOT_SDK_BUCKET_OVERRIDES[@]}" -gt 0 ]]; then
		if [[ -n "${COREBOOT_SDK_BUCKET_OVERRIDES["${arch}"]+x}" ]]; then
			gcs_bucket="${COREBOOT_SDK_BUCKET_OVERRIDES["${arch}"]}"
		fi
	fi

	COREBOOT_SDK_PREFIX="${WORKDIR}/coreboot-sdk"
	COREBOOT_SDK_PREFIX_arm="${COREBOOT_SDK_PREFIX}/bin/arm-eabi-"
	COREBOOT_SDK_PREFIX_arm64="${COREBOOT_SDK_PREFIX}/bin/aarch64-elf-"
	COREBOOT_SDK_PREFIX_nds32="${COREBOOT_SDK_PREFIX}/bin/nds32le-elf-"
	COREBOOT_SDK_PREFIX_riscv="${COREBOOT_SDK_PREFIX}/bin/riscv64-elf-"
	COREBOOT_SDK_PREFIX_x86_32="${COREBOOT_SDK_PREFIX}/bin/i386-elf-"
	COREBOOT_SDK_PREFIX_x86_64="${COREBOOT_SDK_PREFIX}/bin/x86_64-elf-"

	: "${SRC_URI:=}" "${RESTRICT:=}"

	# We pull from gs://chromiumos-sdk.
	RESTRICT+=" mirror"

	local uri="gs://${gcs_bucket}/toolchains/coreboot-sdk-${arch}/${version}.tar.zst -> ${dest}"
	local use_flag
	for use_flag in "$@"; do
		uri="${use_flag}? ( ${uri} )"
	done
	SRC_URI+=" ${uri}"
}

# Replaces the coreboot-sdk libc with the SDK's.
#
# This is necessary because portage's sandboxing puts the chroot's
# libsandbox.so, which is built against the SDK's glibc, in LD_PRELOAD. Trying
# to use coreboot-sdk's glibc with libsandbox.so can lead to breakages after
# glibc updates, e.g., b/395911671
#
# Since glibc offers very strong promises about backwards-compatibility, and the
# glibc in the SDK is the upstream for coreboot-sdk's glibc anyway, punching
# this hole seems fine.
_coreboot-sdk_punch_libc_holes() {
	local coreboot_sdk_root="$1"
	local dsos_to_forward=(
		lib/ld-linux-x86-64.so.2
		lib/libc.so.6
		lib/libm.so.6
	)
	local dso src targ
	for dso in "${dsos_to_forward[@]}"; do
		src="/${dso}"
		targ="${coreboot_sdk_root}/${dso}"
		if [[ ! -e "${targ}" ]]; then
			die "No such coreboot DSO: ${targ}"
		fi
		if [[ ! -e "${src}" ]]; then
			die "No such SDK DSO: ${src}"
		fi
		rm -f "${targ}" || die
		# Note that this can't be a symlink, due to reclient logic;
		# see post-commit discussion on crrev.com/c/6254912.
		cp -a "${src}" "${targ}" || die
	done
}

# @FUNCTION: coreboot-sdk_src_unpack
# @DESCRIPTION:
#   Unpack toolchains enabled via coreboot-sdk_enable.  This function will call
#   cros-workon_src_unpack as required, so most users won't need to define a
#   src_unpack or call either function manually.
coreboot-sdk_src_unpack() {
	# cros-workon usage is so common, and this eclass is normally inherited
	# after.  So everyone doesn't need to define a src_unpack to call both, and
	# we don't break existing users, automatically call cros-workon_src_unpack
	# as required.
	if [[ "$(type -t cros-workon_src_unpack)" == "function" ]]; then
		cros-workon_src_unpack || die
	fi

	local value arch tarball unpacked_something

	for value in "${COREBOOT_SDK_ENABLED_TOOLCHAINS[@]}"; do
		unpacked_something=1
		arch="${value%%:*}"
		tarball="${value#*:}"

		# The tarball may not exist as the SRC_URI value can be conditional to
		# USE flags.
		if [[ ! -f "${DISTDIR}/${tarball}" ]]; then
			einfo "Tarball does not exist for coreboot-sdk-${arch}, skipping"
			continue
		fi

		einfo "Setting up coreboot-sdk toolchain: ${tarball}"
		mkdir -p "${COREBOOT_SDK_PREFIX}" || die
		tar -I pzstd -C "${COREBOOT_SDK_PREFIX}" --no-same-owner -xf \
			"${DISTDIR}/${tarball}" || die

		if [[ "${arch}" == iasl ]]; then
			# Need to patch up edk2config for new prefix.
			sed -i -e "s#/opt/coreboot-sdk#${COREBOOT_SDK_PREFIX}#g" \
				"${COREBOOT_SDK_PREFIX}/share/edk2config/tools_def.txt" || die
		fi
	done

	if [[ -n "${unpacked_something}" ]]; then
		_coreboot-sdk_punch_libc_holes "${COREBOOT_SDK_PREFIX}"
	fi
}

EXPORT_FUNCTIONS src_unpack

fi
