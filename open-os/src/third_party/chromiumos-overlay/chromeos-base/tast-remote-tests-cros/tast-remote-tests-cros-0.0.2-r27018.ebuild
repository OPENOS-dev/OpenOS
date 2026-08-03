# Copyright 2018 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT=("b51b1bb827626468cd6903b9c69dd0d3b036c35e" "ab9500dbea3e7f93d8de0ec93155974043edc10b")
CROS_WORKON_TREE=("1b7f5c2050c188c9a8d612da439c856fac19ae27" "f6bd52880286424e96382675b8abe48475a71a2f")
CROS_WORKON_PROJECT=(
	"chromiumos/platform/tast-tests"
	"chromiumos/platform/tast"
)
CROS_WORKON_LOCALNAME=(
	"platform/tast-tests"
	"platform/tast"
)
CROS_WORKON_DESTDIR=(
	"${S}"
	"${S}/tast-base"
)

CROS_GO_WORKSPACE=(
	"${CROS_WORKON_DESTDIR[@]}"
)

CROS_GO_TEST=(
	# Also test support packages that live above remote/bundles/.
	"go.chromium.org/tast/..."
	"go.chromium.org/tast-tests/..."
)
CROS_GO_VET=(
	"${CROS_GO_TEST[@]}"
)

TAST_BUNDLE_EXCLUDE_DATA_FILES="1"
TAST_BUNDLE_ROOT="go.chromium.org/tast-tests/cros"

inherit cros-workon tast-bundle

DESCRIPTION="Bundle of remote integration tests for Chrome OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/tast-tests/"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

# Build-time dependencies should be added to tast-build-deps, not here.
# tast-cmd is an exception because it also depends on tast-build-deps.
# tast-cmd is required because its module contains the core tast pkgs.
DEPEND="
	chromeos-base/tast-build-deps:=
	chromeos-base/cros-config-api
	chromeos-base/tast-cmd
"

RDEPEND="
	chromeos-base/tast-tests-remote-data
	dev-python/pillow
	media-libs/opencv
"
