# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

inherit cros-constants
CROS_WORKON_REPO="${CROS_GIT_HOST_URL}"

PROJECTS=(
	"jedi"
	"padme"
)

CONFIG_PATH="sw_build_config/platform/chromeos-config"

CROS_WORKON_PROJECT=( "chromiumos/project" )
CROS_WORKON_LOCALNAME=( "project_public" )
CROS_WORKON_SUBTREE=( "$(printf "jedi/%s/${CONFIG_PATH} " "${PROJECTS[@]}")" )
CROS_WORKON_DESTDIR=( "${PROJECTS[@]/#/${S}/}" )
# shellcheck disable=SC2034  # Unused variables left for readability
CROS_BOARDS=( jedi )

inherit cros-unibuild cros-workon

DESCRIPTION="Chrome OS Model configuration package for jedi"
HOMEPAGE="https://www.chromium.org/chromium-os"
SRC_URI=""

LICENSE="BSD-Google"
SLOT="0/${PF}"
KEYWORDS="~*"

RDEPEND="!chromeos-base/chromeos-config-bsp-jedi"

src_compile() {
	platform_json_compile
}


src_install() {
	platform_json_install
}
