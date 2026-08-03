# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="05276fed7048278c54a74f593639dd4a3d5259f9"
CROS_WORKON_TREE=("e5f30b134cf23eb554ed044c3634e2ede137842f" "d99eb7ce0475c44158925c1e8ad9fffae663c216" "1e2cf58256b13aa9a8b3d1b3428e9bec6f4c7208")
CROS_WORKON_PROJECT="chromiumos/platform/dev-util"
CROS_WORKON_LOCALNAME=("../platform/dev")
CROS_WORKON_SUBTREE="src/go.chromium.org/chromiumos/test/hpt src/go.chromium.org/chromiumos/test/publish src/go.chromium.org/chromiumos/test/util"

inherit cros-go cros-workon

DESCRIPTION="High Performance Tracing implementation for CFT"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/dev-util/+/HEAD/src/go.chromium.org/chromiumos/test/hpt"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

CROS_GO_WORKSPACE=(
	"${S}"
)

CROS_GO_BINARIES=(
	"go.chromium.org/chromiumos/test/hpt/cros-hpt"
)

CROS_GO_TEST=(
	"go.chromium.org/chromiumos/test/hpt/cros-hpt/..."
)

CROS_GO_VET=(
	"${CROS_GO_TEST[@]}"
)

DEPEND="
	dev-go/gcp-pubsub
	dev-util/cros-publish
	dev-util/cros-test
	dev-util/hdctools
	dev-go/mock
	dev-go/protobuf
	chromeos-base/cros-config-api
	dev-go/gcp-storage
	dev-go/luci-go
"

src_prepare() {
	# CGO_ENABLED=0 will make the executable statically linked.
	export CGO_ENABLED=0

	default
}
