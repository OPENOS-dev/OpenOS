# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2.

EAPI=7

CROS_WORKON_COMMIT="ce4bed460493919c8f1c7ea614856d2fd79933d2"
CROS_WORKON_TREE="70e996d49f0a4c553f3509ae270e99f9f032d16c"
CROS_WORKON_PROJECT="chromiumos/third_party/pigweed/pigweed"
CROS_WORKON_LOCALNAME="third_party/pigweed"

PYTHON_COMPAT=( python3_11 )

DISTUTILS_USE_PEP517=setuptools
inherit cros-workon distutils-r1

DESCRIPTION="Sustained, robust, and rapid embedded product development for large teams"
HOMEPAGE="https://pigweed.dev/"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE=""

# pw_tokenizer moving into this package
RDEPEND="
	!chromeos-base/pw_tokenizer
"

# Build Dependencies
BDEPEND="
	cross-arm-none-eabi/gcc
	dev-libs/protobuf
	dev-util/ninja
	dev-util/gn
"

pkg_setup() {
	cros-workon_pkg_setup
	python_setup
}

src_prepare() {
	# Setup Pigweed environment
	export PW_ROOT="${S}"
	mkdir "${S}/local" || die
	cp "${FILESDIR}/BUILD.gn" "${PW_ROOT}/local/BUILD.gn" || die
	cp "${FILESDIR}/pigweed.gni" "${PW_ROOT}/build_overrides/pigweed.gni" || die
	cp "${FILESDIR}/pigweed_environment.gni" "${PW_ROOT}/build_overrides/pigweed_environment.gni" || die

	eapply_user
}

src_configure() {
	cd "${S}" || die

	GN_ARGS=(
		"pw_build_PYTHON_BUILD_VENV=\"//local:empty_build_venv\""
		"pw_protobuf_compiler_GENERATE_PROTOS_ARGS=\"--no-experimental-editions\""
		"pw_protobuf_compiler_GENERATE_PYTHON_TYPE_HINTS=false"
		"pw_env_setup_CIPD_ARM=\"//\""
		"pw_env_setup_CIPD_MSRV_PYTHON=\"${PYTHON}\""
		"pw_env_setup_CIPD_PYTHON311=\"${PYTHON}\""
	)

	echo "GN_ARGS=${GN_ARGS[*]}"

	# Generate Pigweed Python Source Tree
	gn gen out --args="${GN_ARGS[*]}" || die
}

src_compile() {
	cd "${S}" || die

	TARGET="$(gn ls out --as=output \
		'//pw_env_setup:pypi_pigweed_python_source_tree(//pw_build/python_toolchain:python)')" || die
	ninja -C out "${TARGET}" || die

	# Path to Pigweed Python source tree
	local PW_PYTHON_OUT="out/python/obj/pw_env_setup/pypi_pigweed_python_source_tree"

	# Avoid ebuild collision with arm-none-eabi-gdb
	sed -i '/arm-none-eabi-gdb/d' "${PW_PYTHON_OUT}"/setup.cfg || die

	cd "${S}/${PW_PYTHON_OUT}" || die

	distutils-r1_src_compile
}
