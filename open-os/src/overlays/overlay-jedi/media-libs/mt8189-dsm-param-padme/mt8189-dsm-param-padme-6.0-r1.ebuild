# Copyright 2025 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="MT8189 CS35L51G DSM firmware and tuning files"
SRC_URI="
gs://chromeos-localmirror/distfiles/dsm-param-padme-${PV}.tar.xz
"

LICENSE="LICENSE.dsm"
SLOT="0"
KEYWORDS="-* arm64 arm"

S="${WORKDIR}"

src_install() {
	insinto /lib/firmware/cirrus

	doins cs35l41-dsp1-spk-prot-104315ee-left.bin
	doins cs35l41-dsp1-spk-cali-104315ee-left.bin
	doins cs35l41-dsp1-spk-prot-104315ee-right.bin
	doins cs35l41-dsp1-spk-cali-104315ee-right.bin
	doins cs35l41-dsp1-spk-prot-104315ee.wmfw
	# All speakers share the same extended firmware file (.wmfw)
	dosym ./cs35l41-dsp1-spk-prot-104315ee.wmfw /lib/firmware/cirrus/cs35l41-dsp1-spk-cali-104315ee.wmfw
}
