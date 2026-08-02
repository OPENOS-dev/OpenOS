# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT=("362404e596160add78f63bc42ff2081b91941af5" "5e9a89d06c41edf5cf43da8acf5f26ed104887e6")
CROS_WORKON_TREE=("f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "1cd1013057a51ae459cfe5a0b07deaf4793e68e7" "47c9d8a1ba175459aaf9f255c44f91df349864ab" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "43eb4f30218ee6fc055f185786d914bccd668086" "693bb2d63562c6eff050d04f75aab1e9251e6548")
inherit cros-constants

CROS_WORKON_PROJECT=(
	"chromiumos/platform2"
	"aosp/platform/frameworks/native"
)
CROS_WORKON_LOCALNAME=(
	"platform2"
	"aosp/frameworks/native"
)
CROS_WORKON_REPO=(
	"${CROS_GIT_HOST_URL}"
	"${CROS_GIT_HOST_URL}"
)
CROS_WORKON_DESTDIR=(
	"${S}/platform2"
	"${S}/platform2/aosp/frameworks/native"
)
CROS_WORKON_EGIT_BRANCH=(
	"main"
	"master"
)
# TODO(crbug.com/809389): Remove libmems from this list.
CROS_WORKON_SUBTREE=(".gn iioservice libmems common-mk metrics mojo_service_manager" "")
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="iioservice/daemon"

inherit cros-sanitizers cros-workon platform user

DESCRIPTION="Chrome OS sensor HAL IPC util."

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="+seccomp"

RDEPEND="
	>=chromeos-base/metrics-0.0.1-r3152:=
	chromeos-base/libiioservice_ipc:=
	chromeos-base/libmems:=
	chromeos-base/mems_setup
	chromeos-base/mojo_service_manager:=
	dev-cpp/abseil-cpp:=
	virtual/chromeos-ec-driver-init
"

DEPEND="${RDEPEND}
	chromeos-base/system_api:=
"

BDEPEND="
	chromeos-base/minijail
"

pkg_preinst() {
	enewuser "iioservice"
	enewgroup "iioservice"
}
