# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="53192364919227d79c78a26f3bbf94a77641be9c"
CROS_WORKON_TREE="b875c901460e6de93b3c0fac9f69f8a1e0912d5b"
CROS_WORKON_PROJECT="chromiumos/platform/dev-util"
CROS_WORKON_LOCALNAME=("../platform/dev")
CROS_WORKON_SUBTREE="src/go.chromium.org/chromiumos/test/pre_process"

inherit cros-go cros-workon

DESCRIPTION="Test finder for find tests that match the specified test suite tags"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/dev-util/+/HEAD/src/go.chromium.org/chromiumos/test/pre_process"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

CROS_GO_WORKSPACE=(
	"${S}"
)
CROS_GO_BINARIES=(
	"go.chromium.org/chromiumos/test/pre_process/cmd/pre-process"
)

CROS_GO_TEST=(
	"go.chromium.org/chromiumos/test/pre_process/cmd/pre-process..."
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
	# Disable cgo and PIE on building Tast binaries. See:
	# https://crbug.com/976196
	# https://github.com/golang/go/issues/30986#issuecomment-475626018
	export CGO_ENABLED=0
	export GOPIE=0

	default
}
