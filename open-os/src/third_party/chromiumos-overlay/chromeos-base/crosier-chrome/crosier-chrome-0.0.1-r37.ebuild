# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT=("34a0f840f19578a107cab10605dccfb8879a4f55" "a543b1b9a593daf792eda5317584700ad0e11e55")
CROS_WORKON_TREE=("12da7141ada2e029be2fac3cacb2aa96fa7b1b13" "257c599d32da998f862303bc5587fe80ee66a982")
CROS_WORKON_INCREMENTAL_BUILD=1

CROS_WORKON_PROJECT=(
	"chromiumos/platform2"
	"chromiumos/platform/dev-util"
)
CROS_WORKON_LOCALNAME=(
	"platform2"
	"platform/dev"
)
CROS_WORKON_SUBTREE=(
	"crosier-chrome"
	"test/gtest"
)
CROS_WORKON_DESTDIR=(
	"${S}/platform2"
	"${S}/platform/dev"
)

PYTHON_COMPAT=( python3_11 )

inherit cros-constants cros-workon gtest python-any-r1

DESCRIPTION="Install Crosier tests, metadata and support files"
HOMEPAGE="https://www.chromium.org"
SRC_URI=""

LICENSE="BSD-Google"
SLOT=0
KEYWORDS="*"

IUSE="chrome_internal exclude_crosier"

RDEPEND="
	chromeos-base/chromeos-chrome:=
	chromeos-base/chromeos-fonts:=
	chromeos-base/gestures:=
	chromeos-base/libevdev:=
	>=dev-cpp/gtest-1.10.0:=
	dev-libs/expat:=
	dev-libs/libffi:=
	dev-libs/nspr:=
	dev-libs/nss:=
	media-libs/alsa-lib:=
	media-libs/fontconfig:=
	media-libs/libsync:=
	media-libs/minigbm:=
	media-libs/libcras:=
	net-print/cups:=
	sys-apps/dbus:=
	x11-libs/libdrm:=
	x11-libs/libxkbcommon:=
"
DEPEND="${RDEPEND}"

BDEPEND="
	$(python_gen_any_dep '
		chromeos-base/cros-config-api[${PYTHON_USEDEP}]
		dev-python/jsonschema[${PYTHON_USEDEP}]
		dev-python/protobuf-python[${PYTHON_USEDEP}]
		dev-python/pyyaml[${PYTHON_USEDEP}]
	')
"

python_check_deps() {
	python_has_version -b \
		"chromeos-base/cros-config-api[${PYTHON_USEDEP}]" \
		"dev-python/jsonschema[${PYTHON_USEDEP}]" \
		"dev-python/protobuf-python[${PYTHON_USEDEP}]" \
		"dev-python/pyyaml[${PYTHON_USEDEP}]"
}

# List of Crosier tests to exclude from metadata generation.
# Format: base yaml file names, i.e. "bluetooth_integration_test.yaml"
GTEST_METADATA_EXCLUDE=()

GTEST_TEST_INSTALL_DIR="/usr/libexec/crosier"

src_install() {
	einfo "Deploying Crosier tests"

	exeinto "${GTEST_TEST_INSTALL_DIR}"
	BINARY_DIR="${SYSROOT}/usr/local/build/autotest/client/deps/chrome_test/test_src/out/Release"

	if ! use exclude_crosier; then
		# To save space, we keep the test binary compressed until used.
		ARCHIVE_PATH="${T}/chromeos_integration_tests.tar.bz2"
		tar cvfj "${ARCHIVE_PATH}" -C "${BINARY_DIR}" "chromeos_integration_tests"
		doexe "${ARCHIVE_PATH}"

		doexe "${BINARY_DIR}/fake_chrome"
		doexe "${BINARY_DIR}/test_sudo_helper.py"
		doexe "${BINARY_DIR}/reset_dut.py"
		doexe "${S}/platform2/crosier-chrome/init_env.sh"
		doexe "${S}/platform2/crosier-chrome/run_tests.sh"

		insinto "${GTEST_TEST_INSTALL_DIR}"
		doins "${BINARY_DIR}/libvk_swiftshader.so"
		if use chrome_internal; then
			doins "${BINARY_DIR}/test_accounts.json"
		fi
		doins -r "${BINARY_DIR}/web_handwriting"
		doins -r "${BINARY_DIR}/test_fonts/test_fonts"
		insinto "${GTEST_TEST_INSTALL_DIR}/etc/fonts"
		doins "${BINARY_DIR}/test_fonts/fontconfig/fonts.conf"
	fi

	if [[ ! -d "${BINARY_DIR}"/crosier_metadata ]]; then
		ewarn "Metadata directory: ${BINARY_DIR}/crosier_metadata does not exist."
		return
	fi

	local metadata_files=()
	local f
	# Skip excluded metadata files.
	for f in "${BINARY_DIR}"/crosier_metadata/* ; do
		if ! has "${f##*/}" "${GTEST_METADATA_EXCLUDE[@]}" ; then
			metadata_files+=("${f}")
		fi
	done

	# Generate test metadata.
	einfo "Compiling Crosier metadata files: ${metadata_files[*]}"
	install_all_gtest_metadata "${metadata_files[@]}"
}
