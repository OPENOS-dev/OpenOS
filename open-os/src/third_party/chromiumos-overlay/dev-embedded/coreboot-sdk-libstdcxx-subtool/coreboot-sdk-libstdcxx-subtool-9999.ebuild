# Copyright 2025 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-subtool cros-workon

DESCRIPTION="Subtools config for coreboot-sdk libstdcxx packages."
HOMEPAGE="https://www.coreboot.org"

LICENSE="GPL-3 LGPL-3"
SLOT="0"
KEYWORDS="~*"

# A mapping of subtool package to the portage package that provides it.
declare -A PACKAGES=(
	["coreboot-sdk-libstdcxx-aarch64-elf"]="cross-embedded-aarch64-elf/coreboot-sdk-libstdcxx"
	["coreboot-sdk-libstdcxx-arm-eabi"]="cross-embedded-arm-eabi/coreboot-sdk-libstdcxx"
	["coreboot-sdk-libstdcxx-i386-elf"]="cross-embedded-i386-elf/coreboot-sdk-libstdcxx"
	["coreboot-sdk-libstdcxx-riscv64-elf"]="cross-embedded-riscv64-elf/coreboot-sdk-libstdcxx"
)

RDEPEND="${PACKAGES[*]}"

src_compile() {
	local name
	for name in "${!PACKAGES[@]}"; do
		sed -e "s/%NAME%/${name}/g" -e "s#%EBUILD%#${PACKAGES[${name}]}#g" \
			"${FILESDIR}/subtool.textproto" \
			> "${WORKDIR}/${name}_subtool.textproto" || die
	done
}

src_install() {
	local name
	for name in "${!PACKAGES[@]}"; do
		cros-subtool_src_install "${WORKDIR}/${name}_subtool.textproto"
	done
}
