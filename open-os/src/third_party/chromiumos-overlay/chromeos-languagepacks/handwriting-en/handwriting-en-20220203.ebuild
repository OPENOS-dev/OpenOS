# Copyright 2024 The ChromiumOS Authors. All rights reserved.
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit dlc

DESCRIPTION="Handwriting en Language Pack for ChromiumOS"

# Clients of Language Packs (Handwriting) need to update this path when new
# versions are available.
SRC_URI="gs://chromeos-localmirror/distfiles/languagepack-handwriting-en-${PV}.tar.xz"


LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"

IUSE="dlc"
REQUIRED_USE="dlc"

# DLC variables.
# Allocate 4KB * 8750 = 35MB
DLC_PREALLOC_BLOCKS="8750"

# Enabled scaled design.
DLC_SCALED=true

S="${WORKDIR}"
src_unpack() {
	local archive="${SRC_URI##*/}"
	unpack ${archive}
}

src_install() {
	# Setup DLC paths. We don't need any subdirectory inside the DLC path.
	insinto "$(dlc_add_path /)"

	# Install handwriting models for en.
	doins compact.fst.local latin_indy.tflite latin_indy_conf.tflite
	doins latin_indy_seg.tflite qrnn.recospec.local

	# This command packages the files into a DLC.
	dlc_src_install
}
