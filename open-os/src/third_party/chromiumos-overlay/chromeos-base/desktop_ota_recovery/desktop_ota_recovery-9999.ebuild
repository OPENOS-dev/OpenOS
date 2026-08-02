# Copyright 2025 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk desktop_ota_recovery .gn"

PLATFORM_SUBDIR="desktop_ota_recovery"

inherit cros-workon platform

DESCRIPTION="The desktop ota recovery main logic."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/desktop_ota_recovery/"
KEYWORDS="~*"
LICENSE="BSD-Google"
