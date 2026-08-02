# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "0a73e14ae1ced1296f87bfeeacd2917bf320a575")
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_INCREMENTAL_BUILD=1

CROS_WORKON_SUBTREE="common-mk .gn resourced/src/vm_grpc/proto resourced/vm_grpc/interface"

PLATFORM_SUBDIR="resourced/vm_grpc/interface"

inherit cros-go cros-workon platform cros-protobuf

DESCRIPTION="Resourced lib to interact with resourced. Usable on Chrome OS and Guest VM"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/resourced"

LICENSE="BSD-Google"
KEYWORDS="*"

COMMON_DEPEND="
	chromeos-base/minijail:=
	net-libs/grpc:=
	dev-go/protobuf-legacy-api:=
"

RDEPEND="
	${COMMON_DEPEND}
"

DEPEND="
	${COMMON_DEPEND}
	dev-go/grpc:=
	sys-kernel/linux-headers:=
"

src_install() {
	platform_src_install

	dolib.so "${OUT}"/lib/libresourceD.so
}
