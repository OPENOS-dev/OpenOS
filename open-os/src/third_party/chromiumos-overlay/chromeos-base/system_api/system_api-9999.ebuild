# Copyright 2011 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_GO_PACKAGES=(
	"go.chromium.org/chromiumos/system_api/..."
)

CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk system_api .gn"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="system_api"
WANT_LIBCHROME="no"
WANT_LIBBRILLO="no"

inherit cros-fuzzer cros-go cros-workon platform cros-protobuf

DESCRIPTION="Chrome OS system API (D-Bus service names, etc.)"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/system_api/"
LICENSE="BSD-Google"
KEYWORDS="~*"
IUSE="cros_host"
# This package has no unittests.
RESTRICT="test"

RDEPEND="
	dev-cpp/abseil-cpp:=
	cros_host? ( net-libs/grpc:= )
"

DEPEND="${RDEPEND}
	dev-go/protobuf:=
	dev-go/protobuf-legacy-api:=
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

	find "${D}"/usr/include/ \
		'(' -name OWNERS -o -name DIR_METADATA -o -name '*.md' ')' \
		-delete || die

	cros-go_src_install
}
