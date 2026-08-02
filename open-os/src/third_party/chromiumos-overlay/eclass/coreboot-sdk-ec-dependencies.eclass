# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# @ECLASS: coreboot-sdk-ec-dependencies.eclass
# @BLURB: Common dependency declaration for the EC
# @DESCRIPTION:
# coreboot-sdk uses the subtool. A few EC ebuilds use the same versions of
# the subtool toolchains.  Consolidate them all into a single place.

if [[ -z "${_ECLASS_COREBOOT_SDK_EC_DEPS}" ]]; then
_ECLASS_COREBOOT_SDK_EC_DEPS=1

# Check for EAPI 7+.
case "${EAPI:-0}" in
[0123456]) die "unsupported EAPI (${EAPI}) in eclass (${ECLASS})" ;;
esac

# @ECLASS-VARIABLE: COREBOOT_SDK_VERSIONS
# @DESCRIPTION:
#   Associative array of the architectures and their respective versions
#   that should be used by ebuilds inheriting from this eclass.
# shellcheck disable=SC2034
declare -gA COREBOOT_SDK_VERSIONS=(
	[nds32le-elf]="14.2.0-r4/d1a518c4a097666d86058e1444f85ca015d15511"
	[i386-elf]="14.2.0-r4/dfbb0486385d938cfa9d73080b9d18d598ed4f49"
	[picolibc-i386-elf]="14.2.0-r2/eb8e8e687b95461de8cbf278f1c2076952d0ad17"
	[libstdcxx-i386-elf]="14.2.0-r2/42ef9d2e87a76a37963175d7d428734e65648b87"
	[arm-eabi]="14.2.0-r4/ea40ddd1d190c3b48435c474c0b545719371813a"
	[picolibc-arm-eabi]="14.2.0-r2/422a2aa3e1ef891b5bece9a3deb696ff83840d2e"
	[libstdcxx-arm-eabi]="14.2.0-r2/5d7d82e150e44aa1968f1d366e267e6687b328ac"
	[riscv-elf]="14.2.0-r3/9a27b9e82341c6e9126624f516ab3f261abb3e73"
	[riscv64-elf]="14.2.0-r4/96b949749225b6aaa92ed810ab769e8dfb12f000"
	[picolibc-riscv64-elf]="14.2.0-r2/f571e990ef0278564b31124f8bb109d9a697eb69"
	[libstdcxx-riscv64-elf]="14.2.0-r2/17abf22a3a252e2d5d2062ced5324f7a623689e0"
)

# @ECLASS-VARIABLE: COREBOOT_SDK_BUCKET_OVERRIDES
# @DESCRIPTION:
#   Associative array of the architectures and their respective override
#   GCS buckets to be used.
#
#   To use a local build and/or point the ebuild to a different bucket for
#   a binary, uncomment the below array and add an entry with the
#   architecture/bucket name key/value pair.
#
#   For example:
#       [i386-elf]="chromeos-throw-away-bucket"
#
#   NOTE: DO NOT SUBMIT THIS ARRAY
#       Merging any changes to the below array will cause production builds to use
#       an unverified and unmanaged toolchain.
#declare -gA COREBOOT_SDK_BUCKET_OVERRIDES=(
#)

fi
