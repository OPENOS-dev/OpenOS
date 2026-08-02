# Copyright 2026 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="TI TPS6699x Firmware Binary"
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
	local bf=tps6699x_GOOG0K00.bin
	local hf=tps6699x_GOOG0K00.hash

	# Build a hash file containing the 3 version bytes (major-minor-patch)
	# followed by an 8-byte configuration identifier bundled in the
	# package tarball.
	{
		echo -ne '\x'"${fw_major_ver_hex}"
		echo -ne '\x'"${fw_minor_ver_hex}"
		echo -ne '\x'"${fw_patch_ver_hex}"
	} >> "${hf}"
	cat "${P}/config_name.bin" >> "${hf}"

	insinto /firmware/tps6699x
	newins "${hf}" "${hf}"
	newins "${P}/tps6699x_${PV}.bin" "${bf}"
}
