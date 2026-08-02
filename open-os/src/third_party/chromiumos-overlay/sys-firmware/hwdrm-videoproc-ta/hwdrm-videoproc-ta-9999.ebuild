# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_SUBTREE="hwdrm-videoproc-ta"
CROS_WORKON_DESTDIR="${S}/platform2"

inherit cros-workon coreboot-sdk coreboot-sdk-ap-dependencies

DESCRIPTION="Trusted Application for HWDRM Video Processing for Op-Tee on ARM"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="~*"
CHIPSETS=(mt8195 mt8188 mt8196)
IUSE="
	${CHIPSETS[*]/#/optee_}
"

RDEPEND=""

DEPEND="
	${RDEPEND}
	sys-firmware/optee_os_tadevkit
"
BDEPEND=""

coreboot-sdk_enable aarch64-elf

src_configure() {
	local chipset
	for chipset in "${CHIPSETS[@]}"; do
		if use "optee_${chipset}"; then
			export PLATFORM="mediatek-${chipset}"
			break
		fi
	done
	[[ -n ${PLATFORM} ]] || die "unhandled chipset"
	export OPTEE_DIR="${SYSROOT}/build/share/optee"
	export CROSS_COMPILE64=${COREBOOT_SDK_PREFIX_arm64}
	export CROSS_COMPILE_core=${COREBOOT_SDK_PREFIX_arm64}
	export TA_DEV_KIT_DIR=${OPTEE_DIR}/export-ta_arm64
	export TA_OUTPUT_DIR="${WORKDIR}/out"

	# CFLAGS/CXXFLAGS/CPPFLAGS/LDFLAGS are set for userland, but those options
	# don't apply properly to firmware so unset them.
	unset CFLAGS CXXFLAGS CPPFLAGS LDFLAGS
}

src_compile() {
	emake -C "${S}/platform2/hwdrm-videoproc-ta"
}

src_install() {
	insinto /build/share/optee/tas
	doins "${WORKDIR}/out/ebb0fd23-257e-4cd4-82de-8833c3a12603.elf"
}
