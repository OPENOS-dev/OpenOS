# Copyright 2025 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

FIRMWARE_EC_BOARD="rosalia"

# TODO(b/400474462): Set FIRMWARE_EC_RELEASE_REPLACE_RO to "yes"
FIRMWARE_EC_RELEASE_REPLACE_RO="no"

CROS_WORKON_PROJECT=(
	"chromiumos/platform/ec"
	"chromiumos/third_party/cryptoc"
	"external/gob/boringssl/boringssl"
	"external/github.com/google/googletest"
)

CROS_WORKON_LOCALNAME=(
	"../platform/release-firmware/fpmcu-helipilot/ec"
	"cryptoc"
	"boringssl"
	"googletest"
)

CROS_WORKON_DESTDIR=(
	"${S}/platform/ec-legacy"
	"${S}/third_party/cryptoc"
	"${S}/third_party/boringssl"
	"${S}/third_party/googletest"
)

CROS_WORKON_EGIT_BRANCH=(
	"firmware-fpmcu-helipilot-release"
	" " # default value is space. See "cros-workon.eclass" for details.
	" " # default value is space. See "cros-workon.eclass" for details.
	" " # default value is space. See "cros-workon.eclass" for details.
)

inherit cros-workon cros-ec-release cros-sanitizers

HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/ec/+/ec-legacy/README.md"
LICENSE="BSD-Google"
KEYWORDS="~*"

src_configure() {
	sanitizers-setup-env
	default
}
