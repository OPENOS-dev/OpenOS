# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

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
KEYWORDS="~*"
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
