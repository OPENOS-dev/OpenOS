# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT=("f1619213577b072b2d86004d7d37763c8af5dcf0" "906136da2c93e6dfe3c93763c93dfae59d24e54d")
CROS_WORKON_TREE=("5a4d2b2ed3372fda717d487fa924aa22fd749776" "71e8653b25816e9271e8b6751ddb0ff9ec131555")
CROS_WORKON_PROJECT=("chromiumos/platform/crdyboot" "chromiumos/platform/vboot_reference")
CROS_WORKON_LOCALNAME=("../platform/crdyboot" "../platform/vboot_reference")
CROS_WORKON_DESTDIR=("${S}" "${S}/third_party/vboot_reference")
SRC_URI="
	crdyboot_presigned? ( gs://chromeos-localmirror/distfiles/crdyboot-signed-1.0.6-4bfc3d57d4c2.tar.gz )
	test? ( gs://chromeos-localmirror/distfiles/crdyboot_test_data_b77077f4.tar.xz )"

# We're not building for CrOS userspace.
# This is used by cros-rust.eclass, so disable 'unused' complaints
# shellcheck disable=SC2034
CROS_RUST_FORCE_STATIC_LINK=1

inherit cros-workon cros-rust

DESCRIPTION="UEFI bootloaders for ChromeOS Flex"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE="crdyboot_presigned crdyshim_dev_key"

DEPEND="
	dev-libs/nss:=
	dev-rust/third-party-crates-src:=
"
RDEPEND="${DEPEND}"

# Clear the subdir so that we stay in the root of the repo.
CROS_RUST_SUBDIR=""

UEFI_TARGET_I686="i686-unknown-uefi"
UEFI_TARGET_X64_64="x86_64-unknown-uefi"
UEFI_TARGETS=(
	"${UEFI_TARGET_I686}"
	"${UEFI_TARGET_X64_64}"
)

src_prepare() {
	# Drop some packages that are not needed.
	sed -i 's:"tools/enroller",::' "${S}/Cargo.toml" || die
	sed -i 's:"tools/uefi_test_tool",::' "${S}/Cargo.toml" || die
	sed -i 's:"xtask",::' "${S}/Cargo.toml" || die
	sed -i 's:"avb",::' "${S}/Cargo.toml" || die
	sed -i 's:"bootimg",::' "${S}/Cargo.toml" || die
	# ecargo_test does not skip the optional dependencies
	# for the android feature. Remove these optional unused
	# features from the Cargo file.
	sed -i -e '/^android = \[/s/\[.*\]/\[\]/' \
		-e '/^avb =/d' \
		-e '/^bootimg =/d' "${S}/crdyboot/Cargo.toml" || die

	# Move the test data into the workspace subdirectory, where
	# tests expect to find it.
	if use test; then
		mv "${WORKDIR}/crdyboot_test_data" "${S}/workspace/" || die
	fi

	default
}

src_configure() {
	# Remove flags that come from BOARD_RUSTFLAGS and are
	# incompatible with the uefi targets.
	unset CROS_BASE_RUSTFLAGS

	cros-rust_src_configure

	# ECARGO_HOME is defined in cros-rust.eclass
	# shellcheck disable=SC2154
	local ecargo_config="${ECARGO_HOME}/config.toml"

	# Enable the c_variadic feature, used for logging from vboot.
	sed -i 's/-Zallow-features=/&c_variadic,/' "${ecargo_config}" || die

	# Set the appropriate linker for UEFI targets.
	cat <<- EOF >> "${ecargo_config}"
	[target.i686-unknown-uefi]
	linker = "lld-link"

	[target.x86_64-unknown-uefi]
	linker = "lld-link"
	EOF
}

src_compile() {
	# Skip this phase when code coverage is enabled. The UEFI
	# targets are no-std, so they are not trivially compatible with
	# `-Cinstrument-coverage`. Since the actual code coverage is
	# provided by the test phase (which uses a non-UEFI target),
	# skipping compilation here does not affect code coverage.
	if use rust-coverage; then
		return 0
	fi

	# We need to pass in a `--target` to the C compiler, but the
	# compiler-wrapper for the board's CC appends the board target
	# at the end, overriding that setting. Use
	# `x86_64-pc-linux-gnu-clang` instead, as the host wrapper
	# happens to not suffix a target. See
	# `compiler_wrapper/clang_flags.go` at the end of
	# `processClangFlags`.
	export CC="x86_64-pc-linux-gnu-clang"

	local cargo_args=(
		build
		--offline
		--release
		--package crdyboot
		--package crdyshim
	)

	# Default to embedding the release public key in crdyshim. If
	# `USE=crdyshim_dev_key` is set, use the crdyshim dev key from
	# vboot_reference instead.
	if use crdyshim_dev_key; then
		cargo_args+=( --features use_dev_pubkey )
	fi

	for uefi_target in "${UEFI_TARGETS[@]}"; do
		ecargo "${cargo_args[@]}" --target="${uefi_target}"
	done
}

# Get the standard suffix for a UEFI boot executable.
uefi_arch_suffix() {
	local uefi_target="$1"
	if [[ "${uefi_target}" == "${UEFI_TARGET_I686}" ]]; then
		echo "ia32.efi"
	elif [[ "${uefi_target}" == "${UEFI_TARGET_X64_64}" ]]; then
		echo "x64.efi"
	else
		die "unknown arch: ${uefi_target}"
	fi
}

src_install() {
	# Skip this phase when code coverage is enabled. Since the
	# compile phase was skipped, there's nothing to install.
	if use rust-coverage; then
		return 0
	fi

	for uefi_target in "${UEFI_TARGETS[@]}"; do
		# CARGO_TARGET_DIR is defined in an eclass
		# shellcheck disable=SC2154
		local release_dir="${CARGO_TARGET_DIR}/${uefi_target}/release"

		# Install crdyboot to the boot dir on the rootfs.
		insinto /boot/efi/boot
		newins "${release_dir}/crdyboot.efi" \
			"crdyboot$(uefi_arch_suffix "${uefi_target}")"

		# Install the unsigned crdyshim executables to a
		# location outside the boot dir. These executables are
		# built here so they can be extracted from the image and
		# signed, but are otherwise unused.
		insinto /usr/share/crdyshim
		newins "${release_dir}/crdyshim.efi" \
			"crdyshim$(uefi_arch_suffix "${uefi_target}")"
	done

	# Optionally install the prebuilt crdyboot executables and
	# signatures to the boot dir on the rootfs.
	#
	# In an unsigned build, nothing will use these files. In a
	# signed build, the unsigned crdyboot files will be overwritten
	# with the presigned files.
	if use crdyboot_presigned; then
		insinto /boot/efi/boot/presigned
		doins "${WORKDIR}"/crdyboot_presigned/*
	fi
}
