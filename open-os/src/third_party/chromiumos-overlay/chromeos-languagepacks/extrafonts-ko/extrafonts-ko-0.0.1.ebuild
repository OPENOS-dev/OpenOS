# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit dlc

GULIM_COMMIT="3bb27b7504e45031f893e2c2b441b10741f6d222"
BATANG_COMMIT="be9f8bd78ee4e96e73e1c45447f46491ceb5d10e"

GULIM_MIRROR_FILENAME="${PN}-gulim-${GULIM_COMMIT}.ttc"
BATANG_MIRROR_FILENAME="${PN}-batang-${BATANG_COMMIT}.ttc"

DESCRIPTION="Chromium OS language pack for extra Korean fonts"
HOMEPAGE="https://github.com/googlefonts/gulim
	https://github.com/googlefonts/batang"
SRC_URI="https://github.com/googlefonts/gulim/raw/${GULIM_COMMIT}/fonts/ttc/gulim-regular.ttc -> ${GULIM_MIRROR_FILENAME}
	https://github.com/googlefonts/batang/raw/${BATANG_COMMIT}/fonts/ttc/batang-regular.ttc -> ${BATANG_MIRROR_FILENAME}"

LICENSE="OFL-1.1"
SLOT="0"
KEYWORDS="*"
IUSE="dlc"
REQUIRED_USE="dlc"

# A compressed .zip containing both fonts at v0.0.1 is ~15 MiB large, so
# preallocate 4 KiB * 5120 = 20 MiB.
DLC_PREALLOC_BLOCKS="5120"
# Enabled scaled design.
DLC_SCALED=true

S="${DISTDIR}"

src_unpack() {
	# The sources are .ttc files, so there is nothing to unpack.
	true;
}

src_install() {
	insinto "$(dlc_add_path /)"
	newins "${GULIM_MIRROR_FILENAME}" gulim-regular.ttc
	newins "${BATANG_MIRROR_FILENAME}" batang-regular.ttc
	dlc_src_install
}
