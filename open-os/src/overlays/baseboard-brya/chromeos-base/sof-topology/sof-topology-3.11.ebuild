# Copyright 2021 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

DESCRIPTION="SOF topology files for Brya"
SRC_URI="gs://chromeos-localmirror/distfiles/${PN}-brya-${PV}.tar.xz"

LICENSE="SOF"
SLOT="0"
KEYWORDS="*"

S=${WORKDIR}/${PN}-brya-${PV}

src_install() {
	insinto /lib/firmware/intel/sof-tplg
	doins ./*.tplg
	insinto /lib/firmware/intel/sof-tplg/pdm1
	doins ./pdm1/*.tplg

	# Add symlinks for all tplg files to RPL ones with name replacing prefix to "sof-rpl"
	for TPLG_DIR in ./ ./pdm1/ ; do
		for TPLG_FPATH in "${TPLG_DIR}"*.tplg ; do
			dosym "$(basename "${TPLG_FPATH}")" /lib/firmware/intel/sof-tplg/"${TPLG_FPATH//sof-adl/sof-rpl}"
		done
	done

	# max98360a-cs42l42 shares the same tplg as max98360a-rt5682 for both ADL and RPL
	dosym ./sof-adl-max98360a-rt5682.tplg /lib/firmware/intel/sof-tplg/sof-adl-max98360a-cs42l42.tplg
	dosym ./sof-adl-max98360a-rt5682.tplg /lib/firmware/intel/sof-tplg/sof-rpl-max98360a-cs42l42.tplg

	# TODO(htcheong): rename the topology file directly on next release
	dosym ./sof-adl-sdw-max98373-rt5682.tplg /lib/firmware/intel/sof-tplg/sof-adl-sdw-max98373-rt5682-4ch.tplg
	dosym ./sof-adl-sdw-max98373-rt5682.tplg /lib/firmware/intel/sof-tplg/sof-rpl-sdw-max98373-rt5682-4ch.tplg
}
