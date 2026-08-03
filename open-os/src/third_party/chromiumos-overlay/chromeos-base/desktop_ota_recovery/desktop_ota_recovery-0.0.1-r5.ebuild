# Copyright 2025 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "2e5011ee083014e458ce93c814e53fe1083a9f93" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk desktop_ota_recovery .gn"

PLATFORM_SUBDIR="desktop_ota_recovery"

inherit cros-workon platform

DESCRIPTION="The desktop ota recovery main logic."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/desktop_ota_recovery/"
KEYWORDS="*"
LICENSE="BSD-Google"
