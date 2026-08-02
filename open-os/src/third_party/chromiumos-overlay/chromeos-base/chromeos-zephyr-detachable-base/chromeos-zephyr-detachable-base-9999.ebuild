# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.makefile file.

EAPI=7

CROS_WORKON_USE_VCSID=1
CROS_WORKON_PROJECT=(
	"chromiumos/third_party/zephyrproject"
	"chromiumos/third_party/pigweed/pigweed"
	"chromiumos/platform/ec"
)

CROS_WORKON_LOCALNAME=(
	"third_party/zephyrproject"
	"third_party/pigweed"
	"platform/ec"
)

CROS_WORKON_DESTDIR=(
	"${S}/zephyrproject"
	"${S}/zephyrproject/modules/pigweed"
	"${S}/zephyrproject/modules/ec"
)

IUSE="has_chromeos_zephyr_touch_firmware"

DEPEND="
	has_chromeos_zephyr_touch_firmware? ( chromeos-base/chromeos-zephyr-touch-firmware:= )
"

inherit cros-workon cros-zephyr-utils python-any-r1 coreboot-sdk coreboot-sdk-ec-dependencies

DESCRIPTION="Zephyr based firmware for detachable base"
KEYWORDS="~*"

coreboot-sdk_enable riscv64-elf

update_touchpad_hash() {
	local project=$1
	local ec_fw="build/${project}/output/ec.bin"
	local prikey="build/${project}/output/key.vbprik2"
	local tp_fw="${SYSROOT}/firmware/${project}/touchpad.bin"

	if [[ -f "${tp_fw}" ]]; then
		echo "Generate touchpad hash for ${project}"
		"${EPYTHON}" "${S}/zephyrproject/modules/ec/util/gen_touchpad_hash_zephyr.py" \
			--ec-fw="${ec_fw}" --tp-fw="${tp_fw}" \
			--prikey="${prikey}" || die
	fi
}

src_compile() {
	local project

	cros-zephyr-compile zephyr-detachable-base

	if use has_chromeos_zephyr_touch_firmware; then
		while read -r _ && read -r project; do
			update_touchpad_hash "${project}" || die
		done < <(cros_config_host get-firmware-build-combinations zephyr-detachable-base || die)
	fi
}

src_install() {
	local project
	local filename

	while read -r _ && read -r project; do
		if [[ -z "${project}" ]]; then
			continue
		fi

		insinto "/firmware/${project}"
		# Install everything except the signing key
		while read -r filename; do
			doins "${filename}"
		done < <(find "build/${project}/output" '!' -name '*.vbprik2' -type f)
	done < <(cros_config_host get-firmware-build-combinations zephyr-detachable-base || die)
}
