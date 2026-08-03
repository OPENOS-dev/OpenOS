# Copyright 2021 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.makefile file.

EAPI=7

CROS_WORKON_COMMIT=("8c3719f3ba4bd018bdf242905bee85a3668a5ac1" "ce4bed460493919c8f1c7ea614856d2fd79933d2" "22ddd18cef071f61fa17c54d72f536c97696a847")
CROS_WORKON_TREE=("123239e7f10ad93cab8546673f5ea5d2ced064c4" "70e996d49f0a4c553f3509ae270e99f9f032d16c" "1387f1c1e8aeb0cc210b5099180dbfdbbad30e21")
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

CROS_WORKON_EGIT_BRANCH=(
	"main"
	"main"
	"main"
)

inherit cros-workon cros-zephyr-utils coreboot-sdk coreboot-sdk-ec-dependencies

DESCRIPTION="Zephyr based Embedded Controller firmware"
KEYWORDS="*"

MERGED_DB="tokens.bin"
PROJECT_DB="database.bin"
PW_ROOT="${S}/zephyrproject/modules/pigweed"

IUSE="internal"
DEPEND="internal? ( chromeos-base/chromeos-zephyr-private-files:= )"

# cros-zephyr-compile will generate individual project token databases.
# Find all individual project token databases and merge them into single
# merged database.
merge_token_db() {
	if [[ -n $(find build -name "${PROJECT_DB}") ]]; then
		find build -name "${PROJECT_DB}" -exec \
			"${PW_ROOT}"/pw_tokenizer/py/pw_tokenizer/database.py \
			create --type binary --force --database "build/${MERGED_DB}" {} + \
			|| die
	fi
}

coreboot-sdk_enable arm-eabi
coreboot-sdk_enable libstdcxx-arm-eabi
coreboot-sdk_enable picolibc-arm-eabi
coreboot-sdk_enable riscv64-elf
coreboot-sdk_enable libstdcxx-riscv64-elf
coreboot-sdk_enable picolibc-riscv64-elf

# Make private source files available in the source directory
# if private source is available.
src_prepare() {
	default

	local private_directories=("modules/google-private")

	if use internal; then
		# Link the private sources in the private/ sub-directory.
		local dir
		for dir in "${private_directories[@]}"; do
			ln -sfT "${SYSROOT}/firmware/private-directories/${dir}" \
				"${S}/zephyrproject/${dir}" || die
		done
	fi
}

src_compile() {
	cros-zephyr-compile zephyr-ec
	merge_token_db
}

src_install() {
	local firmware_name project
	local root_build_dir="build"

	while read -r firmware_name && read -r project; do
		if [[ -z "${project}" ]]; then
			continue
		fi

		# Do not strip elf files so debug symbols are available
		# in the firmware_from_source.tar.bz2 bundles from builders.
		dostrip -x "/firmware/${firmware_name}"/zephyr.{rw,ro}.elf

		insinto "/firmware/${firmware_name}"
		doins "${root_build_dir}/${project}"/output/*

		# Install token database to projects that enabled tokenization.
		if [[ -n $(find "${root_build_dir}/${project}" -name "${PROJECT_DB}") ]]; then
			# Install database into firmware binary output for release handling.
			doins "${root_build_dir}/${MERGED_DB}"
		fi
	done < <(cros_config_host "get-firmware-build-combinations" zephyr-ec || die)
}
