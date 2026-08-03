# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# @ECLASS: cros-zephyr-utils.eclass
# @MAINTAINER:
# ChromiumOS Firmware Team
# @BUGREPORTS:
# Please report bugs via
# https://b.corp.google.com/issues/new?component=1037860&template=1600056
# @VCSURL: https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/eclass/@ECLASS@
# @BLURB: helper eclass for building ChromiumOS firmware
# @DESCRIPTION:
# Common helper functions for working with ChromiumOS Zephyr EC firmware.

if [[ -z "${_ECLASS_CROS_ZEPHYR_UTILS}" ]]; then
_ECLASS_CROS_ZEPHYR_UTILS="1"

# Check for EAPI 7+.
case "${EAPI:-0}" in
0|1|2|3|4|5|6) die "unsupported EAPI (${EAPI}) in eclass (${ECLASS})" ;;
*) ;;
esac

PYTHON_COMPAT=( python3_11 )

inherit cros-constants cros-protobuf cros-unibuild cros-toolchain-funcs python-any-r1 coreboot-sdk

LICENSE="Apache-2.0 BSD-Google"
IUSE="cros_host unibuild"
REQUIRED_USE="|| ( cros_host unibuild )"

BDEPEND="
	chromeos-base/zephyr-build-tools
	chromeos-base/vboot_reference
	dev-embedded/binman
	$(python_gen_any_dep '
		dev-python/docopt[${PYTHON_USEDEP}]
		dev-python/pykwalify[${PYTHON_USEDEP}]
		dev-python/packaging[${PYTHON_USEDEP}]
	')
	dev-util/cmake
	dev-util/ninja
"
# TODO(b/332969373): Make it so binman doesn't depend on gcc.
BDEPEND+="
	sys-devel/gcc
"

cros-zephyr-utils-python_check_deps() {
	python_has_version -b \
		"dev-python/docopt[${PYTHON_USEDEP}]" \
		"dev-python/pykwalify[${PYTHON_USEDEP}]" \
		"dev-python/packaging[${PYTHON_USEDEP}]"
}

# If another eclass or ebuild needs to override this method, it should make sure
# to call cros-zephyr-utils-python_check_deps.
python_check_deps() {
	cros-zephyr-utils-python_check_deps
}

RDEPEND="${DEPEND}"

echoit() {
	einfo "$@"
	"$@"
}

# Run zmake from the EC source directory, with default arguments for
# modules and Zephyr base location for this ebuild.
run_zmake() {
	echoit env PYTHONPATH="${S}/modules/ec/zephyr/zmake" \
		COREBOOT_SDK_ROOT="${COREBOOT_SDK_PREFIX}" "${EPYTHON}" -m zmake -D \
		--modules-dir="${S}/zephyrproject/modules" \
		--zephyr-base="${S}/zephyrproject/zephyr" \
		"$@"
}


# @FUNCTION: cros-version
# @USAGE: cros-version
# @DESCRIPTION:
# Returns the ChromeOS version string from chromeos_version.sh
cros-version() {
	"${CHROOT_SOURCE_ROOT}"/src/third_party/chromiumos-overlay/chromeos/config/chromeos_version.sh |
		grep "^[[:space:]]*CHROMEOS_VERSION_STRING=" |
		cut -d= -f2 |
		tr - _
}

# @FUNCTION: cros-zephyr-compile
# @USAGE: cros-zephyr-compile <target>
# @DESCRIPTION:
# Compile Zephyr by specified target in chromeos-config.
#
# <target>: zephyr-ec or zephyr-detachable-base for Zephyr target.
cros-zephyr-compile() {
	tc-export CC

	local project
	local target=$1
	local projects=()

	if [[ -z "${target}" ]]; then
		die "Please specify zephyr build target"
	fi

	while read -r _ && read -r project; do
		if [[ -z "${project}" ]]; then
			continue
		fi

		projects+=("${project}")
	done < <(cros_config_host "get-firmware-build-combinations" "${target}" || die)
	if [[ ${#projects[@]} -eq 0 ]]; then
		einfo "No projects found."
		return
	fi
	run_zmake build -B "build" --version "$(cros-version)" --static \
		"${projects[@]}" ||
		die "Failed to build ${projects[*]}."
}

fi  # _ECLASS_CROS_ZEPHYR_UTILS
