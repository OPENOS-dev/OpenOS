# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

# This ebuild is upreved via PuPR, so disable the normal uprev process for
# cros-workon ebuilds.
CROS_WORKON_COMMIT="d2d95e8af89939f893b1443135497c1f5572aebc"
CROS_WORKON_TREE="776139a53bc86333de8672a51ed7879e75909ac9"
CROS_WORKON_MANUAL_UPREV="1"
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-workon

# This ebuild reads the EC token database from the public mirror and installs
# it into driver accessible directory. The database is automatically updated.
if [[ "${PV}" == "9999" ]]; then
	# Can be manually set to the latest version, but that cannot be committed.
	SRC_URI=""
else
	SRC_URI="gs://chromeos-localmirror/distfiles/${P}.bin"
fi

DESCRIPTION="ChromeOS EC Token Database for all supported boards"
LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"

src_install() {
	# Install into /usr/share/cros_ec/tokens.bin
	insinto /usr/share/cros_ec
	newins "${DISTDIR}/${P}.bin" "tokens.bin"
}
