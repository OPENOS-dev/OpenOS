# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="129efbc2a1678787ee26e0f0a538c6f5764e4c1b"
CROS_WORKON_TREE=("d98122ac5298938f05729b39ecc73ca0484fcc24" "ca453e2e5ecaf0910e119ca61381e55d3610a740" "45edd30c1f972a8db114acc537cca5a864f900b8" "88459381377b8676417b224c5e3b442669512ca6" "4624be3360500b03ac5fe91dda05532cfe21bf18" "671a296befd47940497e085ac4500669e990605d" "0d8da2656ddb783f8d3416d51ca870e47597d75d" "b17d25d8417568de9103b335675cfa72bf2768f6" "03c659c9808a7ebae49f93c57c7b18cd4534e90f" "a18741519e9cd0862f326ae9fc2c6097278f465b" "1ad4c63f4aa7de5aa4f77dd465b9593336a00d59")
inherit cros-constants
CROS_WORKON_REPO="${CROS_GIT_HOST_URL}"

PROJECTS=(
	"chinchou"
	"kingler"
	"krabby"
	"kyogre"
	"magikarp"
	"ponyta"
	"skitty"
	"steelix"
	"tentacruel"
	"veluza"
	"voltorb"
)

CONFIG_PATH="sw_build_config/platform/chromeos-config"

CROS_WORKON_PROJECT=( "chromiumos/project" )
CROS_WORKON_LOCALNAME=( "project_public" )
CROS_WORKON_SUBTREE=( "$(printf "corsola/%s/${CONFIG_PATH} " "${PROJECTS[@]}")" )
CROS_WORKON_DESTDIR=( "${PROJECTS[@]/#/${S}/}" )
CROS_BOARDS=( corsola )

inherit cros-unibuild cros-workon

DESCRIPTION="Chrome OS Model configuration package for corsola"
HOMEPAGE="https://www.chromium.org/chromium-os"
SRC_URI=""

LICENSE="BSD-Google"
KEYWORDS="*"

RDEPEND="!chromeos-base/chromeos-config-bsp-corsola"

src_compile() {
	platform_json_compile
}


src_install() {
	platform_json_install
}
