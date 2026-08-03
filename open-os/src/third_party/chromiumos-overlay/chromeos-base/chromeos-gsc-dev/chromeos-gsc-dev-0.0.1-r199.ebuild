# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.makefile file.

EAPI="7"

CROS_WORKON_COMMIT="f03a3057013b117dd1564b42715b8d71a21d086d"
CROS_WORKON_TREE="ce9e476ad48c03569388cd0b3c346eda27278530"
CROS_WORKON_PROJECT=("chromiumos/platform/ec")
CROS_WORKON_LOCALNAME=("platform/gsc-utils")
CROS_WORKON_DESTDIR=("${S}/platform/gsc-utils")
CROS_WORKON_EGIT_BRANCH=("gsc_utils")
CROS_RUST_SUBDIR="rust/explain_ap_ro_verification_status"
CROS_RUST_DISABLE_EDITION_CHECKS=1

inherit cros-rust cros-workon cros-toolchain-funcs cros-sanitizers cros-gcc

DESCRIPTION="Google Security Chip handling utilities"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/ec/+/refs/heads/gsc_utils"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="cros_host"

COMMON_DEPEND="
	cros_host? ( dev-rust/third-party-crates-src:= )
	dev-libs/openssl:0=
	virtual/libusb:1=
"

RDEPEND="
	cros_host? ( dev-util/bazel )
	!<chromeos-base/chromeos-cr50-dev-0.0.2
	${COMMON_DEPEND}
"

DEPEND="${COMMON_DEPEND}"

src_unpack() {
	cros-workon_src_unpack
	if use cros_host; then
		cros-rust_src_unpack
	fi
	S+="/platform/gsc-utils"
}

set_build_env() {
	cros_use_gcc

	tc-export CC BUILD_CC PKG_CONFIG
	export HOSTCC=${CC}
	export BUILDCC=${BUILD_CC}
}

src_compile() {
	set_build_env

	export BOARD=cr50

	emake -C extra/usb_updater clean
	emake -C extra/usb_updater gsctool

	if use cros_host; then
		cd "${CROS_RUST_SUBDIR}" ||
			die "failed to change directory to ${CROS_RUST_SUBDIR}"
		ecargo_build
	fi
}

src_configure() {
	cros_use_gcc
	sanitizers-setup-env
	if use cros_host; then
		cd "${CROS_RUST_SUBDIR}" ||
			die "failed to change directory to ${CROS_RUST_SUBDIR}"
		cros-rust_src_configure
	fi
	default
}

src_install() {
	dosbin "extra/usb_updater/gsctool"
	dosym "gsctool" "/usr/sbin/usb_updater"

	if use cros_host; then
		# Building with bazel for opentitantool doesn't work well in
		# portage at the moment. Just symlink a "binary" in /usr/bin/
		# to a wrapper script that exists in the chroot source
		# checkout. This script will build opentitantool with bazel in
		# the chroot and then forward the commands to the built
		# binary.
		dosym "/mnt/host/source/src/platform/gsc-utils/util/opentitantool.sh" \
			"/usr/bin/opentitantool"
		dobin "$(cros-rust_get_build_dir)/explain_ap_ro_verification_status"
		dobin "util/explain_aprov.sh"
	fi
}

src_test() {
	if ! use cros_host; then
		return  # Rust tests only when building for host.
	fi
	# Iterate through all crates in the Rust subdirectory.
	find rust -type f -name Cargo.toml | while read -r toml; do
		local dir="$(dirname "${toml}")"
		(
			cd "${dir}" ||
				die "failed to change directory to ${dir}"
			cros-rust_src_test
		)
	done
}
