# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="bf8669d768b73e7b368e23e010a5c0aba19e0353"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "745b9d426d9ef83cd0e13b00ebe56852814fe3bf" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD="1"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk printscanmgr .gn"

PLATFORM_SUBDIR="printscanmgr"

inherit cros-workon platform cros-protobuf user

DESCRIPTION="Chrome OS printing and scanning daemon"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/printscanmgr/"
LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="fuzzer"

COMMON_DEPEND="
	chromeos-base/minijail:=
"

RDEPEND="
	${COMMON_DEPEND}
	sys-apps/dbus:=
"

DEPEND="${COMMON_DEPEND}
	chromeos-base/lorgnette-client:=
	chromeos-base/system_api:=[fuzzer?]
"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
	chromeos-base/minijail
"

pkg_preinst() {
	enewuser printscanmgr
	enewgroup printscanmgr
}

src_install() {
	platform_src_install

	# Install fuzzers.
	local fuzzer_component_id="167231"
	platform_fuzzer_install "${S}"/OWNERS "${OUT}"/cups_uri_helper_utils_fuzzer \
		--comp "${fuzzer_component_id}"
}
