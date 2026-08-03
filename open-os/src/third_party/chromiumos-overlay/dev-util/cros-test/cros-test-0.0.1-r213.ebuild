# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="05276fed7048278c54a74f593639dd4a3d5259f9"
CROS_WORKON_TREE=("9e2536252395eb4644f1ea01c758c6bee80da2bb" "1e2cf58256b13aa9a8b3d1b3428e9bec6f4c7208")
CROS_WORKON_PROJECT="chromiumos/platform/dev-util"
CROS_WORKON_LOCALNAME=("../platform/dev")
CROS_WORKON_SUBTREE="src/go.chromium.org/chromiumos/test/execution src/go.chromium.org/chromiumos/test/util"

inherit cros-go cros-workon

DESCRIPTION="Test execution server for running tests and capturing results"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/dev-util/+/HEAD/src/go.chromium.org/chromiumos/test/execution"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

CROS_GO_VERSION="${PF}"

CROS_GO_BINARIES=(
	"go.chromium.org/chromiumos/test/execution/cmd/cros-test"
)

CROS_GO_PACKAGES=(
	"go.chromium.org/chromiumos/test/execution/errors/..."
)

CROS_GO_TEST=(
	"go.chromium.org/chromiumos/test/execution/cmd/cros-test/..."
)

CROS_GO_VET=(
	"${CROS_GO_TEST[@]}"
)

DEPEND="
	chromeos-base/tast-cmd:=
	chromeos-base/tast-proto
	dev-go/cmp
	dev-go/goconvey
	dev-go/grpc
	dev-go/luci-go
	dev-go/protobuf-legacy-api
	dev-go/yaml:3
	dev-util/lro-server
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
