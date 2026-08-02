# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="85086c90284303c1fd60fdd6ac5e6751443e44d6"
CROS_WORKON_TREE="808ec3cc7d6b2804677d73774c2e5e788e2483e3"
CROS_WORKON_PROJECT="chromiumos/third_party/kernel"
CROS_WORKON_LOCALNAME="kernel/upstream"

# This must be inherited *after* EGIT/CROS_WORKON variables defined
inherit cros-workon cros-kernel

HOMEPAGE="https://www.chromium.org/chromium-os/chromiumos-design-docs/chromium-os-kernel"
DESCRIPTION="Chrome OS Linux Kernel latest upstream rc"
KEYWORDS="*"
