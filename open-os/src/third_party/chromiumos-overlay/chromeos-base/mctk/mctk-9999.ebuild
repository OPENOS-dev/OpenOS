# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_SUBTREE=".gn camera/build camera/tools camera/include common-mk"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="camera/tools/mctk"

inherit cros-workon platform

DESCRIPTION="Video4Linux2 media-ctl toolkit"

LICENSE="BSD-Google"
KEYWORDS="~*"
# This package has no unittests.
RESTRICT="test"

RDEPEND="
	dev-libs/re2:=
	dev-libs/libyaml:=
"

DEPEND="
	${RDEPEND}
	sys-kernel/linux-headers:=
"
