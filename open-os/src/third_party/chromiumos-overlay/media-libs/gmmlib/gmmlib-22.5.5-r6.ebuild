# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="86040c51b1452e0686650cf8019536deabccbfa3"
CROS_WORKON_TREE="f902f4c7195aaca1988d3116e8c6abb20fdab46d"
CMAKE_BUILD_TYPE="Release"

CROS_WORKON_PROJECT="chromiumos/third_party/gmmlib"
CROS_WORKON_LOCALNAME="gmmlib"
CROS_WORKON_EGIT_BRANCH="chromeos"

inherit cmake cros-workon

DESCRIPTION="Intel Graphics Memory Management Library"
HOMEPAGE="https://github.com/intel/gmmlib"

KEYWORDS="*"
LICENSE="MIT"
SLOT="0/12.1"
IUSE="+custom-cflags test"
RESTRICT="!test? ( test )"

PATCHES=(
	"${FILESDIR}"/${PN}-20.2.2_conditional_testing.patch
	"${FILESDIR}"/${PN}-20.3.2_cmake_project.patch
	"${FILESDIR}"/${PN}-22.1.1_custom_cflags.patch
)

src_configure() {
	# shellcheck disable=SC2034
	local mycmakeargs=(
		-DBUILD_TESTING="$(usex test)"
		-DBUILD_TYPE="Release"
		-DOVERRIDE_COMPILER_FLAGS="$(usex !custom-cflags)"
	)

	cmake_src_configure
}
