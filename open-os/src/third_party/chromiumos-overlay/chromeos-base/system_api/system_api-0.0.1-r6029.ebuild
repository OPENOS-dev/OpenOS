# Copyright 2011 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="8ea3af2134687a3c1a13767edbf649412bd26e3f"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "0ba423cae8238e4b5b3d178d3ef4ebd9a001c96a" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
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
KEYWORDS="*"
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
