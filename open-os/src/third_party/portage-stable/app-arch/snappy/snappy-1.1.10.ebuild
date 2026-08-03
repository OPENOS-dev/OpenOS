# Copyright 1999-2021 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CMAKE_ECLASS=cmake
inherit cmake-multilib

DESCRIPTION="A high-speed compression/decompression library by Google"
HOMEPAGE="https://github.com/google/snappy"

BUNDLED_BENCHMARK_SHA=d572f4777349d43653b21d6c2fc63020ab326db2
BUNDLED_GOOGLETEST_SHA=b796f7d44681514f58a683a3a71ff17c94edb0c1

SRC_URI="
	https://github.com/google/${PN}/archive/${PV}.tar.gz -> ${P}.tar.gz
	https://github.com/google/benchmark/archive/${BUNDLED_BENCHMARK_SHA}.tar.gz -> benchmark-${BUNDLED_BENCHMARK_SHA}.tar.gz
	https://github.com/google/googletest/archive/${BUNDLED_GOOGLETEST_SHA}.tar.gz -> googletest-${BUNDLED_GOOGLETEST_SHA}.tar.gz
"

LICENSE="BSD"
SLOT="0/${PV%%.*}"
KEYWORDS="*"
IUSE="test"
RESTRICT="!test? ( test )"

# all test dependencies are optional:
# - gflags-2.2 is supposedly needed for command-line option parsing
# but it's a huge hack and does not work,
# - gtest probably gives nicer output,
# - compression libraries are used for benchmarks which we do not run.
DEPEND="test? ( dev-cpp/gtest )"

# AUTHORS is useless, ChangeLog is stale
DOCS=( format_description.txt framing_format.txt NEWS README.md )

src_prepare() {
	local PATCHES=(
		"${FILESDIR}"/snappy-1.1.10-0001-cmake-sign-compare.patch
		"${FILESDIR}"/snappy-1.1.10-0002-cmake-remove-no-rtti.patch
	)

	# Snappy has benchmark and googletest as bundled submodules, but the tarball
	# only has empty folders for them. Prepare maually.
	cp -rl "${WORKDIR}/benchmark-${BUNDLED_BENCHMARK_SHA}"/* "third_party/benchmark"
	cp -rl "${WORKDIR}/googletest-${BUNDLED_GOOGLETEST_SHA}"/* "third_party/googletest"

	# command-line option parsing does not work at all, so just force
	# it off
	sed -i -e '/run_microbenchmarks/s:true:false:' snappy-test.cc || die

	cmake_src_prepare
}

multilib_src_configure() {
	# TODO: would be nice to make unittest build conditional
	# but it is not a priority right now
	local mycmakeargs=(
		-DBUILD_SHARED_LIBS=ON
		# Disable benchmarks and test
		-DSNAPPY_BUILD_BENCHMARKS=OFF
		-DSNAPPY_BUILD_TESTS=OFF

		# Disalbe arm neon because arm32 boards do not support.
		-DSNAPPY_HAVE_NEON=0

		# we do not want to run benchmarks, and those are only used
		# for benchmarks
		-DHAVE_LIBZ=NO
		-DHAVE_LIBLZO2=NO
	)
	cmake_src_configure
}

multilib_src_test() {
	# run tests directly to get verbose output
	cd "${S}" || die
	"${BUILD_DIR}"/snappy_unittest || die
}
