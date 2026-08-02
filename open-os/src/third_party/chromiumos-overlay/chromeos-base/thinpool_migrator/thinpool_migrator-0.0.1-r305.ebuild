# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "b8033e453c7d9518619e90fb100d7d90d7b4026d" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "76be71469e24417fd7ca89c519a37f6940f60b9f" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk featured metrics thinpool_migrator .gn"

PLATFORM_SUBDIR="thinpool_migrator"

inherit cros-workon platform cros-protobuf

DESCRIPTION="Thinpool migrator for ChromiumOS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/thinpool_migrator/"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

COMMON_DEPEND="
	chromeos-base/featured:=
	chromeos-base/metrics:=
	chromeos-base/vpd:=
	sys-apps/rootdev:=
	sys-fs/e2fsprogs:=
	sys-fs/lvm2:=
"
RDEPEND="
	${COMMON_DEPEND}
"
DEPEND="
	${COMMON_DEPEND}
"
