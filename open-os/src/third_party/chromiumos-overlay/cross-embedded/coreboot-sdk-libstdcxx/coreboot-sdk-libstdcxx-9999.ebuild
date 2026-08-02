# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_PROJECT="chromiumos/third_party/coreboot"
CROS_WORKON_LOCALNAME="coreboot"
CROS_WORKON_SUBTREE="util/crossgcc"

inherit cros-workon coreboot-sdk-build coreboot-sdk-versions

if [[ "${CATEGORY}" == cross-embedded ]]; then
	COREBOOT_SDK_ARCH=""
	DESCRIPTION="Template package for cross-embedded-*/coreboot-sdk-libstdcxx"
else
	COREBOOT_SDK_ARCH="${CATEGORY#cross-embedded-}"
	DESCRIPTION="coreboot-sdk cross-compiler for ${COREBOOT_SDK_ARCH} with libstdc++ support"
	BDEPEND="
		dev-embedded/coreboot-sdk-bootstrap
		cross-embedded-${COREBOOT_SDK_ARCH}/coreboot-sdk-picolibc
	"
fi
HOMEPAGE="https://www.coreboot.org"
LICENSE="GPL-3 LGPL-3"
KEYWORDS="~*"

# shellcheck disable=SC2154
SRC_URI="
	${GCC_SRC_URI}
"

# The path this package will install to.
COREBOOT_SDK_DESTDIR="/opt/coreboot-sdk"


src_install() {
	local files

	ARCH="${COREBOOT_SDK_ARCH}"
	if [[ "${COREBOOT_SDK_ARCH}" == riscv-elf ]]; then
		ARCH="riscv64-elf"
	fi
	# libgcc built both as a part of the base build and as a part of the libstdcxx build, avoid conflict
	# shellcheck disable=SC2154
	rm -rf "${WORKDIR}${COREBOOT_SDK_DESTDIR}/lib/gcc/${ARCH}/${GCC_VERSION}/libgcc.a"
	rm -rf "${WORKDIR}${COREBOOT_SDK_DESTDIR}/lib/gcc/${ARCH}/${GCC_VERSION}/libgcov.a"
	rm -rf "${WORKDIR}${COREBOOT_SDK_DESTDIR}/lib/gcc/${ARCH}/${GCC_VERSION}/crtend.o"
	rm -rf "${WORKDIR}${COREBOOT_SDK_DESTDIR}/lib/gcc/${ARCH}/${GCC_VERSION}/crtbegin.o"
	rm -rf "${WORKDIR}${COREBOOT_SDK_DESTDIR}/lib/gcc/${ARCH}/${GCC_VERSION}/include/gcov.h"
	rm -rf "${WORKDIR}${COREBOOT_SDK_DESTDIR}/lib/gcc/${ARCH}/${GCC_VERSION}/include/unwind.h"

	# buildgcc script always generates the tools_def.txt file.  Avoid conflict
	rm -rf "${WORKDIR}${COREBOOT_SDK_DESTDIR}/share/edk2config/tools_def.txt"

	mkdir -p "${D}${COREBOOT_SDK_DESTDIR}/libstdcxx/${ARCH}/include" || die
	cp -r -L "${S}/build-${ARCH}-LIBSTDCXX/${ARCH}/libstdc++-v3/include" "${D}${COREBOOT_SDK_DESTDIR}/libstdcxx/${ARCH}" || die
	cp -r -L "${S}/gcc-${GCC_VERSION}/libstdc++-v3/libsupc++" "${D}${COREBOOT_SDK_DESTDIR}/libstdcxx/${ARCH}/include" || die
	cp -a "${WORKDIR}${COREBOOT_SDK_DESTDIR}/${ARCH}/lib" "${D}${COREBOOT_SDK_DESTDIR}/libstdcxx/${ARCH}" || die

	readarray -t files < <(find "${D}${COREBOOT_SDK_DESTDIR}/libstdcxx" -name '*.[ao]' -printf "/%P\n")
	dostrip -x "${files[@]}"
}

src_compile() {
	local languages="c,c++"

	if [[ "${COREBOOT_SDK_ARCH}" == "" ]]; then
		die "COREBOOT_SDK_ARCH is not set.  You should not directly" \
			"emerge cross-embedded/coreboot-sdk-libstdcxx, but instead the package " \
			"for a specific architecture (cross-embedded-ARCH/coreboot-sdk-libstdcxx)"
	elif [[ "${COREBOOT_SDK_ARCH}" == "arm-eabi" ]]; then
		# Force buildgcc script to use additional arm build flags
		export CFLAGS_FOR_TARGET_EXTRA="-mno-unaligned-access -mcpu=cortex-m4"
	elif [[ "${COREBOOT_SDK_ARCH}" == "riscv64-elf" ]]; then
		# Force buildgcc script to use additional arm build flags
		export CFLAGS_FOR_TARGET_EXTRA="-march=rv32imac_zicsr_zifencei -mabi=ilp32"
	fi
	coreboot-sdk-build_buildgcc \
		--platform "${COREBOOT_SDK_ARCH}" \
		--languages "${languages}" \
		--package "libstdcxx" \
		--savetemps \
		--libstdcxx_include "${COREBOOT_SDK_DESTDIR}/picolibc/coreboot-${COREBOOT_SDK_ARCH}"
}
