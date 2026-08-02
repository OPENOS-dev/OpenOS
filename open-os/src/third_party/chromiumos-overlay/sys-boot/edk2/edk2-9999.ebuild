# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_PROJECT="chromiumos/third_party/edk2"
CROS_WORKON_LOCALNAME="edk2"

inherit cros-workon coreboot-sdk coreboot-sdk-ap-dependencies multiprocessing

DESCRIPTION="EDK II firmware development environment for the UEFI and PI specifications."
HOMEPAGE="https://github.com/tianocore/edk2"

LICENSE="BSD"
KEYWORDS="~*"

BDEPEND=""
RDEPEND=""
DEPEND=""

# EDK2 depends on OpenSSL, Brotli and Mipi-sys-t. EDK2 is building them from
# their source instead of linking against the static libraries built for target
# SoC. This means EDK2 may not be leveraging latest releases of the concerned
# components. But it is fine since it is an experimental payload that goes only
# into alt-fw FMAP section and does not show up during the verified boot flow.
SRC_URI="https://www.openssl.org/source/openssl-3.0.9.tar.gz"
SRC_URI+=" https://github.com/google/brotli/archive/f4153a09f87cbb9c826d8fc12c74642bb2d879ea.tar.gz -> brotli-20220110.tar.gz"
SRC_URI+="  https://github.com/MIPI-Alliance/public-mipi-sys-t/archive/370b5944c046bab043dd8b133727b2135af7747a.tar.gz -> public-mipi-sys-t-1.1-edk2.tar.gz"

BUILDTYPE=RELEASE # DEBUG or RELEASE

coreboot-sdk_enable i386-elf
coreboot-sdk_enable iasl

pkg_setup() {
	cros-workon_pkg_setup
}

src_unpack() {
	cros-workon_src_unpack

	unpack "openssl-3.0.9.tar.gz"
	rm -r "${S}/CryptoPkg/Library/OpensslLib/openssl" || die
	ln -sTf "${WORKDIR}"/openssl-* "${S}/CryptoPkg/Library/OpensslLib/openssl" || die

	unpack "brotli-20220110.tar.gz"
	for TARGET in "${S}/BaseTools/Source/C/BrotliCompress/brotli" \
			"${S}/MdeModulePkg/Library/BrotliCustomDecompressLib/brotli"; do
		rm -r "${TARGET}" || die
		ln -sTf "${WORKDIR}"/brotli-* "${TARGET}" || die
	done

	unpack "public-mipi-sys-t-1.1-edk2.tar.gz"
	rm -r "${S}/MdePkg/Library/MipiSysTLib/mipisyst" || die
	ln -sTf "${WORKDIR}"/public-mipi-sys-t* "${S}/MdePkg/Library/MipiSysTLib/mipisyst" || die
}

src_compile() {
	# shellcheck disable=SC1091
	. ./edksetup.sh

	cat "${COREBOOT_SDK_PREFIX}/share/edk2config/tools_def.txt" \
		>> Conf/tools_def.txt
	( cd BaseTools/Source/C && emake ARCH=X64 )
	export COREBOOT_SDK_PREFIX_arm COREBOOT_SDK_PREFIX_arm64 COREBOOT_SDK_PREFIX_x86_32 COREBOOT_SDK_PREFIX_x86_64
	build -a IA32 -a X64 -b "${BUILDTYPE}" -p UefiPayloadPkg/UefiPayloadPkg.dsc -t COREBOOT \
		-D BOOTLOADER=COREBOOT \
		-D CPU_TIMER_LIB_ENABLE=FALSE -D DISABLE_SERIAL_TERMINAL=TRUE \
		-D PS2_KEYBOARD_ENABLE=TRUE -n "$(makeopts_jobs)" || die
}

src_install() {
	insinto /firmware/tianocore
	doins "Build/UefiPayloadPkgX64/${BUILDTYPE}_COREBOOT/FV/UEFIPAYLOAD.fd"
}
