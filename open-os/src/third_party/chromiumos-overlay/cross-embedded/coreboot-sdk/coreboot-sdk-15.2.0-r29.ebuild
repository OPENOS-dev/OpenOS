# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT="82390cc046aaa9e947f1e5261269204c6ea4122d"
CROS_WORKON_TREE="bc79451ecc77e95c4a0ef3b049d10b4355d09b84"
CROS_WORKON_PROJECT="chromiumos/third_party/coreboot"
CROS_WORKON_LOCALNAME="coreboot"
CROS_WORKON_SUBTREE="util/crossgcc"

inherit cros-workon coreboot-sdk-build

if [[ "${CATEGORY}" == cross-embedded ]]; then
	COREBOOT_SDK_ARCH=""
	DESCRIPTION="Template package for cross-embedded-*/coreboot-sdk"
else
	COREBOOT_SDK_ARCH="${CATEGORY#cross-embedded-}"
	DESCRIPTION="coreboot-sdk cross-compiler for ${COREBOOT_SDK_ARCH}"
fi
HOMEPAGE="https://www.coreboot.org"
LICENSE="GPL-3 LGPL-3"
KEYWORDS="*"
BDEPEND="
	dev-embedded/coreboot-sdk-bootstrap
	dev-embedded/coreboot-sdk-iasl
"

src_compile() {
	local languages

	# Only need Ada support for compilers used by coreboot.
	case "${COREBOOT_SDK_ARCH}" in
		"" )
			die "COREBOOT_SDK_ARCH is not set.  You should not directly" \
				"emerge cross-embedded/coreboot-sdk, but instead the package" \
				"for a specific architecture (cross-embedded-ARCH/coreboot-sdk)"
			;;
		aarch64-elf|arm-eabi|i386-elf|x86_64-elf )
			languages="c,c++,ada"
			;;
		* )
			languages="c,c++"
			;;
	esac

	if [[ "${COREBOOT_SDK_ARCH}" == "arm-eabi" ]]; then
		# Force buildgcc script to use additional arm build flags
		export CFLAGS_FOR_TARGET_EXTRA="-mno-unaligned-access -mcpu=cortex-m4"
	fi
	coreboot-sdk-build_buildgcc \
		--platform "${COREBOOT_SDK_ARCH}" \
		--languages "${languages}"

	# Drop host headers and libs.
	rm -rf "${WORKDIR}${COREBOOT_SDK_DESTDIR:?}"/{include,share}
	rm -rf "${WORKDIR}${COREBOOT_SDK_DESTDIR:?}"/lib/{bfd-plugins,pkgconfig}
	rm -rf "${WORKDIR}${COREBOOT_SDK_DESTDIR:?}"/lib/lib*.{a,la}
}
