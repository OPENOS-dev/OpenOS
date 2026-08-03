# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# @ECLASS: cros-fpmcu-unittests.eclass
# @MAINTAINER:
# Chromium OS Firmware Team
# @BUGREPORTS:
# Please report bugs via http://crbug.com/new (with label Build)
# @VCSURL: https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/eclass/@ECLASS@
# @BLURB: helper eclass for building Chromium OS fingerprint MCU unittest binaries
# @DESCRIPTION:
# Builds the fingerprint MCU unittest binaries.
#
# NOTE: When making changes to this class, make sure to modify all the -9999
# ebuilds that inherit it (e.g., cros-fpmcu-unittests) to work around
# https://issuetracker.google.com/201299127.

# @ECLASS-VARIABLE: FIRMWARE_EC_BOARD
# @DEFAULT_UNSET
# @DESCRIPTION:
# EC "board" to build.
: "${FIRMWARE_EC_BOARD:=}"

if [[ -z "${FIRMWARE_EC_BOARD}" ]]; then
	die "FIRMWARE_EC_BOARD must be specified in ebuild."
fi

inherit cros-ec cros-sanitizers

RDEPEND="
	chromeos-base/libec:=
	dev-embedded/libftdi:=
"

DEPEND="
	${RDEPEND}
"

# Make sure config tools use the latest schema.
BDEPEND="
	>=chromeos-base/chromeos-config-host-0.0.2
"

cros-fpmcu-unittests_src_configure() {
	sanitizers-setup-env
	default
}

cros-fpmcu-unittests_src_compile() {
	cros-ec_set_build_env

	einfo "Building FPMCU unittest binary for targets: ${FIRMWARE_EC_BOARD}"
	# shellcheck disable=SC2154
	emake BOARD="${FIRMWARE_EC_BOARD}" "${EC_OPTS[@]}" clean
	emake BOARD="${FIRMWARE_EC_BOARD}" "${EC_OPTS[@]}" tests
}

cros-fpmcu-unittests_src_install() {
	insinto /firmware/chromeos-fpmcu-unittests/"${FIRMWARE_EC_BOARD}"
	doins build/"${FIRMWARE_EC_BOARD}"/*.bin
}

EXPORT_FUNCTIONS src_configure src_compile src_install
