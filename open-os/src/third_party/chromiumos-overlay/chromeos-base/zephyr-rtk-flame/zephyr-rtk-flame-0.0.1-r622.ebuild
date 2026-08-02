# Copyright 2026 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.makefile file.

EAPI=7

CROS_WORKON_COMMIT=("8c3719f3ba4bd018bdf242905bee85a3668a5ac1" "22ddd18cef071f61fa17c54d72f536c97696a847")
CROS_WORKON_TREE=("123239e7f10ad93cab8546673f5ea5d2ced064c4" "1387f1c1e8aeb0cc210b5099180dbfdbbad30e21")
CROS_WORKON_USE_VCSID=1
CROS_WORKON_PROJECT=(
	"chromiumos/third_party/zephyrproject"
	"chromiumos/platform/ec"
)

CROS_WORKON_LOCALNAME=(
	"third_party/zephyrproject"
	"platform/ec"
)

CROS_WORKON_DESTDIR=(
	"${S}/zephyrproject"
	"${S}/zephyrproject/modules/ec"
)

CROS_WORKON_EGIT_BRANCH=(
	"main"
	"main"
)

inherit cros-workon cros-zephyr-utils coreboot-sdk coreboot-sdk-ec-dependencies

DESCRIPTION="Zephyr-based Monitor Firmware for ChromiumOS RTK EC"
KEYWORDS="*"

export RTK_FLAME_PROJECT="rtk_flame"

coreboot-sdk_enable arm-eabi

src_compile() {
	tc-export CC

	run_zmake build -B "build" --version "$(cros-version)" --static \
		"${RTK_FLAME_PROJECT}" ||
		die "Failed to build ${RTK_FLAME_PROJECT}."

}

src_install() {
	insinto /usr/share/ec-devutils
	doins "build/${RTK_FLAME_PROJECT}/build-singleimage/rts5915_flash_upload.bin"
}
