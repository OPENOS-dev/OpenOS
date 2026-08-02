# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# @ECLASS: cros-zephyr-fpmcu.eclass
# @MAINTAINER:
# Chromium OS Firmware Team
# @BUGREPORTS:
# Please report bugs via http://crbug.com/new (with label Build)
# @VCSURL: https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/eclass/@ECLASS@
# @BLURB: helper eclass for building Zephyr FPMCU firmware
# @DESCRIPTION:
# Release FPMCU firmware needs to be built from specific branches/commits and
# must be signed by the signing daemon. This eclass provides a standardized
# mechanism for that process by building for a specific Zephyr program and
# installing it into /build/<board>/<FPMCU_board>/release/zephyr so that the
# signer can pick it up. Note that this doesn't install the firmware into the
# rootfs; that has to be done by a separate ebuild since the signer runs after
# the build.
#
# NOTE: When making changes to this class, make sure to modify the
# `sys-firmware/chromeos-zephyr-fpmcu-release-*/files/revision_bump' to cause
# package rebuilds (work around for https://issuetracker.google.com/201299127).

if [[ -z "${_ECLASS_CROS_ZEPHYR_FPMCU}" ]]; then
_ECLASS_CROS_ZEPHYR_FPMCU="1"

# Check for EAPI 7+.
case "${EAPI:-0}" in
0|1|2|3|4|5|6) die "unsupported EAPI (${EAPI}) in eclass (${ECLASS})" ;;
*) ;;
esac

# @ECLASS-VARIABLE: ZEPHYR_PROGRAM
# @DEFAULT_UNSET
# @DESCRIPTION:
# Zephyr program to build.
: "${ZEPHYR_PROGRAM:=}"

if [[ -z "${ZEPHYR_PROGRAM}" ]]; then
	die "ZEPHYR_PROGRAM must be specified in ebuild."
fi

inherit cros-workon cros-ec-merge-ro cros-zephyr-utils

IUSE="internal"

BDEPEND="
	cross-armv7m-cros-eabi/binutils
	cross-armv7m-cros-eabi/compiler-rt
	cross-armv7m-cros-eabi/libcxx
	cross-armv7m-cros-eabi/newlib
	cross-riscv32-cros-elf/binutils
	cross-riscv32-cros-elf/compiler-rt
	cross-riscv32-cros-elf/libcxx
	cross-riscv32-cros-elf/newlib
"

# @FUNCTION: cros-zephyr-fpmcu_src_prepare
# @DESCRIPTION:
# Prepare sources for compilation. Create symbolic link to private sources
# in EC repository.
cros-zephyr-fpmcu_src_prepare() {
	eapply_user

	# We don't list the private repositories in CROS_WORKON_PROJECT since this
	# is a public ebuild. Instead, the fingerprint private repositories are
	# installed to "${SYSROOT}/firmware/${ZEPHYR_PROGRAM}/release/fingerprint"
	# by `chromeos-ec-private-files`.
	# We link these private repositories into the "modules" directory.

	# Rename 'egis' to 'egis_fp' to avoid conflict with existing zephyr module.
	ln -sfT "${SYSROOT}/firmware/${ZEPHYR_PROGRAM}/release/fingerprint/egis" \
		"${S}/zephyrproject/modules/egis_fp" || die

	# Rename 'elan' to 'elan_fp' to avoid conflict with existing zephyr module.
	ln -sfT "${SYSROOT}/firmware/${ZEPHYR_PROGRAM}/release/fingerprint/elan" \
		"${S}/zephyrproject/modules/elan_fp" || die

	# Rename 'focaltech' to 'focaltech_fp' to avoid conflict with existing
	# zephyr module.
	ln -sfT "${SYSROOT}/firmware/${ZEPHYR_PROGRAM}/release/fingerprint/focaltech" \
		"${S}/zephyrproject/modules/focaltech_fp" || die

	ln -sfT "${SYSROOT}/firmware/${ZEPHYR_PROGRAM}/release/fingerprint/fpc" \
		"${S}/zephyrproject/modules/fpc" || die
}

# @FUNCTION: cros-zephyr-fpmcu_src_compile
# @DESCRIPTION:
# Compile Zephyr program specified in ZEPHYR_PROGRAM variable.
cros-zephyr-fpmcu_src_compile() {
	# Let the EC Makefiles set the compiler. It already has logic for choosing
	# between gcc and clang for the different architectures that EC builds.
	unset CC

	# Setting SYSROOT to the board's sysroot (e.g., /build/hatch) causes
	# compilation failures when cross-compiling for other targets (e.g., ARM).
	unset SYSROOT

	# Let the EC Makefiles determine architecture flags. For example, using
	# "-march=goldmont" fails when using "armv7m-cros-eabi-clang" with the
	# error: the clang compiler does not support '-march=goldmont'
	filter-flags "-march=*"

	local version="no_version"
	local gdesc="$(git -C zephyrproject/modules/ec describe --dirty --match='v*' 2>/dev/null)"

	if [[ -n "${gdesc}" ]]; then
		local fields vernum numcommits

		IFS="-" read -r -a fields <<< "${gdesc}"
		IFS="." read -r -a vernum <<< "${fields[0]}"
		numcommits=$((vernum[2]+${fields[1]:-0}))
		version="${vernum[0]}.${vernum[1]}.${numcommits}"
	fi

	einfo "Building targets: ${ZEPHYR_PROGRAM}"

	run_zmake build -B "build" --version "${version}" "${ZEPHYR_PROGRAM}" \
		|| die "Failed to build ${ZEPHYR_PROGRAM}."
}

# @FUNCTION: cros-zephyr-fpmcu_src_install
# @DESCRIPTION:
# Install compiled firmware to the firmware/${BOARD}/release/zephyr directory
# Replace RO firmware if requested.
cros-zephyr-fpmcu_src_install() {
	# Replace RO if requested.
	if [[ -n "${FIRMWARE_RELEASE_REPLACE_RO}" ]] && [[ "${FIRMWARE_RELEASE_REPLACE_RO}" == "yes" ]]; then
		if use internal; then
			local firmware_bin_dir="${SYSROOT}/firmware/fpmcu-firmware-binaries"

			# Check if firmware-bin directory exists.
			if [[ ! -d "${firmware_bin_dir}" ]]; then
				die "RO firmware directory not found: ${firmware_bin_dir}"
			fi

			# Find appropriate RO file.
			local ro_fw="$(cros-ec-merge-ro_find_ro_fw "${ZEPHYR_PROGRAM}" "${firmware_bin_dir}")"
			local rw_fw="${S}/build/${ZEPHYR_PROGRAM}/output/ec.bin"
			local merged_fw="$(cros-ec-merge-ro_do_merge "${ro_fw}" "${rw_fw}")"
			cp "${merged_fw}" "${rw_fw}" || die
		else
			einfo "Skipping RO replacement in a public build."
		fi
	fi

	# Do not strip elf files so debug symbols are available
	# in the firmware_from_source.tar.bz2 bundles from builders.
	dostrip -x "/firmware/${ZEPHYR_PROGRAM}/release/zephyr"/zephyr.{rw,ro}.elf

	# Run 'insinto' in subshell to avoid changing the insinto path for
	# the caller.
	(
	insinto "/firmware/${ZEPHYR_PROGRAM}/release/zephyr"
	doins "build/${ZEPHYR_PROGRAM}"/output/*
	)
}

EXPORT_FUNCTIONS src_prepare src_compile src_install

fi  # _ECLASS_CROS_ZEPHYR_FPMCU
