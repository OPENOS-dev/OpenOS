# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="05276fed7048278c54a74f593639dd4a3d5259f9"
CROS_WORKON_TREE=("9e2536252395eb4644f1ea01c758c6bee80da2bb" "021e124d33a6882886b2bd48ae92321cae288d5e" "14b855551ba6771c9a06393d3a9ceb3f97003ea8" "eb2bf247841ea6691d05820f63954c0910112850" "fd559bf3d1f96c111c17247ff73fd4c166072f08" "d99eb7ce0475c44158925c1e8ad9fffae663c216" "2918e3324692000d9e39da6e195da1ea3b4d0975" "1e2cf58256b13aa9a8b3d1b3428e9bec6f4c7208")
CROS_WORKON_PROJECT="chromiumos/platform/dev-util"
CROS_WORKON_LOCALNAME=("../platform/dev")
CROS_WORKON_SUBTREE="src/go.chromium.org/chromiumos/test/execution src/go.chromium.org/chromiumos/test/ctpv2/provision_request_generators/provision-filter src/go.chromium.org/chromiumos/test/ctpv2/test_finder_filter src/go.chromium.org/chromiumos/test/ctpv2/common src/go.chromium.org/chromiumos/test/post_process/cmd/post-process/common src/go.chromium.org/chromiumos/test/publish src/go.chromium.org/chromiumos/test/test_finder src/go.chromium.org/chromiumos/test/util"

inherit cros-go cros-workon

DESCRIPTION="Microservices used by CTPv2"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/dev-util/+/HEAD/src/go.chromium.org/chromiumos/test/dut"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

CROS_GO_WORKSPACE=(
	"${S}"
)

CROS_GO_BINARIES=(
	"go.chromium.org/chromiumos/test/ctpv2/provision_request_generators/provision-filter"
	"go.chromium.org/chromiumos/test/ctpv2/test_finder_filter"
)

CROS_GO_VET=(
	"${CROS_GO_TEST[@]}"
)

# ctpv2/test_finder_filter used to be provided by cros-test-finder before r121, after which
# it was moved to this ebuild because it's part of the ctpv2 module, thus we need to add
# a !<= r120 dependency masking for crost-test-finder to instruct portage how to upgrade.
DEPEND="
	chromeos-base/tast-cmd:=
	chromeos-base/tast-proto
	dev-util/cros-test
	dev-util/cros-test-finder
	dev-util/cros-test-util
	dev-util/post-process
	dev-go/grpc
	dev-go/protobuf
	dev-go/protobuf-legacy-api
	chromeos-base/cros-config-api
	!<=dev-util/cros-test-finder-0.0.1-r120
"
RDEPEND="${DEPEND}"

src_prepare() {
	# Disable cgo and PIE on building binaries. See:
	# https://crbug.com/976196
	# https://github.com/golang/go/issues/30986#issuecomment-475626018
	export CGO_ENABLED=0
	export GOPIE=0

	default
}
