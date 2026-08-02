# Copyright 2018 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_GO_PACKAGES=(
	"go.chromium.org/chromiumos/policy/..."
)

CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk policy_proto .gn"

PLATFORM_SUBDIR="policy_proto"

WANT_LIBCHROME="no"
WANT_LIBBRILLO="no"

inherit cros-go cros-workon platform cros-protobuf

DESCRIPTION="Chrome OS policy protocol buffer binding for go"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/policy_proto"
LICENSE="BSD-Google"
KEYWORDS="~*"
# Disable unittesting for client bindings.
RESTRICT="test"

DEPEND="
	>=chromeos-base/protofiles-0.0.48:=
	dev-go/grpc
	dev-go/protobuf
"

RDEPEND="${DEPEND}"

BDEPEND="
	dev-go/protobuf-legacy-api:=
"

src_install() {
	platform_src_install

	cros-go_src_install
}

src_unpack() {
	platform_src_unpack
	CROS_GO_WORKSPACE="${OUT}/gen/go"
}
