# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="fa49288f446128151560cbcce6a01582cf9569a5"
CROS_WORKON_TREE=("cee48ec7afa799a6476d3569036699dd9152589d" "f283440b701c57221825ce49377ccd964f449ba0" "6d8292c1673370fae095b2b10c1db2c9645f3990" "a4cb24067080149836fbd55b3cdc21bc83918a9f" "a157fa12930114195ea8f93854186927d5da368c" "3c75012f5559925a4e8f5e98c3035795ec0d4a5b" "fa308ea4f9f7626eef3830edbf22a8bec3cf890d" "2d4a432de6b3e49462e7158799f69cd5a0ccb704" "de2c9c97bfead9c4bcde3b90d9ab287d0ae7d286" "3bc760e1c626158ef2523406055c64195fb5f76b" "1e89f60a71ff389b246912ac316c9d36ec61bb95" "506458a091fe243c58d4e260eeaa2957662c233d" "9a8b245a693138ec773cdb599d5c66767c89a65e")
inherit cros-constants
CROS_WORKON_REPO="${CROS_GIT_HOST_URL}"

PROJECTS=(
	"adlnrvp"
	"craask"
	"dirks"
	"dirkson"
	"guren"
	"kaladin"
	"meliks"
	"nereid"
	"nivviks"
	"pujjogatwin"
	"pujjoniru"
	"quandiso"
	"rynar"
)

CONFIG_PATH="sw_build_config/platform/chromeos-config"

CROS_WORKON_PROJECT=( "chromiumos/project" )
CROS_WORKON_LOCALNAME=( "project_public" )
CROS_WORKON_SUBTREE=( "$(printf "nissa/%s/${CONFIG_PATH} " "${PROJECTS[@]}")" )
CROS_WORKON_DESTDIR=( "${PROJECTS[@]/#/${S}/}" )
CROS_BOARDS=( nissa )

inherit cros-unibuild cros-workon

DESCRIPTION="Chrome OS Model configuration package for nissa"
HOMEPAGE="https://www.chromium.org/chromium-os"
SRC_URI=""

LICENSE="BSD-Google"
KEYWORDS="* amd64 x86"
RDEPEND="!chromeos-base/chromeos-config-bsp-nissa"


src_compile() {
	platform_json_compile
}


src_install() {
	platform_json_install
}
