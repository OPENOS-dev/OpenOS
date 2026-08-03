# Copyright 2016 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_PROJECT=(
	"chromiumos/platform2"
	"chromiumos/third_party/pigweed/pigweed"
)
CROS_WORKON_LOCALNAME=(
	"platform2"
	"third_party/pigweed"
)
CROS_WORKON_DESTDIR=(
	"${S}/platform2"
	"${S}/platform2/third_party/pigweed"
)

# TODO(https://crbug.com/809389)
CROS_WORKON_SUBTREE=(
	"build_overrides common-mk libec metrics timberslide .gn"
	""
)

CROS_WORKON_EGIT_BRANCH=(
	"main"
	"upstream/main"
)

PLATFORM_SUBDIR="timberslide"

inherit cros-workon platform

DESCRIPTION="EC log concatenator for Chromium OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/timberslide/"

LICENSE="BSD-Google"
KEYWORDS="~*"
IUSE="+pigweed"

RDEPEND="
	chromeos-base/libec:=
	>=chromeos-base/metrics-0.0.1-r3152:=
	dev-libs/re2:=
	chromeos-base/chromeos-ec-token:=
"

DEPEND="${RDEPEND}"
