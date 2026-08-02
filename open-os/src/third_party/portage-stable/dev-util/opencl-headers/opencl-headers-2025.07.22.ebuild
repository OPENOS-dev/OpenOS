# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit cmake

MY_PN="OpenCL-Headers"
MY_P="${MY_PN}-${PV}"

DESCRIPTION="Unified C language headers for the OpenCL API"
HOMEPAGE="https://github.com/KhronosGroup/OpenCL-Headers"
SRC_URI="https://github.com/KhronosGroup/${MY_PN}/archive/refs/tags/v${PV}.tar.gz -> ${P}.tar.gz"

LICENSE="Apache-2.0"
SLOT="0"
KEYWORDS="*"
IUSE="test"

RESTRICT="!test? ( test )"

S="${WORKDIR}"/${MY_P}

PATCHES=(
	"${FILESDIR}/opencl-headers-00-5d52989.patch"
	"${FILESDIR}/opencl-headers-01-0c54eeb.patch"
)

src_configure() {
	local mycmakeargs=(
		-DBUILD_TESTING=$(usex test)
	)
	cmake_src_configure
}
