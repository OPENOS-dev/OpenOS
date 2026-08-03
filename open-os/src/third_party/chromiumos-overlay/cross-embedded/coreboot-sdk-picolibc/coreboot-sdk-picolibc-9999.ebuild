# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_PROJECT="chromiumos/third_party/zephyrproject"
CROS_WORKON_LOCALNAME="zephyrproject"

inherit cros-workon

if [[ "${CATEGORY}" == "cross-embedded" ]]; then
	DESCRIPTION="Template package for cross-embedded-*/coreboot-sdk-picolibc"
else
	COREBOOT_SDK_ARCH="${CATEGORY#cross-embedded-}"
	DESCRIPTION="coreboot-sdk cross-compiler for ${COREBOOT_SDK_ARCH} with picolibc support"
	BDEPEND="
		dev-embedded/coreboot-sdk-bootstrap
		cross-embedded-${COREBOOT_SDK_ARCH}/coreboot-sdk
	"
fi
HOMEPAGE="https://www.coreboot.org"
LICENSE="GPL-3 LGPL-3"
KEYWORDS="~*"
BDEPEND+="
	dev-build/meson
	dev-util/ninja
"
# The path this package will install to.
COREBOOT_SDK_DESTDIR="/opt/coreboot-sdk"

src_compile() {
	if [[ "${COREBOOT_SDK_ARCH}" == "" ]]; then
		die "COREBOOT_SDK_ARCH is not set.  You should not directly" \
			"emerge cross-embedded/coreboot-sdk-picolibc, but instead the package " \
			"for a specific architecture (cross-embedded-ARCH/coreboot-sdk-picolibc)"
	fi
	ARCH="${COREBOOT_SDK_ARCH}"
	if [[ "${COREBOOT_SDK_ARCH}" == riscv-elf ]]; then
		ARCH="riscv64-elf"
	fi
	BUILD_CC="${COREBOOT_SDK_DESTDIR}-bootstrap/x86_64-pc-linux-gnu-gcc"
	BUILD_CXX="${COREBOOT_SDK_DESTDIR}-bootstrap/x86_64-pc-linux-gnu-g++"
	tc-export BUILD_CC BUILD_CXX

	PICOLIBC_BUILD_DIR="${WORKDIR}/build_dir"
	mkdir -p "${PICOLIBC_BUILD_DIR}"
	mkdir -p "${WORKDIR}${COREBOOT_SDK_DESTDIR}/picolibc"
	cd "${PICOLIBC_BUILD_DIR}" || die
	PATH="${COREBOOT_SDK_DESTDIR}/bin:${PATH}"
	"${S}/modules/lib/picolibc/scripts/do-coreboot-${ARCH}"-configure \
		-Dprefix="${WORKDIR}${COREBOOT_SDK_DESTDIR}" \
		-Dspecsdir="${WORKDIR}${COREBOOT_SDK_DESTDIR}/picolibc/coreboot-${ARCH}"
	ninja
}

src_install() {
	local files
	cd "${PICOLIBC_BUILD_DIR}" || die
	ninja install
	mkdir -p "${WORKDIR}${COREBOOT_SDK_DESTDIR}/picolibc/coreboot-${ARCH}/usr/"
	cd "${WORKDIR}${COREBOOT_SDK_DESTDIR}/picolibc/coreboot-${ARCH}/usr/" || die
	ln -s "../include" "${WORKDIR}${COREBOOT_SDK_DESTDIR}/picolibc/coreboot-${ARCH}/usr/include" || die
	mkdir -p "${D}${COREBOOT_SDK_DESTDIR}/picolibc"
	cp -r -L "${WORKDIR}${COREBOOT_SDK_DESTDIR}/picolibc/coreboot-${ARCH}" "${D}${COREBOOT_SDK_DESTDIR}/picolibc" || die
	readarray -t files < <(find "${D}" -name '*.[ao]' -printf "/%P\n")
	dostrip -x "${files[@]}"
}
