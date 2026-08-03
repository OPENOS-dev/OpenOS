# Copyright 2022 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="987ab0566355a6acab5e4e0fc1a64fa584974208"
CROS_WORKON_TREE=("4c68ad892925860a2df476242d670286b0803e8d" "eb0f019105c2a652a251b01ce043aacc4a6403c9")
CROS_WORKON_PROJECT="chromiumos/platform/dev-util"
CROS_WORKON_LOCALNAME=("../platform/dev")
CROS_WORKON_SUBTREE="src/go.chromium.org/chromiumos/test src/chromiumos/test"

inherit cros-go cros-workon

DESCRIPTION="cros-servod service for CFT"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/dev-util/+/HEAD/src/go.chromium.org/chromiumos/test/servod/cmd"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

CROS_GO_WORKSPACE=(
	"${S}"
)

CROS_GO_BINARIES=(
	"go.chromium.org/chromiumos/test/servod/cmd/cros-servod"
)

CROS_GO_TEST=(
	"go.chromium.org/chromiumos/test/servod/cmd/servodserver"
)

CROS_GO_VET=(
	"${CROS_GO_TEST[@]}"
)

DEPEND="
	dev-util/cros-dut
	dev-util/lro-server
	dev-util/lroold-server
	dev-go/genproto
	dev-go/luci-go
	dev-go/mock
	dev-go/protobuf
	dev-go/protobuf-legacy-api
	chromeos-base/cros-config-api
"
RDEPEND="${DEPEND}"

src_prepare() {
	# CGO_ENABLED=0 will make the executable statically linked.
	export CGO_ENABLED=0

	default
}
