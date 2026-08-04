# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="5901d0a6be8abb7d8a6a62c4cd94333675eb6a15"
CROS_WORKON_TREE=("763e254a6d75c32253bbe81f01c8ba70400c5f69" "c4fa094fb3f496c846de81a05c9ab99e7f85c56c" "437992fe2d08956d93c5cf22ebc692a220dfdfdd" "e5208eb48117dc0ca9395bb930a4786e92351c4d" "3a6ea6f48a0aab6b83232d7b2967229c58a03a8f" "67ef5bc68ea53f950c2845ed95e258952248124e" "6ed171ba00b762d214f26da65a8517540b83b9d8" "089337175129c469c781375905b9a5e40725e76a" "0c0c77ef557fc75762e9533a77a6c6a77abcc8eb" "e7021d0ecec97c283da6eed0f758121b59fe6470" "1f9e7ab5b418a66c4efd6a2606f255909ea6fb5a" "ae0d95bd5c9fabf60944804ad6c87319d5699b18" "11f56b1aede831e61997e131126b5ffe2657b122")
inherit cros-constants
CROS_WORKON_REPO="${CROS_GIT_HOST_URL}"

PROJECTS=(
	"chronicler"
	"collis"
	"copano"
	"delbin"
	"drobit"
	"eldrid"
	"elemi"
	"lindar"
	"terrador"
	"voema"
	"volet"
	"volteer"
	"voxel"
)

CONFIG_PATH="sw_build_config/platform/chromeos-config"

CROS_WORKON_PROJECT=( "chromiumos/project" )
CROS_WORKON_LOCALNAME=( "project_public" )
CROS_WORKON_SUBTREE=( "$(printf "volteer/%s/${CONFIG_PATH} " "${PROJECTS[@]}")" )
CROS_WORKON_DESTDIR=( "${PROJECTS[@]/#/${S}/}" )
CROS_BOARDS=( volteer )

inherit cros-unibuild cros-workon

DESCRIPTION="Chrome OS Model configuration package for volteer"
HOMEPAGE="https://www.chromium.org/chromium-os"
SRC_URI=""

LICENSE="BSD-Google"
KEYWORDS="* amd64 x86"
RDEPEND="!chromeos-base/chromeos-config-bsp-volteer"


src_compile() {
	platform_json_compile
}


src_install() {
	platform_json_install
}
