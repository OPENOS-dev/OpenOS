# Copyright 2018 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

inherit cmake cros-protobuf

COMMIT="3b28530531b154a748fe9884bc9219b4966f0754"
DESCRIPTION="Library to randomly mutate protobuffers."
HOMEPAGE="https://github.com/google/libprotobuf-mutator"
SRC_URI="https://github.com/google/${PN}/archive/${P}.tar.gz"

LICENSE="Apache-2.0"
SLOT="0"
KEYWORDS="*"
IUSE="msan +static-libs"
RESTRICT="test"

PATCHES=( "${FILESDIR}/fix-msan.patch" )

S="${WORKDIR}/${PN}-${COMMIT}"

src_prepare() {
	cmake_src_prepare
	sed -i -e 's/set(CMAKE_CXX_STANDARD 14)/set(CMAKE_CXX_STANDARD 20)/g' \
		"${S}/CMakeLists.txt" || die
}

src_configure() {
	local mycmakeargs=(
		-DTHREADS_PTHREAD_ARG=-pthread
		-DBUILD_SHARED_LIBS:BOOL=$(usex static-libs OFF ON)
		-DCMAKE_CXX_STANDARD=20
	)

	cmake_src_configure
}

src_compile() {
	cmake_build protobuf-mutator protobuf-mutator-libfuzzer
}

src_install() {
	local inst_dir="/usr/include/libprotobuf-mutator"
	insinto "${inst_dir}/port"
	doins port/*.h
	insinto "${inst_dir}/src"
	doins src/*.h
	insinto "${inst_dir}/src/libfuzzer"
	doins src/libfuzzer/*.h

	insinto /usr/share/pkgconfig
	doins "${FILESDIR}/${PN}.pc"

	local base_lib="${BUILD_DIR}/src/libprotobuf-mutator"
	local fuzzer_lib="${BUILD_DIR}/src/libfuzzer/libprotobuf-mutator-libfuzzer"
	if use static-libs; then
		dolib.a "${base_lib}.a"
		dolib.a "${fuzzer_lib}.a"
	else
		dolib.so "${base_lib}.so"
		dolib.so "${fuzzer_lib}.so"
	fi
}
