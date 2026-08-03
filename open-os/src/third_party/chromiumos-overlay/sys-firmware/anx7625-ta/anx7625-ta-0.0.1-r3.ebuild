# Copyright 2025 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="e60fab093309a96ac23d1f0969f8276a59279934"
CROS_WORKON_TREE="aefa75e640a78e6de93db32df30b1801dc958278"
CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_SUBTREE="anx7625-ta"
CROS_WORKON_DESTDIR="${S}/platform2"

inherit cros-workon coreboot-sdk coreboot-sdk-ap-dependencies

DESCRIPTION="Trusted Application for controlling ANX7625 for Op-Tee on ARM"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
CHIPSETS=(mt8196)
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
	emake -C "${S}/platform2/anx7625-ta"
}

src_install() {
	insinto /build/share/optee/tas
	doins "${WORKDIR}/out/9461d0ad-dc5a-48f5-96e4-c165c7389978.elf"
}
