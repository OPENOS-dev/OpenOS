# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="d2d95e8af89939f893b1443135497c1f5572aebc"
CROS_WORKON_TREE="776139a53bc86333de8672a51ed7879e75909ac9"
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-workon

DESCRIPTION="List of packages updated by the SDK subtools builder before it
looks for subtool definitions to bundle and upload."
HOMEPAGE="https://www.chromium.org/chromium-os/developer-library/guides/portage/subtools-builder/"

LICENSE="metapackage"
SLOT="0"
KEYWORDS="*"
IUSE=""

# This package just depends on virtual/target-chromium-os-sdk-subtools (for
# public packages), and a virtual overrides this package to depend on
# virtual/target-chrome-os-sdk-subtools (for private packages) as well.  You'll
# want to add your package to one of those virtuals.
RDEPEND="virtual/target-chromium-os-sdk-subtools"

DEPEND=""
