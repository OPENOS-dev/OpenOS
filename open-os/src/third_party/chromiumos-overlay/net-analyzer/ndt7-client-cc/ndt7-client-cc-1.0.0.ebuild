# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit cmake flag-o-matic

DESCRIPTION="Utility to provide libndt7 library for network performance test."
HOMEPAGE="https://github.com/m-lab/ndt7-client-cc"
GIT_SHA1="4a371cceaaddd26f595359f811d878c8fa71a2ca"
SRC_URI="https://github.com/m-lab/${PN}/archive/${GIT_SHA1}.zip -> ${PN}-${GIT_SHA1}-${PV}.zip"

LICENSE="Apache-2.0"
SLOT="0/${PVR}"
KEYWORDS="*"
IUSE=""

BDEPEND="app-arch/unzip"

COMMON_DEPEND="
	dev-libs/openssl:=
	net-misc/curl:=
"

DEPEND="
	${COMMON_DEPEND}
	sys-kernel/linux-headers:=
"

RDEPEND="
	${COMMON_DEPEND}
"

PATCHES=(
	"${FILESDIR}/0001-Skip-building-unused-binary.patch"
	"${FILESDIR}/0002-Fix-Remove-whitespace-in-literal-operator.patch"
)

CMAKE_IN_SOURCE_BUILD=1
S="${WORKDIR}/${PN}-${GIT_SHA1}"


src_configure() {
	cros_enable_cxx_exceptions
	cmake_src_configure
}

src_install() {
	cmake_src_install

	insinto /usr/"$(get_libdir)"/pkgconfig
	doins "${FILESDIR}/libndt7.pc"
}

src_test() {
	# Skip tests for testing new changes on upstream.
	:
}
