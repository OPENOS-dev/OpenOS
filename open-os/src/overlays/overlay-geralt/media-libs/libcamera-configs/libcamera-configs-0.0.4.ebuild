# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Libcamera Config files for mtkisp7"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"

S="${WORKDIR}"

src_install() {
	local CONFIG_DIR="/etc/camera/libcamera"
	insinto "${CONFIG_DIR}"
	doins "${FILESDIR}"/*
}
