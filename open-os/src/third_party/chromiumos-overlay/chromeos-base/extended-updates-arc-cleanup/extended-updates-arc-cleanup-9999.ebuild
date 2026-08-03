# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_INCREMENTAL_BUILD="1"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk extended-updates/arc-cleanup .gn"

PLATFORM_SUBDIR="extended-updates/arc-cleanup"

inherit cros-workon platform

DESCRIPTION="Utility for cleaning up user's Android data on devices that have lost ARC support after receiving Extended Auto Updates"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/extended-updates/arc-cleanup"

LICENSE="BSD-Google"
KEYWORDS="~*"
SLOT="0/0"
IUSE="arcpp arcvm"

# This package should not be deployed into an ARC-enabled build.
REQUIRED_USE="!arcpp !arcvm"

RDEPEND=""
DEPEND="${RDEPEND}"
BDEPEND="virtual/pkgconfig"
