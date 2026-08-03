# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="6f77effc93fcae4eb7a08edcb086be1fadfa80f4"
CROS_WORKON_TREE="d99eb7ce0475c44158925c1e8ad9fffae663c216"
CROS_WORKON_PROJECT="chromiumos/platform/dev-util"
CROS_WORKON_LOCALNAME=("../platform/dev")
CROS_WORKON_SUBTREE="src/go.chromium.org/chromiumos/test/publish"

inherit cros-go cros-workon

DESCRIPTION="Publish server implementation for uploading test result artifacts to GCS bucket"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/dev-util/+/HEAD/src/go.chromium.org/chromiumos/test/publish"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

CROS_GO_WORKSPACE=(
	"${S}"
)

CROS_GO_BINARIES=(
	"go.chromium.org/chromiumos/test/publish/cmd/gcs-publish"
	"go.chromium.org/chromiumos/test/publish/cmd/tko-publish"
	"go.chromium.org/chromiumos/test/publish/cmd/rdb-publish"
	"go.chromium.org/chromiumos/test/publish/cmd/cpcon-publish"
)

CROS_GO_TEST=(
	"go.chromium.org/chromiumos/test/publish/cmd/..."
)

CROS_GO_VET=(
	"${CROS_GO_TEST[@]}"
)

DEPEND="
	dev-go/crypto
	dev-go/gcp
	dev-go/gcp-storage
	dev-go/grpc
	dev-go/infra-proto
	dev-go/luci-go
	dev-go/mock
	dev-go/protobuf
	dev-go/protobuf-legacy-api
	dev-util/cros-test-util
	dev-util/lro-server
	chromeos-base/cros-config-api
"
RDEPEND="${DEPEND}"

src_prepare() {
	# CGO_ENABLED=0 will make the executable statically linked.
	export CGO_ENABLED=0

	default
}
