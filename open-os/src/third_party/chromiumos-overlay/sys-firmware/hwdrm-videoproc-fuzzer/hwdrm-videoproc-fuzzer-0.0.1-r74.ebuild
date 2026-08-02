# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "10bb44d3837f734024d47402d4f353b5d7e077a8" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_SUBTREE="hwdrm-videoproc-ta"
CROS_WORKON_DESTDIR="${S}/platform2"
CROS_WORKON_SUBTREE="common-mk hwdrm-videoproc-ta .gn"

PLATFORM_SUBDIR="hwdrm-videoproc-ta/fuzzer"

inherit cros-fuzzer cros-sanitizers cros-workon platform

DESCRIPTION="Fuzzer for HWDRM Video Processing code for Op-Tee on ARM"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE=""

RDEPEND=""
DEPEND="${RDEPEND}"

src_install() {
	local fuzzer_component_id="168352"  # ChromeOS>Platform>Graphics>Video
	platform_fuzzer_install "${S}"/../OWNERS "${OUT}"/h264_parser_fuzzer \
		--comp "${fuzzer_component_id}"
}
