# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT=("8b65138050c38379e489e4a323e0472c08497449" "11585a7d4e43496aeb3eb4bd9c9950fbe168954f")
CROS_WORKON_TREE=("d2a78086f87c80edc5349072a5d693f8ac3b4fca" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_GO_PACKAGES=(
	"android.googlesource.com/platform/external/perfetto/protos/perfetto/metrics/github.com/google/perfetto/perfetto_proto"
	"android.googlesource.com/platform/external/perfetto/protos/perfetto/trace/github.com/google/perfetto/perfetto_proto"
)

inherit cros-constants

# This ebuild is upreved via PUpr, so disable the normal uprev process for
# cros-workon ebuilds.
# The repo manifest project config tracks the upstream main branch. This Ebuild
# needs to stay manual uprev (CROS_WORKON_MANUAL_UPREV=1).
CROS_WORKON_MANUAL_UPREV=1
CROS_WORKON_LOCALNAME=("../third_party/perfetto" "../platform2")
CROS_WORKON_PROJECT=("external/github.com/google/perfetto" "chromiumos/platform2")
CROS_WORKON_REPO=("${CROS_GIT_HOST_URL}" "${CROS_GIT_HOST_URL}")
CROS_WORKON_DESTDIR=("${S}/third_party/perfetto" "${S}/platform2")
CROS_WORKON_EGIT_BRANCH=("main" "main")
CROS_WORKON_SUBTREE=("" "common-mk .gn")

PLATFORM_SUBDIR="./"
WANT_LIBCHROME="no"
WANT_LIBBRILLO="no"

inherit cros-go cros-workon platform cros-protobuf

CROS_GO_MODULE_NAME="android.googlesource.com/platform/external/perfetto/protos"

# because the go files get generated during compile, we need to delay
# src_prepare run, because they don't exist at normal prepare time
CROS_GO_DELAYED_PREPARE="1"

# there is no go.mod definition for this project, it's go files
# are generated so we also generate the go.mod
CROS_GO_DEFAULT_EMPTY_MODFILE="1"

# this ebuild version does not respect the Go module semantic versioning requirements
# so we set a pseudo version based on ${PV} to fix it.
CROS_GO_PSEUDO_VERSION="v0.${PV}-${CROS_GO_PSEUDO_VERSION_DATE}-${CROS_WORKON_COMMIT[0]}"

DESCRIPTION="Perfetto go proto for Chrome OS"
HOMEPAGE="https://android.googlesource.com/platform/external/perfetto/+/refs/heads/master/protos/perfetto/metrics/android/"

KEYWORDS="*"
IUSE="cros-debug"
LICENSE="Apache-2.0"
SLOT="0"
# Disable unittesting for client bindings.
RESTRICT="test"

BDEPEND="
	dev-go/protobuf-legacy-api
"

DEPEND="
	dev-go/protobuf
"
RDEPEND="
	${DEPEND}
	!chromeos-base/perfetto_proto
"

src_unpack() {
	platform_src_unpack
	CROS_GO_WORKSPACE="${OUT}/gen/go"
}

src_prepare() {
	default
	cp "${FILESDIR}/BUILD.gn" "${S}"
}

src_install() {
	platform_src_install

	cros-go_src_install
}
