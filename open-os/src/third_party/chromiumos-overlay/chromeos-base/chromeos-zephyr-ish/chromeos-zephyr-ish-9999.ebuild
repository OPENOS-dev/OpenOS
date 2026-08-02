# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.makefile file.

EAPI=7

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
KEYWORDS="~*"
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
