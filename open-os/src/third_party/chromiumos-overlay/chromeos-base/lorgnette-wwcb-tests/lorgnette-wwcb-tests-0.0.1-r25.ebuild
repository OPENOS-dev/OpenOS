# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2.

EAPI=7

CROS_WORKON_COMMIT="8df74694bbbc31a20c1bee46e3f4e29171dec9ed"
CROS_WORKON_TREE="f74bccfa5c25a974dc605abb146834c586836f25"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_SUBTREE="lorgnette/hwtests"

CROS_GO_WORKSPACE="${S}/lorgnette/hwtests"

CROS_GO_TEST=(
	"go.chromium.org/chromiumos/scanning/hwtests"
	"go.chromium.org/chromiumos/scanning/utils"
)

CROS_GO_BINARIES=(
	"go.chromium.org/chromiumos/scanning/scripts/test_scan_source"
	"go.chromium.org/chromiumos/scanning/scripts/test_scanner_capabilities"
)

inherit cros-go cros-workon

DESCRIPTION="Works with Chromebook test suite for scanners"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/lorgnette/hwtests"

LICENSE="BSD-Google"
KEYWORDS="*"
SLOT="0/0"

DEPEND="
	dev-go/cmp
"
RDEPEND="
	chromeos-base/lorgnette_cli
"
