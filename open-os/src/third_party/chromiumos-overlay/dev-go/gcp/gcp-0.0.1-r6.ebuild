# Copyright 2022 The ChromiumOS Authors
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
	"cloud.google.com/go/civil"
	"cloud.google.com/go/internal"
	"cloud.google.com/go/internal/detect"
	"cloud.google.com/go/internal/fields"
	"cloud.google.com/go/internal/optional"
	"cloud.google.com/go/internal/pubsub"
	"cloud.google.com/go/internal/trace"
	"cloud.google.com/go/internal/testutil"
	"cloud.google.com/go/internal/uid"
	"cloud.google.com/go/internal/version"
	"cloud.google.com/go/httpreplay/..."
)

CROS_GO_TEST=(
	"${CROS_GO_PACKAGES[@]}"
)

# temporary cyclic dep workaround until we switch to modules mode
CROS_GO_SKIP_DEP_CHECK="1"

# No git repo for this so use empty-project.
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-go cros-workon

DESCRIPTION="Google Cloud Client Libraries for Go"
HOMEPAGE="https://code.googlesource.com/gocloud"
SRC_URI="$(cros-go_src_uri)"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE=""
RESTRICT="binchecks strip test"

DEPEND="
	dev-go/cmp
	dev-go/gapi
	dev-go/gcp-longrunning
	dev-go/genproto
	dev-go/net
	dev-go/protoc-gen-go-grpc
	dev-go/martian
	dev-go/gax
"
RDEPEND="
	${DEPEND}
	!dev-go/gcp-internal
	!dev-go/gcp-civil
	!<dev-go/gcp-pubsub-0.0.1-r2
"

src_unpack() {
	cros-go_src_unpack
}
