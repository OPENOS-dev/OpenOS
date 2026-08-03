# Copyright 2020-2026 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

PYTHON_COMPAT=( python3_{6..12} )

inherit cmake-multilib cros-debug cros-sanitizers flag-o-matic python-any-r1

DESCRIPTION="Abseil Common Libraries (C++), LTS Branch"
HOMEPAGE="https://abseil.io/"
SRC_URI="https://github.com/abseil/abseil-cpp/archive/${PV}.tar.gz -> ${P}.tar.gz"

LICENSE="Apache-2.0"
SLOT="0/${PVR}"
KEYWORDS="*"
IUSE="test"

DEPEND="
	test? (
		>=dev-cpp/gtest-1.13.0
	)
"
RDEPEND="${DEPEND}
	!dev-cpp/absl
"

BDEPEND="
	${PYTHON_DEPS}
	test? (
		sys-libs/timezone-data
	)
"

RESTRICT="!test? ( test )"

PATCHES=(
	"${FILESDIR}"/use-std-optional.patch
	"${FILESDIR}"/abseil-cpp-20240116.3-dll.patch
	"${FILESDIR}"/abseil-cpp-20240116.3-cord-test-fix.patch
	"${FILESDIR}"/abseil-cpp-20240116.3-symbolize-test-no-icf.patch
)

MAJOR=$(ver_cut 1)
SOVERSION="${MAJOR:2:4}.0.0"

src_prepare() {
	# ChromeOS: force the `mutex.h` header to expose the release version when
	# the `cros-debug` USE flag is not set.
	#
	# Note that a modified version of this patch was upstreamed on November 5,
	# 2025 to Abseil. Therefore, in the future when ChromeOS updates its Abseil
	# LTS copy to a version that is newer than 20251105, this patch needs to be
	# removed.
	use cros-debug || eapply "${FILESDIR}/abseil-cpp-20240116.3-delete-mutex-dtor.patch"

	cmake_src_prepare

	# un-hardcode abseil compiler flags
	sed -i \
		-e '/"-maes",/d' \
		-e '/"-msse4.1",/d' \
		-e '/"-mfpu=neon"/d' \
		-e '/"-march=armv8-a+crypto"/d' \
		absl/copts/copts.py || die

	# now generate cmake files
	python_fix_shebang absl/copts/generate_copts.py
	absl/copts/generate_copts.py || die

	# ChromeOS (b/264420866): Enable a "hardened" build.
	sed -i 's/^#define ABSL_OPTION_HARDENED 0/#define ABSL_OPTION_HARDENED 1/' \
		absl/base/options.h || die

	# ChromeOS: This is the core of the C++20 transition strategy. The new
	# Abseil library is built with C++20, but this sed command modifies its
	# headers to hide C++20-specific features (like std::ordering) from
	# downstream dependencies. This allows other packages that are still on
	# C++17 to link against the new Abseil without being forced to migrate
	# to C++20 immediately.
	sed -i 's/^#define ABSL_OPTION_USE_STD_ORDERING .*/#define ABSL_OPTION_USE_STD_ORDERING 0/' \
		absl/base/options.h || die
}

multilib_src_configure() {
	append-lfs-flags
	# ChromeOS: Prevent exporting inline symbols to improve startup speed.
	#           (go/cros-symbol-slimming)
	append-cxxflags -fvisibility-inlines-hidden
	# ChromeOS: Don't build in debug mode unless the `cros-debug` USE flag is set.
	cros-debug-add-NDEBUG
	# ChromeOS: Assume no interposition and pre-bind DSO-local symbols to
	#           improve startup speed. (go/cros-symbol-slimming)
	append-ldflags -Wl,-Bsymbolic-non-weak
	sanitizers-setup-env  # ChromeOS-specific asan fix.
	# shellcheck disable=SC2034 # Used by cmake.eclass
	local mycmakeargs=(
		# We use `>= c++17` here so that std::string_view is used.
		-DCMAKE_CXX_STANDARD=20
		-DABSL_ENABLE_INSTALL=TRUE
		-DABSL_USE_EXTERNAL_GOOGLETEST=ON
		-DABSL_PROPAGATE_CXX_STD=TRUE
		# ChromeOS: Enable building a monolithic libabseil_dll.so which
		#           significantly reduces the overhead of symbol resolution
		#           inside ld.so. See go/cros-absl-monolithic-so for a complete
		#           explanation and benchmark numbers.
		-DABSL_BUILD_MONOLITHIC_SHARED_LIBS=ON
		-DABSL_BUILD_TEST_HELPERS=$(usex test ON OFF)
		-DABSL_BUILD_TESTING=$(usex test ON OFF)
		$(usex test -DBUILD_TESTING=ON '') # intentional usex, it used both variables for tests.
	)

	cmake_src_configure
}

multilib_src_compile() {
	cmake_src_compile

	sed -e "s/@LIBS@/-labseil_dll/g" -e "s/@PV@/${PV}/g" \
		"${FILESDIR}/absl.pc.in" > absl.pc || die
}

multilib_src_install() {
	cmake_src_install

	insinto "/usr/$(get_libdir)/pkgconfig"
	doins absl.pc

	# Compat symlinks from old separate .so files to monolithic libabseil_dll.so.
	# This is needed for packages that pass -labsl_foo flags directly and for
	# gradual migration of binpkgs. The added `if` checks make this more
	# robust than the previous ebuild by avoiding errors if links already exist.
	local targets=( "${D}/usr/$(get_libdir)/pkgconfig/"absl_*.pc )
	local t libname
	for t in "${targets[@]}"; do
		libname="lib$(basename "${t}" .pc).so"
		if [[ ! -f "${D}/usr/$(get_libdir)/${libname}" ]]; then
			ln -sv "libabseil_dll.so.${SOVERSION}" "${D}/usr/$(get_libdir)/${libname}" || die
		fi
		if [[ ! -f "${D}/usr/$(get_libdir)/${libname}.${SOVERSION}" ]]; then
			ln -sv "libabseil_dll.so.${SOVERSION}" "${D}/usr/$(get_libdir)/${libname}.${SOVERSION}" || die
		fi
	done

	# absl adds all the Cflags to all the pc files even though almost all the
	# .pc files require absl_config.pc. This causes an explosion of flags when
	# including multiple abseil sub-libraries through pkg-config. Until the
	# issue is fixed upstream, strip them out here since this is easier to
	# maintain than a version specific patch.
	find "${D}/usr/$(get_libdir)/pkgconfig" -type f -not -name 'absl_config.pc' -print0 | xargs -0 sed -i '/^Cflags: /d' || die

	# As part of the move to a monolithic shared library, this ensures that all
	# individual pkg-config files (e.g., absl_strings.pc) are symlinked to a
	# single, unified absl.pc file. This simplifies the build process for
	# dependent packages and reinforces the monolithic library approach.
	local pc_file
	for pc_file in "${D}/usr/$(get_libdir)/pkgconfig"/absl_*.pc; do
		ln -sf absl.pc "${pc_file}" || die
	done
}

multilib_src_test() {
	# When building with monolithic shared libraries, the test executables need
	# to find the newly built .so files at runtime. The dynamic linker does not
	# search the build directory by default, so we must explicitly add the
	# library output directories to the LD_LIBRARY_PATH.
	#
	# We use BUILD_DIR here instead of CBUILD_DIR because in a multilib context,
	# BUILD_DIR correctly points to the ABI-specific build directory (e.g.,
	# ..._build-abi_x86_64.amd64) where the libraries for the current
	# architecture are located.
	local lib_dir="${BUILD_DIR}/absl"
	export LD_LIBRARY_PATH="${lib_dir}:${LD_LIBRARY_PATH}"
	cmake_src_test
}
