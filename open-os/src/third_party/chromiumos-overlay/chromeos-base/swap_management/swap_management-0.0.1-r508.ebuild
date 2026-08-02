# Copyright 2022 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "16839d1d0fcecb39e52d1bc3a65aa02d62c1ee6d" "0afd6b553a9d6224db46866e207640441e2cd72f" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "b8033e453c7d9518619e90fb100d7d90d7b4026d" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD="1"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk chromeos-config swap_management metrics featured .gn"

PLATFORM_SUBDIR="swap_management"

inherit cros-sanitizers cros-workon platform cros-protobuf

DESCRIPTION="ChromeOS swap management service"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/swap_management/"
LICENSE="BSD-Google"
SLOT="0/0"
KEYWORDS="*"

COMMON_DEPEND="
	chromeos-base/featured:=
	chromeos-base/metrics:=
	chromeos-base/minijail:=
	dev-cpp/abseil-cpp:=
"

RDEPEND="${COMMON_DEPEND}"

DEPEND="${COMMON_DEPEND}
	chromeos-base/chromeos-config-tools:=
	chromeos-base/power_manager-client:=
	chromeos-base/system_api:=
	sys-apps/dbus:="

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
"
