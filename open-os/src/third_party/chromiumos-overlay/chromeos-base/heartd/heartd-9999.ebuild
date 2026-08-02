# Copyright 2023 The Chromium OS Authors. All rights reserved.
# Distributed under the terms of the GNU General Public License v2

EAPI=7

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
KEYWORDS="~*"
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
