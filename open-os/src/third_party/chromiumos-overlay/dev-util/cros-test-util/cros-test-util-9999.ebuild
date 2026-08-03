# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_PROJECT="chromiumos/platform/dev-util"
CROS_WORKON_LOCALNAME=("../platform/dev")
CROS_WORKON_SUBTREE="src/go.chromium.org/chromiumos/test/util"

inherit cros-go cros-workon

DESCRIPTION="Go utilitary code for CFT. Please see go/cft-docs for detailed information about CFT"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/dev-util/+/HEAD/src/go.chromium.org/chromiumos/test/util"

LICENSE="BSD-Google"
KEYWORDS="~*"
IUSE=""

CROS_GO_WORKSPACE=(
	"${S}"
)

CROS_GO_PACKAGES=(
	"go.chromium.org/chromiumos/test/util/..."
)

CROS_GO_TEST=(
	"go.chromium.org/chromiumos/test/util/..."
)

# avoid a circular dependency with cros-test, this can be dropped when using modules mode
CROS_GO_SKIP_DEP_CHECK="1"

DEPEND="
	chromeos-base/cros-config-api
	dev-go/crypto
	dev-go/grpc
	dev-go/mock
	dev-go/protobuf
	dev-go/protobuf-legacy-api
"
RDEPEND="
	${DEPEND}
	!<=dev-util/cros-test-0.0.1-r101
"
