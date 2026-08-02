# Copyright 2008-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_EXT=1
DISTUTILS_USE_PEP517=setuptools
PYTHON_COMPAT=( python3_{8..12} )

inherit distutils-r1

PARENT_PN="${PN/-python/}"
PARENT_PV="$(ver_cut 2-)"
PARENT_P="${PARENT_PN}-${PARENT_PV}"

if [[ "${PV}" == *9999 ]]; then
	inherit git-r3

	EGIT_REPO_URI="https://github.com/protocolbuffers/protobuf.git"
	EGIT_SUBMODULES=()
	EGIT_CHECKOUT_DIR="${WORKDIR}/${PARENT_P}"
else
	SRC_URI="
		https://github.com/protocolbuffers/protobuf/archive/v${PARENT_PV}.tar.gz
			-> ${PARENT_P}.tar.gz
	"
	KEYWORDS="*"
fi

DESCRIPTION="Google's Protocol Buffers - Python bindings"
HOMEPAGE="
	https://developers.google.com/protocol-buffers/
	https://pypi.org/project/protobuf/
"

LICENSE="BSD"
# The sub-Slot must match the sub-Slot of the core dev-libs/protobuf package,
# which was bumped due to an ABI-breaking upgrade of its abseil-cpp dependency.
SLOT="0/23.3.2"

S="${WORKDIR}/${PARENT_P}/python"

BDEPEND="
	dev-libs/protobuf:${SLOT}
"
RDEPEND="
	${BDEPEND}
	dev-cpp/abseil-cpp:=
"
DEPEND="
	${RDEPEND}
	${PYTHON_DEPS}
"

distutils_enable_tests setup.py

# Same than PATCHES but from repository's root directory,
# please see function `python_prepare_all` below.
# Simplier for users IMHO.
PARENT_PATCHES=(
)

# Here for patches within "python/" subdirectory.
PATCHES=(
	"${FILESDIR}/protobuf-python-4.23.3-cros-abseil.patch"
)

python_prepare_all() {
	pushd "${WORKDIR}/${PARENT_P}" > /dev/null || die
	[[ -n "${PARENT_PATCHES[@]}" ]] && eapply "${PARENT_PATCHES[@]}"
	eapply_user
	popd > /dev/null || die

	distutils-r1_python_prepare_all
}

src_configure() {
	DISTUTILS_ARGS=( --cpp_implementation )
}

python_compile() {
	distutils-r1_python_compile
	find "${BUILD_DIR}" -name "*.pth" -type f -delete || die
}
