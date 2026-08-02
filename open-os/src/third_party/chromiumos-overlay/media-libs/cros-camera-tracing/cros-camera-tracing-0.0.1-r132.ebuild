# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "87b960d376ab8b63d4a97741e1013a3fef49bffa" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3")
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_SUBTREE=".gn camera/tracing common-mk"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="camera/tracing"

inherit cros-workon platform

DESCRIPTION="ChromeOS camera tracing tool."

LICENSE="BSD-Google"
KEYWORDS="*"
# This package has no unittests.
RESTRICT="test"
