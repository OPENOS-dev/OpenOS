# Copyright 2022 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
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
KEYWORDS="~*"

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
