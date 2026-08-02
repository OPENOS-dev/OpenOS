# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT=("0349b538289a073973b9c06b1ebb06d8099cb572" "0dd679081b9c8bfa2583d74e3a17a413709ea362")
CROS_WORKON_TREE=("ff484a7f91051d936adf0dbbb57ff40f3ccc9637" "d99abee3f825248f344c0638d5f9fcdce114b744")
CROS_WORKON_PROJECT=(
	"chromiumos/platform/ec"
	"chromiumos/third_party/cryptoc"
)
CROS_WORKON_LOCALNAME=(
	"platform/ish"
	"third_party/cryptoc"
)
CROS_WORKON_DESTDIR=(
	"${S}/platform/ish"
	"${S}/third_party/cryptoc"
)
CROS_WORKON_EGIT_BRANCH=(
	ish
	main
)

inherit cros-workon cros-unibuild coreboot-sdk

# @ECLASS-VARIABLE: COREBOOT_SDK_VERSIONS
# @DESCRIPTION:
#   Associative array of the architectures and their respective versions
#   that should be used by ebuilds inheriting from this eclass.
declare -gA COREBOOT_SDK_VERSIONS=(
    [i386-elf]="11.3.0-r2/cc719670413bdcf8089775f0aca0506d21a60282"
)

coreboot-sdk_enable i386-elf

DESCRIPTION="CrOS EC-based ISH image used on Arcada and Drallion"
HOMEPAGE="https://www.chromium.org/chromium-os/ec-development"

LICENSE="BSD-Google"
KEYWORDS="*"

src_unpack() {
	coreboot-sdk_src_unpack
	S+="/platform/ish"
}

src_compile() {
	export COREBOOT_SDK_ROOT_x86="${COREBOOT_SDK_PREFIX}"
	export CROSS_COMPILE_CC_NAME=gcc
	export CROSS_COMPILE_i386=${COREBOOT_SDK_PREFIX_x86_32}

	tc-export BUILD_PKG_CONFIG
	export BUILDCC="$(tc-getBUILD_CC)"
	export CC="${CROSS_COMPILE_i386}gcc"

	local target
	while read -r target; do
		if [[ -z "${target}" ]]; then
			continue
		fi
		einfo "Building target: ${target}"
		emake BOARD="${target}" "build/${target}/ec.bin" || die
	done < <(cros_config_host get-firmware-build-targets ish || die)
}

src_install() {
	insinto /lib/firmware/intel

	local target
	while read -r target; do
		if [[ -z "${target}" ]]; then
			continue
		fi
		newins "build/${target}/ec.bin" "${target}.bin"
	done < <(cros_config_host get-firmware-build-targets ish || die)
}
