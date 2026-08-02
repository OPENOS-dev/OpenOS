# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "d77e43912d0bb32c7c16099d8ac4bd8d847f077b" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD="1"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk perfetto_simple_producer .gn"

IUSE="cros-debug"

PLATFORM_SUBDIR="perfetto_simple_producer"

inherit cros-workon platform

DESCRIPTION="Simple Producer of Perfetto for Chromium OS."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/perfetto_simple_producer"

LICENSE="BSD-Google"
KEYWORDS="*"
# This package has no unittests.
RESTRICT="test"

DEPEND="
	chromeos-base/perfetto:="
