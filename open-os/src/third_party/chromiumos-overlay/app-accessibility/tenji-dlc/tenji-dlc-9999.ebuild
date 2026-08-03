# Copyright 2026 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v3

EAPI=7

inherit cros-workon dlc

DESCRIPTION='Provides the WebAssembly (WASM) module for the Japanese Braille
(Tenji) library, enabling support within the ChromeVox screen reader on ChromeOS.
This DLC allows ChromeVox to perform efficient client-side translation between
Japanese text and Braille. The package contains the compiled Tenji WASM binary
and any necessary JavaScript wrapper files. Installing this DLC enhances the
accessibility of ChromeOS for Japanese language users who rely on Braille,
leveraging the performance and portability of WebAssembly.'
HOMEPAGE=""
SRC_URI=gs://chromeos-localmirror/distfiles/${PN}-0.0.24.tar.xz

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="~*"
# "cros_workon info" expects these variables to be set, so use the standard
# empty project.
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

# DLC variables.
# The total size of the Tenji DLC is ~16.5MB.
# DLC_PREALLOC_BLOCKS = (DLC_SIZE * 1.3) / 4000 = 5300.
DLC_PREALLOC_BLOCKS="5300"

# Feed through scaled design.
DLC_SCALED=true
DLC_PRELOAD=true

S="${WORKDIR}"

src_unpack() {
	local archive="${SRC_URI##*/}"
	unpack "${archive}"
}

src_install() {
	into "$(dlc_add_path /)"
	insinto "$(dlc_add_path /)"
	exeinto "$(dlc_add_path /)"
	doins -r "${S}"/*

	dlc_src_install
}
