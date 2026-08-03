# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# Bootstraps newer versions of rustc from older versions of rustc. In the past,
# this package was bootstrapped from the `mrustc` compiler, to avoid trusting
# trust attacks. We use this as our stage0 compiler for `dev-lang/rust`, rather
# than downloading an upstream bootstrap binary.
#
# The version of this ebuild reflects the version of rustc that will
# ultimately be installed.

EAPI=7

inherit toolchain-funcs

DESCRIPTION="Bootstraps the rustc Rust compiler using mrustc"
HOMEPAGE="https://github.com/thepowersgang/mrustc"

SLOT="${PV}"
KEYWORDS="*"

# THIS_VERSION_PREBUILT_NAME is the name of the prebuilt to pull from
# localmirror. Generally speaking, this is `rust-bootstrap-${PV}.tbz2`,
# but may include `-r${N}` suffixes in rare cases.
#
# If empty, this ebuild will compile and install from source, rather than
# just installing a prebuilt.
THIS_VERSION_PREBUILT_NAME=rust-bootstrap-1.91.0-r2.tbz2

# PRIOR_RUST_BOOTSTRAP_VERSION is the version of rust-bootstrap we need in
# order to build the current version of rust-bootstrap.
PRIOR_RUST_BOOTSTRAP_VERSION="1.90.0"

LICENSE="MIT Apache-2.0 BSD-1 BSD-2 BSD-4 UoI-NCSA"

is_only_installing_prebuilt() {
	[[ -n "${THIS_VERSION_PREBUILT_NAME}" ]]
}

# NOTE: We have to mirror rustc's sources, even if we're only copying prebuilts
# around. This is because our prebuilt tarballs contain no licenses, and portage
# requires licenses.
SRC_URI="gs://chromeos-localmirror/distfiles/rustc-${PV}-src.tar.gz"

if is_only_installing_prebuilt; then
	SRC_URI+=" gs://chromeos-localmirror/distfiles/${THIS_VERSION_PREBUILT_NAME}"
else
	# If this package isn't simply installing a prebuilt, we need to BDEPEND
	# on the parent rust-bootstrap, to have something to build from. The
	# expectation is that `is_only_installing_prebuilt` will essentially
	# always be true in production, so this should rarely pull in the prior
	# rust-bootstrap package.
	BDEPEND="
		dev-lang/rust-bootstrap:${PRIOR_RUST_BOOTSTRAP_VERSION}
		dev-util/cmake
		dev-util/ninja
		net-misc/curl
		sys-libs/libcxx
	"
fi

DEPEND="dev-libs/openssl"
RDEPEND="${DEPEND}"

# These tasks take a long time to run for not much benefit: Most of the files
# they check are never installed. Those that are are only there to bootstrap
# the rust ebuild, which has the same RESTRICT anyway.
RESTRICT="binchecks strip"

pkg_setup() {
	# We manually apply patches to rustc, so that we can pass the necessary
	# -p value & be in the correct directory. To prevent default from
	# trying and failing to apply patches, we set PATCHES to empty.
	PATCHES=( )
	S="${WORKDIR}/rustc-${PV}-src"
}

src_prepare() {
	# Call the default implementation. This applies PATCHES.
	default

	is_only_installing_prebuilt && return
	einfo "No rust-bootstrap prebuilt available; building from source"

	einfo "Patching rustc-${PV}"
	(
		cd "${WORKDIR}/rustc-${PV}-src" || die
		eapply -p2 "${FILESDIR}/${PN}-1.48.0-libc++.patch"
		eapply "${FILESDIR}/${PN}-opt-out-of-self-contained-linkers.patch"
	)

	# In order to build rustc with host=x86_64-pc-linux-gnu, the
	# bootstrap compiler needs to recognize x86_64-pc-linux-gnu.
	local host_patch="${FILESDIR}/rust-bootstrap-add-host-target.patch"
	local specs_dir="${WORKDIR}/rustc-${PV}-src/compiler/rustc_target/src/spec/targets"
	(cd "${WORKDIR}/rustc-${PV}-src" || die; eapply -p1 "${host_patch}")
	(
		cd "${specs_dir}" &&
		sed -e 's:"unknown":"pc":g' x86_64_unknown_linux_gnu.rs >x86_64_pc_linux_gnu.rs
	) || die
}

src_configure() {
	# Avoid the default implementation, which overwrites vendored
	# config.guess and config.sub files, which then causes checksum
	# errors during the build, e.g.
	# error: the listed checksum of `/var/tmp/portage/dev-lang/rust-bootstrap-1.46.0/work/rustc-1.46.0-src/vendor/backtrace-sys/src/libbacktrace/config.guess` has changed:
	# expected: 12e217c83267f1ff4bad5d9b2b847032d91e89ec957deb34ec8cb5cef00eba1e
	# actual:   312ea023101dc1de54aa8c50ed0e82cb9c47276316033475ea403cb86fe88ffe
	# (The dev-lang/rust ebuilds in Chrome OS and Gentoo also have custom
	# src_configure implementations.)
	true
}

# Prints the absolute path to the given tool, or `die`s.
tool_abspath() {
	local cmd tool="$1"
	cmd="$(type -P "${tool}")" || die "Couldn't look up ${tool}"
	realpath --no-symlinks "${cmd}" || die
}

src_compile() {
	is_only_installing_prebuilt && return

	tc-export CC PKG_CONFIG

	# ccache llvm builds if FEATURES=ccache is set. Don't set it to true
	# unconditionally, since having `ccache=true` slows builds down by a
	# handful of seconds, even with `CCACHE_DISABLE=true` set.
	local use_ccache=false
	[[ -z "${CCACHE_DISABLE:-}" ]] && use_ccache=true

	local ar ranlib
	# These need to be absolute paths for Rust to accept them; see
	# discussion on b/277968325 comments 6-10.
	ar="$(tool_abspath "$(tc-getBUILD_AR)")"
	ranlib="$(tool_abspath "$(tc-getBUILD_RANLIB)")"

	einfo "Building rustc-${PV} using rustc-${PRIOR_RUST_BOOTSTRAP_VERSION}"
	local rustc_dir="${WORKDIR}/rustc-${PV}-src"
	cd "${rustc_dir}" || die "Could not chdir to ${rustc_dir}"
	cat > config.toml <<EOF
[build]
cargo = "/opt/rust-bootstrap-${PRIOR_RUST_BOOTSTRAP_VERSION}/bin/cargo"
rustc = "/opt/rust-bootstrap-${PRIOR_RUST_BOOTSTRAP_VERSION}/bin/rustc"
docs = false
vendor = true
# extended means we also build cargo and a few other commands.
extended = true
target = ["x86_64-unknown-linux-gnu", "x86_64-pc-linux-gnu"]
# For rust-bootstrap, we need only cargo.
tools = [ "cargo" ]
ccache = ${use_ccache}

[install]
prefix = "${ED}/opt/rust-bootstrap-${PV}"

[rust]
default-linker = "${CC}"
download-rustc = false
llvm-tools = false

[llvm]
download-ci-llvm = false
# For rust-bootstrap, we only need x86_64, which LLVM calls X86.
experimental-targets = ""
targets = "X86"
static-libstdcpp = false

[target.x86_64-unknown-linux-gnu]
ar = "${ar}"
cc = "${CC}"
cxx = "${CXX}"
linker = "${CC}"
ranlib = "${ranlib}"

[target.x86_64-pc-linux-gnu]
ar = "${ar}"
cc = "${CC}"
cxx = "${CXX}"
linker = "${CC}"
ranlib = "${ranlib}"
EOF

	# --stage 2 causes this to use the previously-built compiler,
	# instead of the default behavior of downloading one from
	# upstream.
	./x.py --stage 2 build || die

	# Now fix up the `build_dir` to remove bits that are unused
	# Note that the triple here (& in src_install) is the host triple.
	local build_dir="${WORKDIR}/rustc-${PV}-src/build/x86_64-unknown-linux-gnu/stage2"

	# Remove the src/rust symlink which will be dangling after sources are
	# removed, and the containing src directory.
	rm "${build_dir}/lib/rustlib/src/rust" || die
	rmdir "${build_dir}/lib/rustlib/src" || die

	# Anything in rustlib/ that isn't rustlib/*/lib isn't useful (this is mostly
	# toolchain binaries like `rust-lld`).
	rm -rf \
		"${build_dir}/lib/rustlib/"*"/bin" \
		"${build_dir}/lib/rustlib/"*"/codegen-backends" \
		|| die
}

src_install() {
	local obj tools
	if is_only_installing_prebuilt; then
		obj="${WORKDIR}/opt/rust-bootstrap-${PV}"
		tools="${obj}/bin"
	else
		local build_root="${WORKDIR}/rustc-${PV}-src/build/x86_64-unknown-linux-gnu"
		obj="${build_root}/stage2"
		tools="${build_root}/stage2-tools/x86_64-unknown-linux-gnu/release"
	fi
	exeinto "/opt/${P}/bin"
	# With rustc-1.45.2 at least, regardless of the value of install.libdir,
	# the rpath seems to end up as $ORIGIN/../lib. So install the libraries there.
	insinto "/opt/${P}/lib"
	doexe "${obj}/bin/rustc"
	doexe "${tools}/cargo"
	doins -r "${obj}/lib/"*
	find "${D}" -name '*.so' -exec chmod +x '{}' ';'
}
