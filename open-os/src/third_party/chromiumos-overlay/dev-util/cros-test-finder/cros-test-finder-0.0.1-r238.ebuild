# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="361bf04c0429358aae57f6aa432e093a32643048"
CROS_WORKON_TREE=("eb2bf247841ea6691d05820f63954c0910112850" "fd559bf3d1f96c111c17247ff73fd4c166072f08" "2918e3324692000d9e39da6e195da1ea3b4d0975" "9e2536252395eb4644f1ea01c758c6bee80da2bb")
CROS_WORKON_PROJECT="chromiumos/platform/dev-util"
CROS_WORKON_LOCALNAME=("../platform/dev")
CROS_WORKON_SUBTREE="src/go.chromium.org/chromiumos/test/ctpv2/common src/go.chromium.org/chromiumos/test/post_process/cmd/post-process/common src/go.chromium.org/chromiumos/test/test_finder src/go.chromium.org/chromiumos/test/execution"

inherit cros-go cros-workon

DESCRIPTION="Test finder for find tests that match the specified test suite tags"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/dev-util/+/HEAD/src/go.chromium.org/chromiumos/test/test_finder"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

CROS_GO_VERSION="${PF}"

CROS_GO_BINARIES=(
	"go.chromium.org/chromiumos/test/test_finder/cmd/cros-test-finder"
)

CROS_GO_TEST=(
	"go.chromium.org/chromiumos/test/test_finder/cmd/cros-test-finder/..."
)

CROS_GO_VET=(
	"${CROS_GO_TEST[@]}"
)

DEPEND="
	chromeos-base/tast-cmd:=
	chromeos-base/tast-proto
	dev-util/cros-test
	dev-util/cros-test-util
	dev-util/post-process
	dev-util/lro-server
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
