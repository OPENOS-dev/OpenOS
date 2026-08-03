# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_DESTDIR="${S}/platform2"

inherit cros-workon

DESCRIPTION="u-root based ramfs for kdump"

# u-root + lvm2 licenses.
LICENSE="GPL-2 BSD-2 LGPL-2.1 BSD"
SLOT="0"
KEYWORDS="~*"

BDEPEND="
	app-arch/cpio
	dev-go/u-root
"
DEPEND="
	sys-apps/makedumpfile
	sys-apps/util-linux
	sys-fs/e2fsprogs
	sys-fs/lvm2
"

UROOTGOPATH="/usr/share/u-root"

src_prepare() {
	default

	# Define a temporary GOPATH for local packages.
	local temp_gopath="${T}/gopath"

	# Make the local kdump_dev_init package available under the
	# "kdump-ramfs/kdump_dev_init" import path for the Go builder.
	mkdir -p "${temp_gopath}/src/kdump-ramfs"
	ln -s "${S}/platform2/kdump/ramfs/kdump_dev_init" \
		"${temp_gopath}/src/kdump-ramfs/kdump_dev_init"

	# Set up the environment for the Go build.
	export GOARCH="${ARCH}"
	export GO111MODULE="off"
	export GOPATH="$(go env GOPATH):${temp_gopath}:${UROOTGOPATH}"

	# Need the lddtree from the chromite dir.
	export PATH="${CHROMITE_BIN_DIR}:${PATH}"
}

src_compile() {
	# Build the initramfs with u-root. Note that since u-root is not aware of
	# sysroot, it cannot handle the dependencies of the non-Go binaries we need
	# (e.g., lvm). Thus here we use u-root to build a CPIO with only the go
	# packages, and then build another CPIO with the non-Go dependencies and
	# concat them together as the final result.

	# Step 1: Create the base u-root archive with only Go commands. This
	# ensures we get the correct, default filesystem layout from u-root itself,
	# including essential device nodes like /dev/console.
	local uroot_cmds=(
		"github.com/u-root/u-root/cmds/core/init"
		"github.com/u-root/u-root/cmds/core/mknod"
		"github.com/u-root/u-root/cmds/core/mount"
		"github.com/u-root/u-root/cmds/core/sync"
		"github.com/u-root/u-root/cmds/core/umount"
	)
	local uroot_base_archive="${T}/uroot_base.cpio"
	u-root -o "${uroot_base_archive}" \
		-defaultsh "kdump_dev_init" \
		"kdump-ramfs/kdump_dev_init" \
		"${uroot_cmds[@]}" || die

	# Step 2: Create a directory populated with non-Go dependencies using
	# lddtree's --copy-to-tree feature.
	local deps_staging_root="${T}/deps_staging_root"
	mkdir -p "${deps_staging_root}"

	local bins_to_include=(
		"/sbin/blkid"
		"/sbin/lvm"
		"/sbin/mke2fs"
		"/usr/sbin/makedumpfile"
	)
	lddtree --root="${SYSROOT}" --copy-to-tree="${deps_staging_root}" \
		"${bins_to_include[@]}" || die

	# Create a CPIO archive from that dependency staging directory.
	local deps_archive="${T}/deps.cpio"
	(
		cd "${deps_staging_root}" || die
		find . | cpio -o -H newc > "${deps_archive}"
	) || die

	# Step 3: Combine the two archives by concatenating them. The kernel can
	# read concatenated CPIO archives as a single ramfs.
	cat "${uroot_base_archive}" "${deps_archive}" > "${S}/kdump-rfs.cpio" \
		|| die
}

src_install() {
	insinto /usr/share/kdump/boot
	doins kdump-rfs.cpio
}
