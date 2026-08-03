# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit dlc

# Version 1.X.Y contains only IPAmjMincho version X.Y.
# Versions 2+ may include more fonts.

# Remove the leading `1.` as well as the middle `.`.
IPA_VER=$(ver_rs 1- '' "$(ver_cut 2-)")
MY_P="ipamjm${IPA_VER//.}"

DESCRIPTION="Chromium OS language pack for extra Japanese fonts"
HOMEPAGE="https://moji.or.jp/mojikiban/font/"
# The second-last path segment may need to be updated when the version number
# changes.
SRC_URI="https://dforest.watch.impress.co.jp/library/i/ipamjfont/10750/${MY_P}.zip"

LICENSE="IPAfont"
SLOT="0"
KEYWORDS="*"
IUSE="dlc"
REQUIRED_USE="dlc"

BDEPEND="app-arch/unzip"

# The compressed .zip containing IPAmjMincho 006.01 is ~30 MiB, so preallocate
# 4 KiB * 10240 = 40 MiB.
DLC_PREALLOC_BLOCKS="10240"
# Enabled scaled design.
DLC_SCALED=true

S="${WORKDIR}"

src_install() {
	insinto "$(dlc_add_path /)"
	doins ipamjm.ttf
	dlc_src_install
}
