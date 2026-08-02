# Copyright 2022 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_REPO=(
	"${CROS_GIT_HOST_URL}"
	"${CROS_GIT_INT_HOST_URL}"
)
CROS_WORKON_PROJECT=(
	"chromiumos/third_party/OP-TEE/optee_os"
	"chromeos/vendor/mtk-dramk"
)
CROS_WORKON_LOCALNAME=(
	"optee_os"
	"../partner_private/mediatek/dramk"
)
CROS_WORKON_OPTIONAL_CHECKOUT=(
	"true"
	"use internal && use mediatek_cpu"
)
CROS_WORKON_DESTDIR=(
	"${S}"
	"${S}/dramk"
)

# This is needed so the uprev bot doesn't complain that the dramk directory is
# not available.
CROS_WORKON_MANUAL_UPREV="1"

inherit cros-workon coreboot-sdk coreboot-sdk-ap-dependencies

DESCRIPTION="Op-Tee Secure OS TA Dev Kit"
HOMEPAGE="https://www.github.com/OP-TEE/optee_os"

LICENSE="BSD"
KEYWORDS="~*"
CHIPSETS=(mt8195 mt8188 mt8196)
IUSE="
	${CHIPSETS[*]/#/optee_}
	internal
	mediatek_cpu
"

coreboot-sdk_enable aarch64-elf

src_prepare() {
	use internal && use mediatek_cpu && PATCHES+=( dramk/patches/*.patch )
	default
}

src_configure() {
	local chipset
	for chipset in "${CHIPSETS[@]}"; do
		if use "optee_${chipset}"; then
			export PLATFORM="mediatek-${chipset}"
			export MTK_CHIP_NAME="${chipset}"
			break
		fi
	done
	[[ -n ${PLATFORM} ]] || die "unhandled chipset"
	export CROSS_COMPILE64=${COREBOOT_SDK_PREFIX_arm64}
	export OPTEE_PATH="${S}"
	export O="${WORKDIR}/out"
	export CFG_ARM64_core="y"
	export DEBUG="0"
	export CFG_EARLY_TA="y"
	export ARCH="arm"

	# CFLAGS/CXXFLAGS/CPPFLAGS/LDFLAGS are set for userland, but those options
	# don't apply properly to firmware so unset them.
	unset CFLAGS CXXFLAGS CPPFLAGS LDFLAGS
}

src_compile() {
	emake ta-targets=ta_arm64 ta_dev_kit
}

src_install() {
	# Install the dev kit used when building TAs (makefiles, header files, etc.).
	insinto /build/share/optee
	doins -r "${WORKDIR}/out/export-ta_arm64"
}
