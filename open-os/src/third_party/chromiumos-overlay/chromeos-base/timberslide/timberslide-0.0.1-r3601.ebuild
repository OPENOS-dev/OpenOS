# Copyright 2016 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT=("362404e596160add78f63bc42ff2081b91941af5" "ce4bed460493919c8f1c7ea614856d2fd79933d2")
CROS_WORKON_TREE=("e95d21e45ff5cc81cc5e9f4f65c27eb613532cd2" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "4934b6b332f2a3db7a26bad9f888607a4f12b440" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "c2545f39bf3492543cbcb60967bff59494e2e77a" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "70e996d49f0a4c553f3509ae270e99f9f032d16c")
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
KEYWORDS="*"
IUSE="+pigweed"

RDEPEND="
	chromeos-base/libec:=
	>=chromeos-base/metrics-0.0.1-r3152:=
	dev-libs/re2:=
	chromeos-base/chromeos-ec-token:=
"

DEPEND="${RDEPEND}"
