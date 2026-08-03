# Copyright 2018 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v3
#
# Local fork with Chrome-specific port:
# https://chromium.googlesource.com/chromiumos/third_party/espeak-ng
# See README.chrome in the "chrome" branch for details.

EAPI="7"

DESCRIPTION="Text-to-speech engine"
HOMEPAGE="https://github.com/espeak-ng/espeak-ng"
SRC_URI="gs://chromeos-localmirror/distfiles/${P}.tar.xz"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="*"
IUSE=""

S="${WORKDIR}"

src_install() {
	# eSpeakNG Manifest V2 Deployment
	insinto /usr/share/chromeos-assets/speech_synthesis/espeak-ng
	doins ./espeak-ng-mv2/*.{png,js,json,css,html}

	insinto /usr/share/chromeos-assets/speech_synthesis/espeak-ng/js
	doins ./espeak-ng-mv2/js/*.{js,data,wasm}

	# **NEW** eSpeakNG Manifest V3 Deployment
	insinto /usr/share/chromeos-assets/speech_synthesis/espeak-ng-mv3
	doins ./espeak-ng-mv3/*.{png,js,json,css,html}

	insinto /usr/share/chromeos-assets/speech_synthesis/espeak-ng-mv3/js
	doins ./espeak-ng-mv3/js/*.{js,data,wasm}
}
