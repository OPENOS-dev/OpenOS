# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="8574abb175071dfa0155399814723a1dfdc23902"
CROS_WORKON_TREE="5f14740aa045a65cb4e1b813ef0657f5e03ecb3c"
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="dlcservice"

# Only ever installed into /usr/local for non-production usage.
CROS_GO_BINARIES=(
	"go.chromium.org/chromiumos/dlc/dlctool:/usr/local/bin/dlctool"
)

CROS_GO_PACKAGES=(
	"go.chromium.org/chromiumos/dlc/dlclib"
)

CROS_GO_TEST=(
	"go.chromium.org/chromiumos/dlc/dlclib"
	"go.chromium.org/chromiumos/dlc/dlctool"
)

inherit cros-workon cros-go

DESCRIPTION="All platform dlcservice developer related tooling (boards + SDK)"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/dlcservice/"

LICENSE="BSD-Google"
KEYWORDS="*"

RDEPEND="
	chromeos-base/dlcservice:=
	chromeos-base/dlcservice-metadata:=
	chromeos-base/imageloader:=
	sys-apps/rootdev:=
	sys-fs/e2fsprogs
	sys-fs/squashfs-tools:=
"

DEPEND="${RDEPEND}"

src_unpack() {
	cros-workon_src_unpack
	CROS_GO_WORKSPACE=(
		"${S}/dlcservice"
	)
}
