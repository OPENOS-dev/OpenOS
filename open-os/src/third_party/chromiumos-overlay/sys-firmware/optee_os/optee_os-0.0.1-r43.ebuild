# Copyright 2022 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT=(
	"1e22ba308960f3baf24828557e7addc18805bf9b" # chromiumos/third_party/OP-TEE/optee_os
	"8db0bb72670941bffd966c7d43dd3dde86df3341" # chromeos/vendor/mtk-dramk
)
CROS_WORKON_TREE=(
	"58e4bdcca5e0cb4c38fda2d6cd0c7cb705b8ce18" # chromiumos/third_party/OP-TEE/optee_os
	"755249b670d8792f24100583be1abfb9e2789c00" # chromeos/vendor/mtk-dramk
)

inherit cros-constants

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
CROS_WORKON_MANUAL_UPREV="1"

PYTHON_COMPAT=( python3_11 )

inherit python-any-r1 cros-workon coreboot-sdk coreboot-sdk-ap-dependencies

DESCRIPTION="Op-Tee Secure OS"
HOMEPAGE="https://www.github.com/OP-TEE/optee_os"

LICENSE="BSD"
KEYWORDS="*"
CHIPSETS=(mt8195 mt8188 mt8196)
IUSE="
	${CHIPSETS[*]/#/optee_}
	internal
	mediatek_cpu
"

# Make sure we don't use SDK gcc anymore.
REQUIRED_USE="
	^^ ( ${CHIPSETS[*]/#/optee_} )
"

DEPEND="
	chromeos-base/hwsec-optee-ta:=
	sys-firmware/anx7625-ta:=
	sys-firmware/hdcp-prov4-ta:=
	sys-firmware/hwdrm-videoproc-ta:=
	sys-firmware/mtk-optee-os-ta-bins-chromeos:=
	sys-firmware/optee-oemcrypto-ta:=
"

BDEPEND="
	$(python_gen_any_dep '
		dev-python/pyelftools[${PYTHON_USEDEP}]
	')
"

coreboot-sdk_enable aarch64-elf

python_check_deps() {
	python_has_version -b "dev-python/pyelftools[${PYTHON_USEDEP}]"
}

src_prepare() {
	use internal && use mediatek_cpu && PATCHES+=( dramk/patches/*.patch )
	default
}

src_configure() {
	python_setup

	local chipset
	for chipset in "${CHIPSETS[@]}"; do
		if use "optee_${chipset}"; then
			export PLATFORM="mediatek-${chipset}"
			export MTK_CHIP_NAME="${chipset}"
			break
		fi
	done
	[[ -n "${PLATFORM}" ]] || die "unhandled chipset"
	export CROSS_COMPILE64=${COREBOOT_SDK_PREFIX_arm64}
	export OPTEE_PATH="${S}"
	export O="${WORKDIR}/out"
	export CFG_ARM64_core="y"
	export DEBUG="0"
	export ARCH="arm"
	# TODO(b/249834721): Set this back to zero for production release.
	export CFG_TEE_CORE_LOG_LEVEL="2"
	export CFG_CBMEM_CONSOLE="y"

	export CFG_CORE_ASLR="n"
	export CFG_UART_ENABLE="n"
	export CFG_DRAM_SIZE="0x200000000"
	if use "optee_mt8196"; then
		export CFG_TZDRAM_START="0x80500000"
		export CFG_TZDRAM_SIZE="0x01400000"
		export CFG_CORE_ASYNC_NOTIF_GIC_INTID="987"
	else
		export CFG_TZDRAM_START="0x43000000"
		export CFG_TZDRAM_SIZE="0x01400000"
		export CFG_CORE_ASYNC_NOTIF_GIC_INTID="579"
	fi
	export CFG_TEE_RAM_VA_SIZE="0x01000000"
	export FBSIZE="0x03200000"
	export CFG_CORE_HEAP_SIZE="1048576"
	export CFG_STACK_THREAD_EXTRA="10240"
	export CFG_NUM_THREADS="8"
	export CFG_WITH_USER_TA="y"
	export CFG_CORE_ASYNC_NOTIF="y"
	export CFG_ENABLE_GROUP1S="y"
	export CFG_CACHE_API="y"
	export CFG_RES_VA_FOR_VIRTMAP="y"
	export CFG_DT="y"
	export CFG_MAP_EXT_DT_SECURE="y"
	export CFG_WIDEVINE_PTA="y"
	export CFG_WIDEVINE_HUK="y"

	export CFG_CORE_RESERVED_SHM="n"
	export CFG_RESERVED_VASPACE_SIZE="0x35200000"
	export CFG_WITH_STATS="y"

	# Link in all the MTK specific libraries. We use the whole-archive option to
	# forcibly insert them because there is no dependency on the libs, they all
	# register components via static initialization.
	export LDADD="--whole-archive ${SYSROOT}/build/share/mtk-optee-os-ta-bins-chromeos/${chipset}/lib/* --no-whole-archive"

	# TODO(b/473876594): Fix the sections so that there are no RWX segments
	export LDADD+=" --no-warn-rwx-segments"

	# Include all TAs as early TAs and prevent them from being loaded from the
	# filesystem.
	# a92d116c-ce27-4917-b30c-4a416e2d9351 - OEMCrypto
	# ebb0fd23-257e-4cd4-82de-8833c3a12603 - HWDRM videoproc
	# 0feb839c-ee25-4920-8ee3-ac8daa860d3b - HDCP provisioning
	# ed800e33-3c58-4cae-a7c0-fd160e35e00d - HwSec
	# 99975014-3c7c-54ea-8487-a80d215ea92c - HDCP (MTK)
	# 9461d0ad-dc5a-48f5-96e4-c165c7389978 - ANX7625 (Analogix)
	export EARLY_TA_PATHS="\
		${SYSROOT}/build/share/optee/tas/a92d116c-ce27-4917-b30c-4a416e2d9351.elf \
		${SYSROOT}/build/share/optee/tas/ebb0fd23-257e-4cd4-82de-8833c3a12603.elf \
		${SYSROOT}/build/share/optee/tas/0feb839c-ee25-4920-8ee3-ac8daa860d3b.elf \
		${SYSROOT}/build/share/optee/tas/ed800e33-3c58-4cae-a7c0-fd160e35e00d.elf \
		${SYSROOT}/build/share/mtk-optee-os-ta-bins-chromeos/${chipset}/ta/99975014-3c7c-54ea-8487-a80d215ea92c.elf \
		${SYSROOT}/build/share/optee/tas/9461d0ad-dc5a-48f5-96e4-c165c7389978.elf \
	"
	export CFG_SECSTOR_TA="n"
	export CFG_REE_FS_TA="n"
	export CFG_EARLY_TA="y"
	export CFG_WARN_INSECURE="n"

	# CFLAGS/CXXFLAGS/CPPFLAGS/LDFLAGS are set for userland, but those options
	# don't apply properly to firmware so unset them.
	unset CFLAGS CXXFLAGS CPPFLAGS LDFLAGS
}

src_compile() {
	emake ta-targets=ta_arm64 all

	# Concatenate the header and pager, this is the format we use from the kernel
	# to send to TF-A to load Op-Tee via an SMC.
	cat "${WORKDIR}/out/core/tee-header_v2.bin" \
		"${WORKDIR}/out/core/tee-pager_v2.bin" \
		> "${WORKDIR}/out/tee.bin"
}

src_install() {
	# Copy the Op-Tee ELF file for inclusion as firmware in the rootfs.
	insinto /lib/firmware/optee
	doins "${WORKDIR}/out/tee.bin"
}
