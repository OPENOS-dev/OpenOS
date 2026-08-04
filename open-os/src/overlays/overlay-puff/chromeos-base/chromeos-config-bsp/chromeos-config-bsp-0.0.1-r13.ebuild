# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="367af8e464dd35ec930c8823cc42fd7825eb1d62"
CROS_WORKON_TREE=("82cb6f3691779400b3e815fa91231780280dddb4" "2101a6c2eaf295022600b0ce4181eed684d51a9a" "c136102c884c5cf0207937ce4425ec49281325fe" "d12e639dac5d419a4e9464787f51ce29a190b2ee" "75f1fb5adef2ceb31dbe58fdf09f12d34b0e9488" "7f1a7e78d815d27a11c449b3846c914a7a3df81f" "3ec2ef905de810db5fa9e4533cf75bca807f05bd")
inherit cros-constants
CROS_WORKON_REPO="${CROS_GIT_HOST_URL}"

PROJECTS=(
	"dooly"
    "duffy"
    "faffy"
    "kaisa"
    "noibat"
    "puff"
    "wyvern"
)

CONFIG_PATH="sw_build_config/platform/chromeos-config"

CROS_WORKON_PROJECT=( "chromiumos/project" )
CROS_WORKON_LOCALNAME=( "project_public" )
CROS_WORKON_SUBTREE=( "$(printf "puff/%s/${CONFIG_PATH} " "${PROJECTS[@]}")" )
CROS_WORKON_DESTDIR=( "${PROJECTS[@]/#/${S}/}" )
CROS_BOARDS=( puff )

inherit cros-unibuild cros-workon

DESCRIPTION="Chrome OS Model configuration package for puff"
HOMEPAGE="https://www.chromium.org/chromium-os"
SRC_URI=""

LICENSE="BSD-Google"
KEYWORDS="* amd64 x86"
RDEPEND="!chromeos-base/chromeos-config-bsp-puff"


src_compile() {
	platform_json_compile
}


src_install() {
	platform_json_install
}
