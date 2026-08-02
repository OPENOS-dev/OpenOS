# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("ff0990d6462bf08e9e8bf40e48ea7ddcd185b569" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_SUBTREE="soul/gravedigger common-mk .gn"
CROS_WORKON_DESTDIR="${S}/platform2"
PLATFORM_SUBDIR="soul/gravedigger"

inherit cros-workon cros-rust platform

DESCRIPTION="Utility library for log files"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/soul/gravedigger/"

LICENSE="BSD-Google"
SLOT="0/${PVR}"
KEYWORDS="*"

IUSE="test"

DEPEND="
	chromeos-base/libchrome:=
	dev-rust/libchromeos:=
	dev-rust/third-party-crates-src:=
"

BDEPEND="dev-util/cxxbridge-cmd"

# (crbug.com/1182669): build-time only deps need to be in RDEPEND so they are
# pulled in when installing binpkgs since the full source tree is required to
# use the crate.
RDEPEND="
	${DEPEND}
	${BDEPEND}
"

src_unpack() {
	platform_src_unpack
	cros-rust_src_unpack
}

src_configure() {
	rust_build_dir="${WORKDIR}"
	if [[ -n "${CROS_WORKON_PROJECT}" ]]; then
		# Use a sub directory to avoid unintended interactions with platform.eclass.
		rust_build_dir="$(cros-workon_get_build_dir)/cros-rust"
		mkdir -p "${rust_build_dir}"
	fi
	export RUST_BUILD_DIR="${rust_build_dir}/${CHOST}/release"
	platform_src_configure
	cros-rust_src_configure
}

src_compile() {
	# Check if cxxflags has -fno-exceptions and set -DRUST_CXX_NO_EXCEPTIONS
	# This is required to build the cxx rust dependency.
	if is-flagq -fno-exceptions; then
		append-cxxflags -DRUST_CXX_NO_EXCEPTIONS
	fi
	# Rust code has to be built before C++ because the C++ part expects a library
	# from Rust.
	local features=(
		chromeos
	)
	ecargo_build -v \
		--features="${features[*]}" ||
		die "cargo build failed"

	platform_src_compile
}

platform_pkg_test() {
	platform_test run "${OUT}/gravedigger_test"
}

src_test() {
	platform_src_test
	# Qemu version 8.2 crashes with QEMU internal SIGSEGV {code=MAPERR, addr=0x20}
	# on multiple architectures. Since this seems to affect mostly ARM boards in
	# the CQ let's disable the tests for now.
	# b/333315918
	if use arm || use arm64 ; then
		einfo "Skipping gravedigger rust unit tests on ARM platform"
	else
		cros-rust_src_test
	fi
}

src_install() {
	# This comes from cros-rust.eclass. We don't want to disable the specific
	# `shellcheck`, so set a default value to make the linter happy.
	: "${CARGO_TARGET_DIR:=}"
	cros-rust_src_install
	dolib.a "${CARGO_TARGET_DIR}/${CHOST}/release/libgravedigger_rs.a"
	platform_src_install
}
