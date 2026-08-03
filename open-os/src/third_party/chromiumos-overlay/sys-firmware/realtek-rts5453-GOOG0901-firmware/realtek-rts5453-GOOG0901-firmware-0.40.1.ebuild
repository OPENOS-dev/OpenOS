# Copyright 2025 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="RTS5453 Firmware GOOG0901 Binary"
SRC_URI="gs://chromeos-localmirror/distfiles/${P}.tar.xz"

LICENSE="Google-Partners-Website"
SLOT="0"
KEYWORDS="*"
IUSE=""

S="${WORKDIR}"

DEPEND=""
RDEPEND="${DEPEND}"

# Here are the steps to uprev the RTS5453 firmware.
# 1) Tarball the firmware binary using XZ, including the right directory.
#    ex: tar -cJf realtek-rts5453-GOOG0901-firmware-${PV}.tar.xz \
#          realtek-rts5453-GOOG0901-firmware-${PV}/rts5453_V${PV}.bin
#    Note: Package Version follows the firmware version format - major.minor.patch
# 2) Then upload it at https://pantheon.corp.google.com/storage/browser/chromeos-localmirror/distfiles
# 3) On the uploaded file, click the three-dot-menu, "Edit Permissions", and
#    add a new entry for Public "allUsers" with Reader permission.
# 4) Finally run `ebuild realtek-rts5453-GOOG0901-firmware-${PV}.ebuild manifest`
src_install() {
	local fw_major_ver_hex=$(printf '%02x' "$(echo ${PV} | cut -d. -f1)")
	local fw_minor_ver_hex=$(printf '%02x' "$(echo ${PV} | cut -d. -f2)")
	local fw_patch_ver_hex=$(printf '%02x' "$(echo ${PV} | cut -d. -f3)")
	local bf=rts5453_GOOG0901.bin
	local hf=rts5453_GOOG0901.hash

	echo -ne '\x'"${fw_major_ver_hex}" > "${hf}"
	echo -ne '\x'"${fw_minor_ver_hex}" >> "${hf}"
	echo -ne '\x'"${fw_patch_ver_hex}" >> "${hf}"
	insinto /firmware/rts5453
	newins "${hf}" "${hf}"
	newins "${P}/rts5453_V${PV}.bin" "${bf}"
}
