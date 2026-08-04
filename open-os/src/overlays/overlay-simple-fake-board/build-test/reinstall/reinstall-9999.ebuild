# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"

inherit cros-workon

DESCRIPTION="cros-workon package that can be used to force reinstalls."
HOMEPAGE="http://www.chromium.org"

LICENSE="GPL-2"
KEYWORDS="~*"
IUSE="use-a use-b"

# Swap between USE="use-a" and USE="use-b" to force reinstalls.
RDEPEND="
	use-a? ( !!=build-test/base-pkg-2 )
	use-b? ( !!=build-test/base-pkg-1 )
"
DEPEND="
	use-a? ( =build-test/base-pkg-1:= )
	use-b? ( =build-test/base-pkg-2:= )
"
