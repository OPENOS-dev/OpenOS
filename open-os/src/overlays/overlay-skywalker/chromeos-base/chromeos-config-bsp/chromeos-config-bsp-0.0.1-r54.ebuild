# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT="20916c08087908178c8aa7f4159ce63430044c4c"
CROS_WORKON_TREE=("08aa8b62261a367367c1e54c46ed807eb49b0420" "cf0140dc2533cb8ea6cff2ab539cf52141f80bd2" "0327adfcd0472de961b0adf4fefaebde976b06cc" "795fbc551cc6b967e4d13eb10790615e43850b75" "b9f330f7dd34b973a4946c11b29f4416ee73c568" "63b63f8e9c878a52346ec589ed4e248f8f4494ec" "23934703a1d73dfd1a9eac59e16d8829ce15e2f3" "71cd551a481fcd23a01543a22c8a30850197b53e" "c46aec7df2e5b252c2f8e8898529bc5d659fcc0d")
inherit cros-constants
CROS_WORKON_REPO="${CROS_GIT_HOST_URL}"

PROJECTS=(
	"anakin"
	"baze"
	"dooku"
	"grogu"
	"jaina"
	"r2d2"
	"sheev"
	"skywalker"
	"vader"
)

CONFIG_PATH="sw_build_config/platform/chromeos-config"

CROS_WORKON_PROJECT=( "chromiumos/project" )
CROS_WORKON_LOCALNAME=( "project_public" )
CROS_WORKON_SUBTREE=( "$(printf "skywalker/%s/${CONFIG_PATH} " "${PROJECTS[@]}")" )
CROS_WORKON_DESTDIR=( "${PROJECTS[@]/#/${S}/}" )
CROS_BOARDS=( skywalker )

inherit cros-unibuild cros-workon

DESCRIPTION="Chrome OS Model configuration package for skywalker"
HOMEPAGE="https://www.chromium.org/chromium-os"
SRC_URI=""

LICENSE="BSD-Google"
KEYWORDS="*"

RDEPEND="!chromeos-base/chromeos-config-bsp-skywalker"

src_compile() {
	platform_json_compile
}


src_install() {
	platform_json_install
}
