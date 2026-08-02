# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_GO_PACKAGES=(
	"go.chromium.org/chromiumos/modemfwd/..."
)

CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk modemfwd .gn"

PLATFORM_SUBDIR="modemfwd/proto"

WANT_LIBCHROME="no"
WANT_LIBBRILLO="no"

inherit cros-workon cros-go platform cros-protobuf

DESCRIPTION="modemfwd go proto for Chrome OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/main/modemfwd"

LICENSE="BSD-Google"
KEYWORDS="~*"
# Disable unittesting for client bindings.
RESTRICT="test"

BDEPEND="
	dev-go/protobuf-legacy-api
"

RDEPEND="
	dev-go/protobuf
"

src_unpack() {
	platform_src_unpack
	CROS_GO_WORKSPACE="${OUT}/gen/go"
}

src_install() {
	platform_src_install

	cros-go_src_install
}
