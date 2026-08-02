# Copyright 2019 The ChromiumOS Authors
# Distributed under the terms of the BSD license.

EAPI=7
CROS_WORKON_COMMIT="4c6f1d8b759cd2f4ebf8729977f88b1291d3da8c"
CROS_WORKON_TREE="bead31933295a4cd9ff7c977cc1379eaac723177"
CROS_WORKON_PROJECT="chromiumos/platform/touch_updater"
CROS_WORKON_LOCALNAME="platform/touch_updater"
CROS_WORKON_SUBTREE="common"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-workon tmpfiles

DESCRIPTION="Common shell libraries for touch firmware updater wrapper scripts"
HOMEPAGE="https://www.chromium.org/chromium-os"

LICENSE="BSD-Google"
SLOT="0/0"
KEYWORDS="*"

RDEPEND="
	chromeos-base/chromeos-config-tools
	chromeos-base/vboot_reference
	!<chromeos-base/touch_updater-0.0.1-r167
"

src_install() {
	dotmpfiles common/tmpfiles.d/*.conf

	insinto "/opt/google/touch/scripts"
	doins common/scripts/*.sh
}
