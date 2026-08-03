# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# @ECLASS: coreboot-sdk-ap-dependencies.eclass
# @BLURB: Common dependency declaration for the AP
# @DESCRIPTION:
# coreboot-sdk uses the subtool. A few AP ebuilds use the same versions of
# the subtool toolchains.  Consolidate them all into a single place.

if [[ -z "${_ECLASS_COREBOOT_SDK_AP_DEPS}" ]]; then
_ECLASS_COREBOOT_SDK_AP_DEPS=1

# Check for EAPI 7+.
case "${EAPI:-0}" in
[0123456]) die "unsupported EAPI (${EAPI}) in eclass (${ECLASS})" ;;
esac

# @ECLASS-VARIABLE: COREBOOT_SDK_VERSIONS
# @DESCRIPTION:
#   Associative array of the architectures and their respective versions
#   that should be used by ebuilds inheriting from this eclass.
#   When you update this, also update the manifest files for all relevant
#   packages.
#
#   cros_sdk --enter
#   for d in $(find ~/chromiumos/src/third_party/chromiumos-overlay \
#     ~/chromiumos/src/private-overlays -name Manifest \
#     -exec grep -q '^DIST coreboot-sdk-' {} \; -print | \
#     sed -e 's/\/Manifest$//') ; do for i in $d/*.ebuild ; do
#       ebuild $i manifest;
#       re='(.*-r)([0-9]+)\.ebuild$'; if [[ "$i" =~ $re ]] && \
#         egrep -q 'CROS_WORKON_MANUAL_UPREV="?1"?' $d/*-9999.ebuild ; then
#         echo mv "$i" "${BASH_REMATCH[1]}$(( 1 + ${BASH_REMATCH[2]} )).ebuild" ;
#         mv "$i" "${BASH_REMATCH[1]}$(( 1 + ${BASH_REMATCH[2]} )).ebuild" ;
#       fi ; done; done
declare -gA COREBOOT_SDK_VERSIONS=(
	[aarch64-elf]="14.2.0-r6/fd37a4a0aaaac463f4b7497975f406a958d15812"
	[arm-eabi]="14.2.0-r6/818451e954afd20428272e300f74960e293d541d"
	[i386-elf]="14.2.0-r6/bce1746b2b84b6f36fbb2448e40761fff326714a"
	[iasl]="14.2.0-r6/e725285a38d593c4a9cd6f06f76beee02592b932"
	[x86_64-elf]="14.2.0-r6/5f2f11928fb6d4e8cc49374da1e529c4b21a1faf"
)
export COREBOOT_SDK_VERSIONS

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
