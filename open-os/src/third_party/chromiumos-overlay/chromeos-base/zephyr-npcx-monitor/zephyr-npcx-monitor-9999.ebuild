# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.makefile file.

EAPI=7

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

DESCRIPTION="Zephyr-based Monitor Firmware for ChromiumOS NPCX EC"
KEYWORDS="~*"

export NPCX_MONITOR_PROJECT="npcx_monitor"

coreboot-sdk_enable arm-eabi

src_compile() {
	tc-export CC

	run_zmake build -B "build" --version "$(cros-version)" --static \
		"${NPCX_MONITOR_PROJECT}" ||
		die "Failed to build ${NPCX_MONITOR_PROJECT}."

}

src_install() {
	insinto /usr/share/ec-devutils
	doins "build/${NPCX_MONITOR_PROJECT}/build-singleimage/npcx_monitor.bin"
}
