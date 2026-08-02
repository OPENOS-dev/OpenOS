# Copyright 2019 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.makefile file.

EAPI="7"

CROS_WORKON_PROJECT=(
	"chromiumos/platform/ec"
	"chromiumos/third_party/tpm2"
	"chromiumos/third_party/cryptoc"
	"chromiumos/platform/pinweaver"
	"chromiumos/platform/ec"
)
CROS_WORKON_LOCALNAME=(
	"platform/cr50"
	"third_party/tpm2"
	"third_party/cryptoc"
	"platform/pinweaver"
	"platform/gsc-utils"
)
CROS_WORKON_DESTDIR=(
	"${S}/platform/ec-legacy"
	"${S}/third_party/tpm2"
	"${S}/third_party/cryptoc"
	"${S}/platform/pinweaver"
	"${S}/platform/gsc-utils"
)
CROS_WORKON_EGIT_BRANCH=(
	"cr50_stab"
	"main"
	"main"
	"main"
	"gsc_utils"
)

# shellcheck disable=SC2034  # Used by cros-protobuf.eclass
CROS_PROTOBUF_APPLY_DEFAULT_DEPS=0
inherit coreboot-sdk cros-workon cros-toolchain-funcs cros-sanitizers cros-protobuf

DESCRIPTION="Google Security Chip firmware code"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/ec/+/refs/heads/cr50_stab"
MIRROR_PATH="gs://chromeos-localmirror/distfiles/"
CR50_ROS=(cr50.prod.ro.A.0.0.14.hex cr50.prod.ro.B.0.0.14.hex)
SRC_URI="${CR50_ROS[*]/#/${MIRROR_PATH}}"

LICENSE="BSD-Google"
KEYWORDS="~*"
IUSE="asan cros_host fuzzer msan quiet reef ubsan verbose"

# This comes from cros-protobuf.eclass. We don't want to disable the specific
# `shellcheck`, so set a default value to make the linter happy.
: "${CROS_PROTOBUF_DEPS:=}"

COMMON_DEPEND="
	dev-libs/openssl:0=
	virtual/libusb:1=
	fuzzer? ( ${CROS_PROTOBUF_DEPS} )
"

RDEPEND="
	!<chromeos-base/chromeos-ec-0.0.2
	!<chromeos-base/ec-utils-0.0.2
	chromeos-base/chromeos-gsc-dev
	${COMMON_DEPEND}
"

# Need to control versions of chromeos-ec and chromeos-config packages to
# prevent file collision in /firmware/cr50.
DEPEND="
	${COMMON_DEPEND}
	fuzzer? ( dev-libs/libprotobuf-mutator:= )
"

BDEPEND="reef? ( chromeos-base/cr50-utils )"
# We don't want binchecks since we're cross-compiling firmware images using
# non-standard layout.
RESTRICT="binchecks"

# @ECLASS-VARIABLE: COREBOOT_SDK_VERSIONS
# @DESCRIPTION:
#   Associative array of the architectures and their respective versions
#   that should be used by ebuilds inheriting from this eclass.
declare -gA COREBOOT_SDK_VERSIONS=(
    [arm-eabi]="11.3.0-r2/fab0b0cdb09e4b603baaeb22a3e426d9bc0693da"
)

coreboot-sdk_enable arm-eabi

src_unpack() {
	coreboot-sdk_src_unpack
	S+="/platform/ec-legacy"
}

set_build_env() {
	cros_use_gcc

	export CROSS_COMPILE=${COREBOOT_SDK_PREFIX_arm}

	tc-export CC BUILD_CC PKG_CONFIG
	export HOSTCC=${CC}
	export BUILDCC=${BUILD_CC}

	EC_OPTS=( all dis )
	use quiet && EC_OPTS+=( -s 'V=0' )
	use verbose && EC_OPTS+=( 'V=1' )
}

src_compile() {
	set_build_env
	export COREBOOT_SDK_ROOT_arm="${COREBOOT_SDK_PREFIX}"
	export CROSS_COMPILE_CC_NAME=gcc

	export BOARD=cr50

	if use fuzzer ; then
		local sanitizers=()
		use asan && sanitizers+=( 'TEST_ASAN=y' )
		use msan && sanitizers+=( 'TEST_MSAN=y' )
		use ubsan && sanitizers+=( 'TEST_UBSAN=y' )
		emake buildfuzztests "${sanitizers[@]}"
	fi

	if ! use reef; then
		elog "Not building Cr50 binaries"
		return
	fi

	emake clean
	emake "${EC_OPTS[@]}"
	emake "out=build/cr50_ct" "CRYPTO_TEST=1" "${EC_OPTS[@]}"
	emake "out=build/cr50_ct_rb" "CRYPTO_TEST=1" "H1_RED_BOARD=1" \
			"${EC_OPTS[@]}"
}

#
# Install the build artifacts.
#
install_cr50_build_artifacts () {
	local build_dir="${1}"
	local dest_dir="${2}"
	local elf_suffix="${3}"

	einfo "Installing cr50 from ${build_dir} into ${dest_dir}"

	insinto "${dest_dir}"
	doins "${build_dir}/ec.bin"
	doins "${build_dir}/RW/ec.RW.dis"
	doins "${build_dir}/RW/ec.RW.map"
	doins "${build_dir}/RW/ec.RW_B.map"
	doins "${build_dir}/RW/board/cr50/dcrypto/fips_module.o"
	newins "${build_dir}/RW/ec.RW.elf.fips" "ec.RW.elf${elf_suffix}"
	newins "${build_dir}/RW/ec.RW_B.elf.fips" "ec.RW_B.elf${elf_suffix}"
	newins "${build_dir}/RW/updated.manifest.json" "prod.json"
}

#
# Install additional files, necessary for Cr50 signer inputs.
#
install_cr50_signer_aid () {
	local blob

	elog "Installing Cr50 signer support files"

	for blob in "${CR50_ROS[@]}"; do
		local dest_name

		# Carve out prod.ro.? from the RO blob file name. It is known
		# to follow the pattern of "*prod.ro.[AB].x.y.z.hex".
		dest_name="${blob/*prod.ro/prod.ro}"
		newins "${DISTDIR}/${blob}" "${dest_name::9}"
	done

	doins "${S}/board/cr50/rma_key_blob".*.{prod,test}
	doins "${S}/util/signer/fuses.xml"
}

src_configure() {
	sanitizers-setup-env
	default
}

src_install() {
	local build_dir
	local dest_dir

	dosbin "util/chargen"

	if use fuzzer ; then
		local f

		insinto /usr/libexec/fuzzers
		exeinto /usr/libexec/fuzzers
		for f in build/host/*_fuzz/*_fuzz.exe; do
			local fuzzer="$(basename "${f}")"
			local custom_owners="${S}/fuzz/${fuzzer%exe}owners"
			fuzzer="ec_${fuzzer%_fuzz.exe}_fuzzer"
			newexe "${f}" "${fuzzer}"
			einfo "CUSTOM OWNERS = '${custom_owners}'"
			if [[ -f "${custom_owners}" ]]; then
				newins "${custom_owners}" "${fuzzer}.owners"
			else
				newins "${S}/OWNERS" "${fuzzer}.owners"
			fi
		done
	fi

	if ! use cros_host; then
		exeinto /usr/local/bin
		doexe "util/ap_ro_hash.py"
	fi

	if ! use reef; then
		elog "Not installing Cr50 binaries"
		return
	fi

	install_cr50_build_artifacts "build/cr50" "/firmware/cr50" ""

	install_cr50_signer_aid

	# Save the CRYPTO_TEST artifacts, so it's easy to run crypto tests for
	# this build. CRYPTO_TEST images are only going to be dev signed, so
	# there's no need to install signer artifacts.
	# The crypto test build should not be prod signed. Do a couple of things
	# to make sure the signer ignores it. Rename it to "crypto_test" so "50"
	# isn't in the name. The signer searches for "50" to find the build
	# artifacts, so it should ignore crypto_test. Change the elf filenames
	# too just to be safe.
	install_cr50_build_artifacts "build/cr50_ct" "/firmware/crypto_test" \
			".test"
	install_cr50_build_artifacts "build/cr50_ct_rb" \
			"/firmware/crypto_test_rb" ".test"
}
