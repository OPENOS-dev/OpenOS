# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_PROJECT="chromiumos/platform/dev-util"
CROS_WORKON_LOCALNAME=("../platform/dev")
CROS_WORKON_SUBTREE="src/go.chromium.org/chromiumos/test/provision"

inherit cros-go cros-workon

DESCRIPTION="Provision server implementation for installing CrOS on a test device"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/dev-util/+/HEAD/src/go.chromium.org/chromiumos/test/provision"

LICENSE="BSD-Google"
KEYWORDS="~*"
IUSE=""

CROS_GO_WORKSPACE=(
	"${S}"
)

CROS_GO_BINARIES=(
	"go.chromium.org/chromiumos/test/provision/v2/android-provision"
	"go.chromium.org/chromiumos/test/provision/v2/cros-provision"
	"go.chromium.org/chromiumos/test/provision/v2/vm-provision"
)

CROS_GO_TEST=(
	"go.chromium.org/chromiumos/test/provision/cmd/..."
	"go.chromium.org/chromiumos/test/provision/v2/android-provision/..."
	"go.chromium.org/chromiumos/test/provision/v2/cros-provision/..."
	"go.chromium.org/chromiumos/test/provision/v2/vm-provision"
)

CROS_GO_VET=(
	"${CROS_GO_TEST[@]}"
)

DEPEND="
	dev-util/cros-test
	dev-util/cros-test-util
	dev-util/lro-server
	dev-util/lroold-server
	dev-go/assertions
	dev-go/genproto
	dev-go/go-gls
	dev-go/goconvey
	dev-go/luci-go
	dev-go/mock
	dev-go/opencensus
	dev-go/protobuf
	dev-go/protobuf-legacy-api
	chromeos-base/cros-config-api
	dev-go/gcp-pubsub
	dev-go/gcp-storage
"
RDEPEND="
	${DEPEND}
	!dev-util/vm-provision
	!dev-util/android-provision
"

src_prepare() {
	# CGO_ENABLED=0 will make the executable statically linked.
	export CGO_ENABLED=0

	default
}
