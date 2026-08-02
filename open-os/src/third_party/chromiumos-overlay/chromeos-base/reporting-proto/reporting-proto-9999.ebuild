# Copyright 2022 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_GO_PACKAGES=(
	"go.chromium.org/chromiumos/reporting/..."
)

CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk missive .gn"

PLATFORM_SUBDIR="missive/proto"

# The missive gn files expect to find libbrillo and libchrome. We should
# evaluate if we can refactor to remove this dependency.
WANT_LIBCHROME="yes"
WANT_LIBBRILLO="yes"

inherit cros-workon cros-go platform cros-protobuf

DESCRIPTION="reporting/missive go protos for ChromeOS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/main/missive"

LICENSE="BSD-Google"
KEYWORDS="~*"
# Disable unittesting for client bindings.
RESTRICT="test"

DEPEND="
	${RDEPEND}
"

BDEPEND="
	dev-go/protobuf-legacy-api
"

src_unpack() {
	platform_src_unpack
	CROS_GO_WORKSPACE="${OUT}/gen/go"
}

src_install() {
	platform_src_install

	cros-go_src_install
}
