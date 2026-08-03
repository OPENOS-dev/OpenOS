# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="9d6c703372686ff185400412a907be4313bac21f"
CROS_WORKON_TREE="6cce374cf0d47c3c2cba395590af13c246d934b7"
CROS_WORKON_PROJECT="chromiumos/platform/dev-util"
CROS_WORKON_LOCALNAME=("../platform/dev")
CROS_WORKON_SUBTREE="src/go.chromium.org/chromiumos/test/post_process"

inherit cros-go cros-workon

DESCRIPTION="Test finder for find tests that match the specified test suite tags"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/dev-util/+/HEAD/src/go.chromium.org/chromiumos/test/post_process"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

CROS_GO_WORKSPACE=(
	"${S}"
)
CROS_GO_BINARIES=(
	"go.chromium.org/chromiumos/test/post_process/cmd/post-process"
)

CROS_GO_TEST=(
	"go.chromium.org/chromiumos/test/post_process/cmd/post-process..."
)

CROS_GO_VET=(
	"${CROS_GO_TEST[@]}"
)

DEPEND="
	dev-go/genproto
	dev-go/gcp-bigquery
	dev-go/protobuf
	dev-go/protobuf-legacy-api
	chromeos-base/cros-config-api
	dev-util/cros-test
	dev-util/cros-test-util
"
RDEPEND="${DEPEND}"

src_prepare() {
	export CGO_ENABLED=0
	export GOPIE=0

	default
}
