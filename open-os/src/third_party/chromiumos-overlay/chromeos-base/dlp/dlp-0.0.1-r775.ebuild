# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "832756314eec04598443ce5bd5e2c804915d78c0" "b8033e453c7d9518619e90fb100d7d90d7b4026d" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk dlp featured metrics .gn"

PLATFORM_SUBDIR="dlp"

inherit cros-workon libchrome platform cros-protobuf user

DESCRIPTION="A daemon that provides support for Data Leak Prevention restrictions for file accesses."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/dlp/"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="fuzzer"

COMMON_DEPEND="
	chromeos-base/featured:=
	chromeos-base/metrics:=
	chromeos-base/minijail:=
	chromeos-base/system_api:=
	!dev-db/leveldb
	dev-db/sqlite:=
	dev-libs/leveldb:=
"
RDEPEND="${COMMON_DEPEND}"
DEPEND="${COMMON_DEPEND}
	chromeos-base/session_manager-client:=
	sys-apps/dbus:=
	fuzzer? ( dev-libs/libprotobuf-mutator:= )
"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
	chromeos-base/minijail
"

src_install() {
	platform_src_install

	local daemon_store="/etc/daemon-store/dlp"
	dodir "${daemon_store}"
	fperms 0700 "${daemon_store}"
	fowners dlp:dlp "${daemon_store}"

	local fuzzer_component_id="892101"
	platform_fuzzer_install "${S}"/OWNERS "${OUT}"/dlp_adaptor_fuzzer \
		--comp "${fuzzer_component_id}"
}

pkg_setup() {
	enewuser "dlp"
	enewgroup "dlp"
	cros-workon_pkg_setup
}
