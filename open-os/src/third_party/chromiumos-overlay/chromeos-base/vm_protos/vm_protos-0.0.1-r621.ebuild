# Copyright 2019 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="8ea3af2134687a3c1a13767edbf649412bd26e3f"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "abbd78dde0a579e68b274043f7a7c82bf20ff078" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_GO_PACKAGES=(
	"go.chromium.org/chromiumos/vm_tools/..."
)

CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk vm_tools/proto .gn"

PLATFORM_SUBDIR="vm_tools/proto"

WANT_LIBCHROME="no"
WANT_LIBBRILLO="no"

inherit cros-go cros-workon platform cros-protobuf

DESCRIPTION="Chrome OS VM protobuf API"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/vm_tools/proto"
LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="fuzzer"
# Disable unittesting for client bindings.
RESTRICT="test"

RDEPEND="
	dev-cpp/abseil-cpp:=
	net-libs/grpc:=
	!<chromeos-base/vm_guest_tools-0.0.2
	!<chromeos-base/vm_host_tools-0.0.2
"
DEPEND="
	${RDEPEND}
	dev-go/protobuf-legacy-api:=
	dev-go/grpc:=
"

BDEPEND="
	dev-go/protobuf-legacy-api
	net-libs/grpc
"

src_unpack() {
	platform_src_unpack
	CROS_GO_WORKSPACE="${OUT}/gen/go"
}

src_install() {
	platform_src_install

	insinto /usr/"$(get_libdir)"/pkgconfig
	doins vm_protos.pc

	insinto /usr/include/vm_protos/proto_bindings
	doins "${OUT}"/gen/include/vm_protos/proto_bindings/*.h

	dolib.a "${OUT}"/*.a

	cros-go_src_install
}
