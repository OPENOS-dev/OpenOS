# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.makefile file.

EAPI=7

CROS_WORKON_COMMIT="30194971a96d0c1d91ada5d04a84679a3ed45d05"
CROS_WORKON_TREE="4a83749dc7caa0a42caae9edd7f83722293354a8"
CROS_WORKON_PROJECT=(
	"chromiumos/platform/ec"
)
CROS_WORKON_LOCALNAME=(
	"platform/ec-legacy"
)
CROS_WORKON_DESTDIR=(
	"${S}/platform/ec-legacy"
)
CROS_WORKON_EGIT_BRANCH=(
	"ec-legacy"
)

inherit cros-ec cros-workon cros-sanitizers cros-unibuild

DESCRIPTION="Monitor Firmware for ChromiumOS NPCX EC"
KEYWORDS="*"

RDEPEND="
	chromeos-base/libec:=
"

DEPEND="
	${RDEPEND}
"

get_target_boards() {
	EC_BOARDS=("helipilot")
}

src_configure() {
	sanitizers-setup-env
	default
}

src_compile() {
	cros-ec_set_build_env
	get_target_boards

	local target
	einfo "Building npcx_monitor.bin for targets: ${EC_BOARDS[*]}"
	for target in "${EC_BOARDS[@]}"; do
		emake V=1 BOARD="${target}" "build/${target}/chip/npcx/spiflashfw/npcx_monitor.bin"
	done
}

src_install() {
	local target
	for target in "${EC_BOARDS[@]}"; do
		insinto /usr/share/npcx
		newins "build/${target}/chip/npcx/spiflashfw/npcx_monitor.bin" "npcx_monitor_${target}.bin"
	done
}
