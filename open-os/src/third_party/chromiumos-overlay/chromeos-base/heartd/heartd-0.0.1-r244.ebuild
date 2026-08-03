# Copyright 2023 The Chromium OS Authors. All rights reserved.
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "a614296718b76bc1343020a08c1077f9267ee695" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "43eb4f30218ee6fc055f185786d914bccd668086" "b9456d2f3facf22d130c0d8aa5110c4962620af7" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_SUBTREE="common-mk heartd metrics mojo_service_manager libpmt .gn"

PLATFORM_SUBDIR="heartd"

inherit cros-workon platform user

DESCRIPTION="ChromeOS heartd (Health Ensure and Accident Resolve Treatment)."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/heartd"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

COMMON_DEPEND="
	chromeos-base/metrics:=
	chromeos-base/mojo_service_manager:=
	dev-db/sqlite:=
	chromeos-base/libpmt:=
	dev-libs/protobuf:=
"

DEPEND="
	${COMMON_DEPEND}
	chromeos-base/power_manager-client:=
	chromeos-base/system_api:=
"

RDEPEND="
	${COMMON_DEPEND}
	acct-group/intel-pmt
	chromeos-base/minijail:=
"

BDEPEND="
	chromeos-base/minijail:=
"

pkg_preinst() {
	enewuser "heartd"
	enewgroup "heartd"
}

src_install() {
	platform_src_install
}
