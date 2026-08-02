# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT=("e67643c64a105f6f744b007eb857f381ace07e8e" "994a857415f55e5c280e4f54645ba6d794c400f9")
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "053efb1147e4a19c64ccfbfe000531ab8e6dbe55" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "d69e65546eae1a8c5d4d994a154815cae1efacb2")
CROS_WORKON_LOCALNAME=(
	"platform2"
	"chromium/src/media/midi"
)
CROS_WORKON_PROJECT=(
	"chromiumos/platform2"
	"chromium/src/media/midi"
)
CROS_WORKON_DESTDIR=(
	"${S}/platform2"
	"${S}/platform2/media/midi"
)
CROS_WORKON_EGIT_BRANCH="main"
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_SUBTREE=(
	"common-mk midis .gn"
	""
)

PLATFORM_SUBDIR="midis"

inherit cros-workon platform user

DESCRIPTION="MIDI Server for Chromium OS"
HOMEPAGE=""

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="+seccomp asan fuzzer"

COMMON_DEPEND="
	media-libs/alsa-lib:=
	chromeos-base/libbrillo:=[asan?,fuzzer?]
"

RDEPEND="${COMMON_DEPEND}"
DEPEND="
	${COMMON_DEPEND}
	chromeos-base/system_api:=
"
BDEPEND="
	chromeos-base/minijail
"

src_install() {
	platform_src_install

	# fuzzer_component_id is unknown/unlisted
	platform_fuzzer_install "${S}"/OWNERS "${OUT}"/midis_seq_handler_fuzzer
}

pkg_preinst() {
	enewuser midis
	enewgroup midis
}
