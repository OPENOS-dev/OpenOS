# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT=("eab043673f13dad016fb685452b703ccdb3f89a2" "61c30f0d70adfeaf9cb0979f27a678a9cdd87946")
CROS_WORKON_TREE=("2ea2be0b316edcb88e849f5422a7bb5aae75cc38" "ed7fc6979a1dbe18179fb4433a67f41c7768f070")
CROS_WORKON_PROJECT=(
	"chromiumos/platform/dev-util"
	"chromiumos/config"
)

CROS_WORKON_LOCALNAME=(
	"../platform/dev"
	"../config"
)

CROS_WORKON_SUBTREE=(
	"src"
	"python"
)

CROS_WORKON_DESTDIR=(
	"${S}"
	"${S}/config"
)

PYTHON_COMPAT=( python3_11 )

inherit cros-go cros-workon python-any-r1

DESCRIPTION="Utility to check if a DUT is ready to be used for testing"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/dev-util/+/HEAD/src/go.chromium.org/chromiumos/test/check"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""
# This package has no unittests.
RESTRICT="test"

CROS_GO_VERSION="${PF}"

CROS_GO_BINARIES=(
	"go.chromium.org/chromiumos/test/check/cmd/cros_test_ready"
)

CROS_GO_TEST=(
	"go.chromium.org/chromiumos/test/check/cmd/cros_test_ready/..."
)

CROS_GO_VET=(
	"${CROS_GO_TEST[@]}"
)

DEPEND="
	chromeos-base/cros-config-api
	chromeos-base/autotest-client
	chromeos-base/tast-local-tests-cros
	dev-go/protobuf-legacy-api
	dev-go/subcommands
"

BDEPEND="
	$(python_gen_any_dep '
		chromeos-base/cros-config-api[${PYTHON_USEDEP}]
		dev-python/protobuf-python[${PYTHON_USEDEP}]
		dev-python/six[${PYTHON_USEDEP}]
	')
"

python_check_deps() {
	python_has_version -b \
		"chromeos-base/cros-config-api[${PYTHON_USEDEP}]" \
		"dev-python/protobuf-python[${PYTHON_USEDEP}]" \
		"dev-python/six[${PYTHON_USEDEP}]"
}

pkg_setup() {
	cros-workon_pkg_setup
	python_setup
}

src_prepare() {
	export CGO_ENABLED=0
	export GOPIE=0

	default
}

src_install() {
	default
	cros-go_src_install
	# Set the path for the configure file generator.
	local generator="${S}/src/chromiumos/test/check/python/cros_test_ready_config_generator.py"
	local path="${WORKDIR}/cros_test_ready_config.jsonpb"

	export PYTHONDONTWRITEBYTECODE=1
	"${generator}" -src_root "${ROOT}" -output_file="${path}" || die

	insinto /etc
	doins "${path}"
}
