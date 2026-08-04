# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="f800754b934fb0e781c1f154d369b4bde50d0365"
CROS_WORKON_TREE=("80c6bd27db69fabf21234cd1b520ff651ed47d52" "c3f03c2e4b1bfa19b920711209b062bb584293d7" "3a97f66c0d64d7037efac6a95d6e8fbf8898e71a" "d3edb0c64451da5ec6c5ea47c6d8bd824be1272d" "46a8f64189a187c68865a83855b0c20dee47dfcc" "20b6cfb670b75dd3e75965613512127dfb6a2362" "58d99c20a1e5b4a16717b8cf5587ccf28285f1f1" "54ff3c23f3a88f9a6c07ea96f7519026d82885e6")
inherit cros-constants
CROS_WORKON_REPO="${CROS_GIT_HOST_URL}"

PROJECTS=(
    "berknip"
    "dalboz"
    "dirinboz"
    "ezkinil"
    "morphius"
    "trembyle"
    "vilboz"
    "woomax"
)

CONFIG_PATH="sw_build_config/platform/chromeos-config"

CROS_WORKON_PROJECT=( "chromiumos/project" )
CROS_WORKON_LOCALNAME=( "project_public" )
CROS_WORKON_SUBTREE=( "$(printf "zork/%s/${CONFIG_PATH} " "${PROJECTS[@]}")" )
CROS_WORKON_DESTDIR=( "${PROJECTS[@]/#/${S}/}" )
CROS_BOARDS=( zork )

inherit cros-unibuild cros-workon

DESCRIPTION="Chrome OS Model configuration package for zork"
HOMEPAGE="https://www.chromium.org/chromium-os"
SRC_URI=""

LICENSE="BSD-Google"
KEYWORDS="* amd64 x86"
RDEPEND="!chromeos-base/chromeos-config-bsp-zork"


src_compile() {
	platform_json_compile
}


src_install() {
	platform_json_install
}
