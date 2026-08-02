# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

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
KEYWORDS="~*"
IUSE=""

RDEPEND=""
DEPEND="${RDEPEND}"

src_install() {
	local fuzzer_component_id="168352"  # ChromeOS>Platform>Graphics>Video
	platform_fuzzer_install "${S}"/../OWNERS "${OUT}"/h264_parser_fuzzer \
		--comp "${fuzzer_component_id}"
}
