# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

DESCRIPTION="Firmware for tools based on Chromium OS EC"
HOMEPAGE="https://www.chromium.org/chromium-os/ec-development"

# stable channel firmware
C2D2_NAME="c2d2_v2.4.82-dbed085877"                     # servo branch builder 05/30/2024
SERVO_MICRO_NAME="servo_micro_v2.4.85-0480cc7379"       # servo branch builder 03/04/2026
SERVO_V4_NAME="servo_v4_v2.4.83-5e9611ca0c"             # servo branch builder 08/28/2024
SERVO_V4P1_NAME="servo_v4p1_v2.0.29298-4d4a4e980"       # EC ToT from 09/23/2025
SWEETBERRY_NAME="sweetberry_v2.4.76-01f828e3a6"         # servo-firmware-R81-12768.204.0

# Prev channel firmware
C2D2_NAME_PREV="c2d2_v2.4.73-d771c18ba9"                # servo-firmware-R81-12768.40.0
SERVO_MICRO_NAME_PREV="servo_micro_v2.4.82-dbed085877"  # servo branch builder 05/30/2024
SERVO_V4_NAME_PREV="servo_v4_v2.4.58-c37246f9c"         # servo-firmware-R81-12768.74.0
SERVO_V4P1_NAME_PREV="servo_v4p1_v2.0.27354-3eeb06336"  # EC ToT from 01/27/2025
SWEETBERRY_NAME_PREV="sweetberry_v2.3.7-096c7ee84"      # servo-firmware-R70-11011.14.0

# Dev channel firmware
C2D2_NAME_DEV="c2d2_v2.4.82-dbed085877"                 # servo branch builder 05/30/2024
SERVO_MICRO_NAME_DEV="servo_micro_v2.4.85-0480cc7379"   # servo branch builder 04/03/2026

# Alpha channel firmware
SERVO_V4P1_NAME_ALPHA="servo_v4p1_v2.0.29601-4fd1021ab" # EC Legacy from 12/12/2025
SERVO_V4_NAME_ALPHA="servo_v4_v2.4.83-5e9611ca0c"       # servo branch builder 08/28/2024

UPDATER_PATH="/usr/share/servo_updater/firmware"

MIRROR_PATH="gs://chromeos-localmirror/distfiles/"

SRC_URI="
	${MIRROR_PATH}/${C2D2_NAME}.tar.xz
	${MIRROR_PATH}/${C2D2_NAME_DEV}.tar.xz
	${MIRROR_PATH}/${C2D2_NAME_PREV}.tar.xz
	${MIRROR_PATH}/${SERVO_MICRO_NAME}.tar.xz
	${MIRROR_PATH}/${SERVO_MICRO_NAME_DEV}.tar.xz
	${MIRROR_PATH}/${SERVO_MICRO_NAME_PREV}.tar.xz
	${MIRROR_PATH}/${SERVO_V4_NAME}.tar.xz
	${MIRROR_PATH}/${SERVO_V4_NAME_PREV}.tar.xz
	${MIRROR_PATH}/${SERVO_V4_NAME_ALPHA}.tar.xz
	${MIRROR_PATH}/${SERVO_V4P1_NAME}.tar.xz
	${MIRROR_PATH}/${SERVO_V4P1_NAME_PREV}.tar.xz
	${MIRROR_PATH}/${SERVO_V4P1_NAME_ALPHA}.tar.xz
	${MIRROR_PATH}/${SWEETBERRY_NAME}.tar.xz
	${MIRROR_PATH}/${SWEETBERRY_NAME_PREV}.tar.gz
	"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"

DEPEND=""
RDEPEND="!<chromeos-base/ec-devutils-0.0.2"

S="${WORKDIR}"

src_install() {
	insinto "${UPDATER_PATH}"

	doins "${C2D2_NAME}.bin"
	doins "${C2D2_NAME_DEV}.bin"
	doins "${C2D2_NAME_PREV}.bin"
	dosym "${C2D2_NAME}.bin" "${UPDATER_PATH}/c2d2.alpha.bin"
	dosym "${C2D2_NAME}.bin" "${UPDATER_PATH}/c2d2.stable.bin"
	dosym "${C2D2_NAME_DEV}.bin" "${UPDATER_PATH}/c2d2.dev.bin"
	dosym "${C2D2_NAME_PREV}.bin" "${UPDATER_PATH}/c2d2.prev.bin"

	doins "${SERVO_MICRO_NAME}.bin"
	doins "${SERVO_MICRO_NAME_DEV}.bin"
	doins "${SERVO_MICRO_NAME_PREV}.bin"
	dosym "${SERVO_MICRO_NAME}.bin" "${UPDATER_PATH}/servo_micro.alpha.bin"
	dosym "${SERVO_MICRO_NAME}.bin" "${UPDATER_PATH}/servo_micro.stable.bin"
	dosym "${SERVO_MICRO_NAME_DEV}.bin" "${UPDATER_PATH}/servo_micro.dev.bin"
	dosym "${SERVO_MICRO_NAME_PREV}.bin" "${UPDATER_PATH}/servo_micro.prev.bin"

	doins "${SERVO_V4_NAME}.bin"
	doins "${SERVO_V4_NAME_PREV}.bin"
	doins "${SERVO_V4_NAME_ALPHA}.bin"
	dosym "${SERVO_V4_NAME_ALPHA}.bin" "${UPDATER_PATH}/servo_v4.alpha.bin"
	dosym "${SERVO_V4_NAME}.bin" "${UPDATER_PATH}/servo_v4.stable.bin"
	dosym "${SERVO_V4_NAME}.bin" "${UPDATER_PATH}/servo_v4.dev.bin"
	dosym "${SERVO_V4_NAME_PREV}.bin" "${UPDATER_PATH}/servo_v4.prev.bin"

	doins "${SERVO_V4P1_NAME}.bin"
	doins "${SERVO_V4P1_NAME_PREV}.bin"
	doins "${SERVO_V4P1_NAME_ALPHA}.bin"
	dosym "${SERVO_V4P1_NAME_ALPHA}.bin" "${UPDATER_PATH}/servo_v4p1.alpha.bin"
	dosym "${SERVO_V4P1_NAME}.bin" "${UPDATER_PATH}/servo_v4p1.stable.bin"
	dosym "${SERVO_V4P1_NAME}.bin" "${UPDATER_PATH}/servo_v4p1.dev.bin"
	dosym "${SERVO_V4P1_NAME_PREV}.bin" "${UPDATER_PATH}/servo_v4p1.prev.bin"

	doins "${SWEETBERRY_NAME}.bin"
	doins "${SWEETBERRY_NAME_PREV}.bin"
	dosym "${SWEETBERRY_NAME}.bin" "${UPDATER_PATH}/sweetberry.alpha.bin"
	dosym "${SWEETBERRY_NAME}.bin" "${UPDATER_PATH}/sweetberry.stable.bin"
	dosym "${SWEETBERRY_NAME}.bin" "${UPDATER_PATH}/sweetberry.dev.bin"
	dosym "${SWEETBERRY_NAME_PREV}.bin" "${UPDATER_PATH}/sweetberry.prev.bin"
}
