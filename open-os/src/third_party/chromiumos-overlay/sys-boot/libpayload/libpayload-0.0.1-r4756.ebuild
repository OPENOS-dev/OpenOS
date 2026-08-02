# Copyright 2012 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# Change this version number when any change is made to configs/files under
# libpayload and an auto-revbump is required.
# VERSION=REVBUMP-0.0.18

EAPI=7
CROS_WORKON_COMMIT=("6041432d8a86a143fe1d3fe3c0a88c905f011087" "906136da2c93e6dfe3c93763c93dfae59d24e54d" "0e55aeca367c80a0b8570535b4e50e18897eed9d")
CROS_WORKON_TREE=("8057ce3c229e2c0f6d1b3d01cfbc7d8cf61fc65f" "d393877053b416c12909fec869a5dd3b85f4fe6a" "b7d2e32f8e762c89664577f59b7a1a6ce77267ab" "94184418896be33ca57031e5fddd23bb8829e136" "ef4cae279a4da75f4f982873b1d2ca4e3d75b205" "d509ef449a0f7cc8a53159eb5cb9b9a3e4761f8d" "a737255cd6e3b6dbc4084583500ad07005237451")
CROS_WORKON_PROJECT=(
	"chromiumos/third_party/coreboot"
	"chromiumos/platform/vboot_reference"
	"platform/external/avb"
)
CROS_WORKON_EGIT_BRANCH=(
	"main"
	"main"
	""
)

DESCRIPTION="coreboot's libpayload library"
HOMEPAGE="http://www.coreboot.org"
LICENSE="GPL-2"
KEYWORDS="*"
IUSE="avb unibuild verbose ti50_onboard vboot_disable_cbfs_integration"

# No pre-unibuild boards build firmware on ToT anymore.  Assume
# unibuild to keep ebuild clean.
REQUIRED_USE="unibuild"

DEPEND="
	chromeos-base/chromeos-config:=
	dev-libs/nss:=
"

# While this package is never actually executed, we still need to specify
# RDEPEND. A binary version of this package could exist that was built using an
# outdated version of chromeos-config. Without the RDEPEND this stale binary
# package is considered valid by the package manager. This is problematic
# because we could have two binary packages installed having been build with
# different versions of chromeos-config. By specifying the RDEPEND we force
# the package manager to ensure the two versions use the same chromeos-config.
RDEPEND="${DEPEND}"

CROS_WORKON_LOCALNAME=(
	"coreboot"
	"../platform/vboot_reference"
	"../aosp/external/avb"
)

VBOOT_DESTDIR="${S}/3rdparty/vboot"
LIBAVB_DESTDIR="${S}/libavb"
CROS_WORKON_DESTDIR=(
	"${S}"
	"${S}/3rdparty/vboot"
	"${LIBAVB_DESTDIR}"
)

# commonlib, kconfig and xcompile are reused from coreboot.
# Everything else is not supposed to matter for
# libpayload.
CROS_WORKON_SUBTREE=(
	"payloads/libpayload src/commonlib util/kconfig util/xcompile"
	"Makefile firmware"
	"libavb"
)

# Disable binary checks for PIE and relative relocatons.
# Don't strip to ease remote GDB use (cbfstool strips final binaries anyway).
# This is only okay because this ebuild only installs files into
# ${SYSROOT}/firmware, which is not copied to the final system image.
RESTRICT="binchecks strip"

# Disable warnings for executable stack.
QA_EXECSTACK="*"

inherit cros-workon cros-toolchain-funcs coreboot-sdk coreboot-sdk-ap-dependencies cros-sanitizers cros-unibuild

LIBPAYLOAD_BUILD_NAMES=()
LIBPAYLOAD_BUILD_TARGETS=()

coreboot-sdk_enable i386-elf amd64
coreboot-sdk_enable x86_64-elf amd64
coreboot-sdk_enable aarch64-elf arm64
coreboot-sdk_enable arm-eabi arm
coreboot-sdk_enable iasl

src_unpack() {
	coreboot-sdk_src_unpack
	S+="/payloads/libpayload"
}

src_configure() {
	sanitizers-setup-env

	local name
	local target

	export GENERIC_COMPILER_PREFIX="invalid"

	while read -r name && read -r target; do
		LIBPAYLOAD_BUILD_NAMES+=("${name}")
		LIBPAYLOAD_BUILD_TARGETS+=("${target}")
	done < <(cros_config_host get-firmware-build-combinations libpayload)

	for target in "${LIBPAYLOAD_BUILD_TARGETS[@]}"; do
		if [[ ! -s "${FILESDIR}/configs/config.${target}" ]]; then
			die "libpayload config does not exist for ${target}"
		fi
	done
}

# build libpayload for a certain config
#   $1: path to the dotconfig
#   $2: path to the build directory
libpayload_compile() {
	local dotconfig="$(realpath "$1")"
	local objdir="$(realpath "$2")"
	local OPTS=(
		obj="${objdir}"
		DOTCONFIG="${dotconfig}"
		HOSTCC="$(tc-getBUILD_CC)"
		HOSTCXX="$(tc-getBUILD_CXX)"
		VBOOT_SOURCE="${VBOOT_DESTDIR}"
		LIBAVB_SRCDIR="${LIBAVB_DESTDIR}"
		USE_AVB="$(usev avb)"
	)
	use verbose && OPTS+=( "V=1" )

	yes "" | emake "${OPTS[@]}" oldconfig
	emake "${OPTS[@]}"
}

src_compile() {
	export CROSS_COMPILE_x86=${COREBOOT_SDK_PREFIX_x86_32}
	export CROSS_COMPILE_x64=${COREBOOT_SDK_PREFIX_x86_64}
	export CROSS_COMPILE_arm64=${COREBOOT_SDK_PREFIX_arm64}
	export CROSS_COMPILE_arm=${COREBOOT_SDK_PREFIX_arm}

	# we have all kinds of userland-cruft in CFLAGS that has no place in firmware.
	# coreboot ignores CFLAGS, libpayload doesn't, so prune it.
	unset CFLAGS

	local unique_targets=()
	readarray -t unique_targets \
		< <(printf "%s\n" "${LIBPAYLOAD_BUILD_TARGETS[@]}" | sort -u)

	local target
	local board_config
	local dotconfig
	local dotconfig_gdb
	for target in "${unique_targets[@]}"; do
		board_config="${FILESDIR}/configs/config.${target}"

		dotconfig="${T}/config.${target}"
		if use ti50_onboard; then
			echo "CONFIG_LP_CBFS_VERIFICATION=y" >> "${dotconfig}"
		fi
		if use ti50_onboard && ! use vboot_disable_cbfs_integration; then
			echo "CONFIG_LP_VBOOT_CBFS_INTEGRATION=y" >> "${dotconfig}"
		fi
		cat "${board_config}" >> "${dotconfig}"
		# In case "${board_config}" does not have a newline at the end
		echo >> "${dotconfig}"
		libpayload_compile "${dotconfig}" "${T}/${target}"

		# Build a second set of libraries with GDB support for developers
		dotconfig_gdb="${T}/config_gdb.${target}"
		cp "${dotconfig}" "${dotconfig_gdb}"
		echo "CONFIG_LP_REMOTEGDB=y" >> "${dotconfig_gdb}"
		libpayload_compile "${dotconfig_gdb}" "${T}/${target}.gdb"
	done
}

src_install() {
	local i
	local name
	local target
	local opts

	for i in "${!LIBPAYLOAD_BUILD_TARGETS[@]}"; do
		name="${LIBPAYLOAD_BUILD_NAMES[${i}]}"
		target="${LIBPAYLOAD_BUILD_TARGETS[${i}]}"
		opts=(
			"VBOOT_SOURCE=${VBOOT_DESTDIR}"
			"LIBAVB_SRCDIR=${LIBAVB_DESTDIR}"
			"USE_AVB=$(usev avb)"
		)

		emake obj="${T}/${target}" \
			DOTCONFIG="${T}/config.${target}" "${opts[@]}" \
			DESTDIR="${D}/firmware/${name}/libpayload" install

		emake obj="${T}/${target}.gdb" \
			DOTCONFIG="${T}/config_gdb.${target}" "${opts[@]}" \
			DESTDIR="${D}/firmware/${name}/libpayload.gdb" install
	done
}
