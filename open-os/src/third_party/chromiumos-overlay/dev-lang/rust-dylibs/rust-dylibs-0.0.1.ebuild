# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2
#
# This package exists to install Rust DSOs and metadata on boards.
# We rely on cross-*/rust to actually generate the DSOs, so no actual
# building/etc of code is performed. Linking against DSOs is only
# supported for userspaces on CrOS devices at the moment.

EAPI=7

IUSE="cros_host"

REQUIRED_USE="
	!cros_host
"

# This isn't meant to be installed on the host, so no BDEPENDs needed there.
BDEPEND="
	!cros_host? (
		arm? ( cross-armv7a-cros-linux-gnueabihf/rust:= )
		arm64? ( cross-aarch64-cros-linux-gnu/rust:= )
		amd64? ( cross-x86_64-cros-linux-gnu/rust:= )
	)
"

# Match cros-rust.eclass' dependencies for invalidation. Don't inherit cros-rust
# directly, since essentially all of its defaults and logic aren't desirable
# here, and inheriting it can easily introduce a circular dependency if
# mishandled.
DEPEND="
	virtual/rust:1=
	virtual/rust-binaries:=
"
RDEPEND="
	virtual/rust:1=
	virtual/rust-binaries:=
"

KEYWORDS="*"
LICENSE="|| ( UoI-NCSA MIT )"

SLOT="0/${PVR}"
S="${T}"

dso_dir() {
	local dso_dir="/usr/lib/rust-sysroots/panic-abort/lib64/rustlib/${CHOST}/lib"
	[[ -e "${dso_dir}" ]] || die "${dso_dir} does not exist - are you building for a regular CrOS target?"
	echo "${dso_dir}"
}

libstd_name() {
	local libstds
	local dso_dir="$(dso_dir)"
	read -r -a libstds < <(shopt -s nullglob; echo "${dso_dir}"/libstd-*.so) || die "no libstd found"
	[[ "${#libstds[@]}" -ne 1 ]] && die "Expected exactly one file matching ${dso_dir}/libstd*.so; got ${libstds[*]}"
	echo "${libstds[0]}"
}

src_unpack() {
	:
}

src_prepare() {
	# `src_prepare` requires this to be called before exiting. The
	# actual use of user patches with this ebuild is not supported.
	eapply_user
}

src_compile() {
	# When including rust static libraries, it is still necessary to link the
	# standard library so provide it via a pkg-config file so it can be
	# listed in `Requires:` entries. It may be necessary to add -Wl,--exclude-libs
	# to the static library you're linking to make sure the symbols aren't
	# used from the static library anyway.
	local std_lib_name=$(basename "$(libstd_name)" .so)
	std_lib_name="${std_lib_name#???}"
	sed -e "s/@STD_LIB@/${std_lib_name}/" -e "s/@PVR@/${PVR}/" \
		"${FILESDIR}/rust-dylib.pc.in" > rust-dylib.pc || die
}

src_install() {
	einfo "Installing libstd from $(dso_dir)"

	dolib.so "$(libstd_name)"

	insinto "/usr/$(get_libdir)/pkgconfig"
	doins rust-dylib.pc
}
