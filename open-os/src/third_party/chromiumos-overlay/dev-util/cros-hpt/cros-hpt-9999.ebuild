# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_PROJECT="chromiumos/platform/dev-util"
CROS_WORKON_LOCALNAME=("../platform/dev")
CROS_WORKON_SUBTREE="src/go.chromium.org/chromiumos/test/hpt src/go.chromium.org/chromiumos/test/publish src/go.chromium.org/chromiumos/test/util"

inherit cros-go cros-workon

DESCRIPTION="High Performance Tracing implementation for CFT"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/dev-util/+/HEAD/src/go.chromium.org/chromiumos/test/hpt"

LICENSE="BSD-Google"
KEYWORDS="~*"
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
