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
	# Also test support packages that live above local/bundles/.
	"go.chromium.org/tast/..."
	"go.chromium.org/tast-tests/..."
)
CROS_GO_VET=(
	"${CROS_GO_TEST[@]}"
)

TAST_BUNDLE_EXCLUDE_DATA_FILES="1"
TAST_BUNDLE_ROOT="go.chromium.org/tast-tests/cros"

inherit cros-workon tast-bundle

DESCRIPTION="Bundle of local integration tests for Chrome OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/tast-tests/"

LICENSE="Apache-2.0 BSD-Google"
SLOT="0/0"
KEYWORDS="*"
IUSE="arc chromeless_tty chromeless_tests cups"

# Build-time dependencies should be added to tast-build-deps, not here.
DEPEND="chromeos-base/tast-build-deps:="

RDEPEND="
	app-misc/c_rehash
	chromeos-base/perfetto
	chromeos-base/tast-local-helpers-cros
	chromeos-base/tast-tests-local-data
	chromeos-base/virtual-usb-printer
	chromeos-base/wprgo
	!chromeless_tty? (
		!chromeless_tests? (
			chromeos-base/drm-tests
		)
	)
	dev-libs/openssl:0=
	dev-python/pyftdi
	arc? (
		chromeos-base/tast-local-apks-cros
		dev-util/android-tools
		dev-util/android-uiautomator-server
	)
	net-misc/curl
	cups? (
		net-print/ippsample
	)
	sys-apps/memtester
	sys-apps/rootdev
	virtual/udev
"
