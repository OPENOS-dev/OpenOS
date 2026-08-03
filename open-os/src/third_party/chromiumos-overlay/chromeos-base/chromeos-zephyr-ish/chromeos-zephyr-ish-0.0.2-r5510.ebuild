# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.makefile file.

EAPI=7

CROS_WORKON_COMMIT=("8c3719f3ba4bd018bdf242905bee85a3668a5ac1" "ce4bed460493919c8f1c7ea614856d2fd79933d2" "22ddd18cef071f61fa17c54d72f536c97696a847")
CROS_WORKON_TREE=("123239e7f10ad93cab8546673f5ea5d2ced064c4" "70e996d49f0a4c553f3509ae270e99f9f032d16c" "1387f1c1e8aeb0cc210b5099180dbfdbbad30e21")
inherit cros-workon cros-zephyr-utils coreboot-sdk coreboot-sdk-ec-dependencies

CROS_WORKON_USE_VCSID=1
CROS_WORKON_PROJECT=(
	"chromiumos/third_party/zephyrproject"
	"chromiumos/third_party/pigweed/pigweed"
	"chromiumos/platform/ec"
)

CROS_WORKON_LOCALNAME=(
	"third_party/zephyrproject"
	"third_party/pigweed"
	"platform/ec"
)

CROS_WORKON_DESTDIR=(
	"${S}/zephyrproject"
	"${S}/zephyrproject/modules/pigweed"
	"${S}/zephyrproject/modules/ec"
)

CROS_WORKON_REPO=${CROS_GIT_INT_HOST_URL}

DESCRIPTION="Zephyr based firmware for ISH enabled boards"
KEYWORDS="*"
IUSE="zephyr_ish_pinned"

coreboot-sdk_enable i386-elf
coreboot-sdk_enable libstdcxx-i386-elf
coreboot-sdk_enable picolibc-i386-elf

src_compile() {
	cros-zephyr-compile ish
}

src_install() {
	local project
	local firmware_name

	while read -r firmware_name && read -r project; do
		if [[ -z "${project}" ]]; then
			continue
		fi

		# Install into firmware directory so it gets uploaded
		insinto "/firmware/${firmware_name}/${project}"
		doins "build/${project}"/output/*
	done < <(cros_config_host "get-firmware-build-combinations" ish || die)
}
