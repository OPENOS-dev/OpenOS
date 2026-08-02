# Copyright 2019 The ChromiumOS Authors
# This file distributed under the terms of the BSD license.

EAPI="7"

CROS_WORKON_COMMIT="ffe3794c3fd862832dcfb828232de1bcdd790221"
CROS_WORKON_TREE="def62d78dbb7733302b0cd4c771d3b7a5af301d6"
CROS_WORKON_PROJECT="chromiumos/third_party/novatek-tcon-fw-update-tool"
CROS_WORKON_LOCALNAME="novatek-tcon-fw-update-tool"

inherit cros-common.mk cros-workon

DESCRIPTION="Novatek TCON Firmware Updater"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/third_party/novatek-tcon-fw-update-tool/"

LICENSE="BSD"
SLOT="0"
KEYWORDS="*"

src_install() {
	dosbin "${OUT}"/nvt-tcon-fw-updater
}
