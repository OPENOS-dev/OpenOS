# Copyright 2012 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="ded182ed0453784f9c1b20cdc58a68506d458684"
CROS_WORKON_TREE="a2297a0c43f7048dec6a1141b20dc73f2f8fbbd8"
CROS_WORKON_PROJECT="chromiumos/platform/inputcontrol"
CROS_WORKON_LOCALNAME="platform/inputcontrol"

inherit cros-workon

DESCRIPTION="A collection of utilities for configuring input devices"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/inputcontrol/"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

RDEPEND="app-arch/gzip"
DEPEND=""
