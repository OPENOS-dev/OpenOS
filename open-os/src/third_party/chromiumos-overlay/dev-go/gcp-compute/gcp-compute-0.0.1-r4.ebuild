# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2.

EAPI=7

# The dev-go/gcp* packages are versioned separately but all come from the same
# repo to simplify updates we set them all to be the same ebuild version but
# all should point to same git hash corresponding to a release and be update
# together
CROS_WORKON_COMMIT="d2d95e8af89939f893b1443135497c1f5572aebc"
CROS_WORKON_TREE="776139a53bc86333de8672a51ed7879e75909ac9"
CROS_GO_SOURCE="github.com/GoogleCloudPlatform/google-cloud-go:cloud.google.com/go 06a54a16a5866cce966547c51e203b9e09a25bc0"

CROS_GO_PACKAGES=(
	"cloud.google.com/go/compute/internal"
)

CROS_GO_TEST=(
	"${CROS_GO_PACKAGES[@]}"
)

inherit cros-go cros-workon

# No git repo for this so use empty-project.
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

DESCRIPTION="Google Cloud Client Libraries for Go Compute API"
HOMEPAGE="https://code.googlesource.com/gocloud"
SRC_URI="$(cros-go_src_uri)"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE=""
RESTRICT="binchecks strip test"

DEPEND="
	dev-go/net
	dev-go/gapi
"
RDEPEND="${DEPEND}"

src_unpack() {
	cros-go_src_unpack
}
