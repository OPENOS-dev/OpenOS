# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

# Note: this ebuild template is for files that include a revision specifier
# (e.g. -r1) in the file name.

DESCRIPTION="RTS5453 Firmware retimer_jhl8040 Binary"
SRC_URI="gs://chromeos-localmirror/distfiles/${P}-${PR}.tar.xz"

LICENSE="Google-Partners-Website"
SLOT="0"
KEYWORDS="*"
IUSE=""

S="${WORKDIR}"

DEPEND=""
RDEPEND="${DEPEND}"

src_install() {
	local fw_major_ver_hex=$(printf '%02x' "$(echo ${PV} | cut -d. -f1)")
	local fw_minor_ver_hex=$(printf '%02x' "$(echo ${PV} | cut -d. -f2)")
	local fw_patch_ver_hex=$(printf '%02x' "$(echo ${PV} | cut -d. -f3)")
	local bf=rts5453_retimer_jhl8040.bin
	local hf=rts5453_retimer_jhl8040.hash

	echo -ne '\x'"${fw_major_ver_hex}" > "${hf}"
	echo -ne '\x'"${fw_minor_ver_hex}" >> "${hf}"
	echo -ne '\x'"${fw_patch_ver_hex}" >> "${hf}"
	insinto /firmware/rts5453
	newins "${hf}" "${hf}"
	newins "${P}-${PR}/rts5453_V${PV}.bin" "${bf}"
}
