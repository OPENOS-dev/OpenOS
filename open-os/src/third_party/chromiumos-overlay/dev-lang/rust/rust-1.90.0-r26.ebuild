# Copyright 1999-2018 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

EAPI=7

# NOTE(b/197889836): Once we converge Rust's LLVM with sys-devel/llvm's, these
# will have more exciting values.
CROS_WORKON_COMMIT="d2d95e8af89939f893b1443135497c1f5572aebc"
CROS_WORKON_TREE="776139a53bc86333de8672a51ed7879e75909ac9"
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

# shellcheck disable=SC2034 # Used by cros-rustc.eclass
CROS_RUSTC_BOOTSTRAP_FROM_RUST_HOST=1

# shellcheck disable=SC2034
PYTHON_COMPAT=( python3_{6..11} )

inherit cros-workon cros-rustc

BDEPEND="sys-devel/llvm"
KEYWORDS="*"

# dev-lang/rust was emptied out after r5. Now its artifacts are installed by
# cross-*/rust packages.
RDEPEND="
	!<=dev-lang/rust-1.73.0-r5
"

# RUSTC_TARGET_TRIPLES contains the list of target triples as they appear in the
# cros_sdk. If this list gets changed, ensure that each of these values has a
# corresponding compiler/rustc_target/src/spec file created below and a line
# referring to it in 0001-add-cros-targets.patch.

RUSTC_TARGET_TRIPLES=( "${CATEGORY#cross-}" )
case "${CATEGORY}" in
	cross-x86_64-cros-linux-gnu )
		RUSTC_BARE_TARGET_TRIPLES=(
			# These UEFI targets are used by and supported for
			# ChromeOS flex; contact chromeos-flex-eng@google.com
			# with any questions. Please add
			# chromeos-toolchain@google.com if you would like to use
			# any of these triples for your project.
			i686-unknown-uefi
			x86_64-unknown-uefi
		)
		# No need to change BDEPEND; x86 compiler RT is provided by
		# sys-devel/llvm.
		;;

	cross-armv7a-cros-linux-gnueabihf )
		# TODO(b/297387332): Is this the right split?
		RUSTC_BARE_TARGET_TRIPLES=(
			thumbv6m-none-eabi # Cortex-M0, M0+, M1
			thumbv7m-none-eabi # Cortex-M3
			thumbv7em-none-eabihf # Cortex-M4F, M7F, FPU, hardfloat
		)
		BDEPEND+=" ${CATEGORY}/compiler-rt "
		;;

	cross-aarch64-cros-linux-gnu )
		BDEPEND+=" ${CATEGORY}/compiler-rt "
		;;

	* )
		RUSTC_TARGET_TRIPLES=()
		;;
esac

# We need the cross-compiler sysroots to build stdlib.
for TRIPLE in "${RUSTC_TARGET_TRIPLES[@]}"; do
	BDEPEND+="
		cross-${TRIPLE}/glibc
		cross-${TRIPLE}/libcxx
		cross-${TRIPLE}/libxcrypt
		cross-${TRIPLE}/linux-headers
		cross-${TRIPLE}/llvm-libunwind
	"
done

IS_NOP_BUILD=false
[[ "${#RUSTC_TARGET_TRIPLES[@]}" -eq 0 ]] && IS_NOP_BUILD=true

# Triples to build with the panic=unwind (AKA default) build configuration.
# Without an explicit `--sysroot`, this is the libstd that cargo/rustc will
# select.
DEFAULT_STDLIB_TRIPLES=(
	"${RUSTC_TARGET_TRIPLES[@]}"
	"${RUSTC_BARE_TARGET_TRIPLES[@]}"
)

# Triples to build with the panic=abort build configuration. When statically
# linking libstd, the panic strategy of libstd doesn't matter. When dynamically
# linking, the panic strategy ends up getting built into libstd.so, so the
# 'default' stdlib.so (with panic="unwind" set) can't be dynamically linked with
# binaries with panic="abort" set. All CrOS target binaries are built with
# panic="abort", and they generally prefer to link dynamically with libstd, so
# include those here.
PANIC_ABORT_TRIPLES=(
	"${RUSTC_TARGET_TRIPLES[@]}"
	# Baremetal doesn't dynamically link, so it's not a good use of
	# resources to build them here.
)

src_unpack() {
	# TODO: Would be nice to not have this hack.
	if "${IS_NOP_BUILD}"; then
		if [[ "${CATEGORY}" != "dev-lang" ]]; then
			die "Unknown/unsupported Rust category: ${CATEGORY}"
		fi

		S="${T}"
		return
	fi

	cros-rustc_src_unpack
}

src_prepare() {
	if "${IS_NOP_BUILD}"; then
		# `src_prepare` requires this to be called before exiting. The
		# actual use of user patches with this ebuild is not supported.
		eapply_user
		return
	fi
	cros-rustc_src_prepare
}

src_configure() {
	"${IS_NOP_BUILD}" && return
	cros-rustc_src_configure
}

# Copies libraries for each `triple` from Rust's build directory into `to_dir`.
# These artifacts are meant to be installed by `_install_rust_triples`.
# Usage: _copy_rust_triples "${to_dir}" "${triples[@]}"
_copy_rust_triples() {
	local to_dir="$1"
	shift
	local triples=( "$@" )

	mkdir "${to_dir}" || die
	local child target triple
	local libdir="${CROS_RUSTC_BUILD_DIR:?}/${CBUILD}/stage1/lib64"
	# Some triples only install in `lib/` as of Rust 1.85.
	if [[ ! -d "${libdir}" ]]; then
		libdir="${libdir%64}"
		[[ -d "${libdir}" ]] || die "No libdir found"
	fi
	local obj="${libdir}/rustlib"
	for triple in "${triples[@]}"; do
		target="${to_dir}/${triple}"
		mkdir -p "${target}" || die
		for child in "${obj}/${triple}/"*; do
			# Skip the /bin directory, since that just contains LLVM
			# tools that rust-host has already installed.
			if [[ "${child}" != */bin ]]; then
				cp -a "${child}" "${target}" || die
			fi
		done
	done
}

src_compile() {
	"${IS_NOP_BUILD}" && return

	cros-rustc_src_compile library
	_copy_rust_triples "${T}/default-stdlib" "${DEFAULT_STDLIB_TRIPLES[@]}"

	local cargo_config="${S}/.cargo/config.toml"
	cat - >>"${cargo_config}" <<EOF || die

[profile.release]
panic = "abort"
EOF
	cros-rustc_src_compile library
	_copy_rust_triples "${T}/panic-eq-abort" "${PANIC_ABORT_TRIPLES[@]}"
}

# Installs the given `triples` in `dir_name`.
# Usage: _copy_rust_triples "${dir_name}" "${triples[@]}"
_install_rust_triples() {
	local dir_name="$1"
	shift
	local triples=( "$@" )

	local triple
	for triple in "${triples[@]}"; do
		doins -r "${dir_name}/${triple}"
	done
}

src_install() {
	"${IS_NOP_BUILD}" && return

	local lib="$(get_libdir)"
	insinto "/usr/${lib}/rustlib"
	_install_rust_triples "${T}/default-stdlib" "${DEFAULT_STDLIB_TRIPLES[@]}"

	insinto "/usr/${lib}/rust-sysroots/panic-abort/${lib}/rustlib"
	_install_rust_triples "${T}/panic-eq-abort" "${PANIC_ABORT_TRIPLES[@]}"
}
