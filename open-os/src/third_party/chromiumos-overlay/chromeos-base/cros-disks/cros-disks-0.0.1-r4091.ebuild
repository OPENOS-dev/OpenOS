# Copyright 2014 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "14e9bac999bdb38b0da9d0fc2da455ba8fc7546b" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD="1"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
# TODO(crbug.com/809389): Avoid directly including headers from other packages.
CROS_WORKON_SUBTREE="common-mk cros-disks metrics .gn"

PLATFORM_SUBDIR="cros-disks"
# Tests use /dev/loop*.
PLATFORM_HOST_DEV_TEST="yes"

inherit cros-workon platform user

DESCRIPTION="Disk mounting daemon for Chromium OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/cros-disks/"
LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="chromeless_tty fuzzer +seccomp"

COMMON_DEPEND="
	>=chromeos-base/metrics-0.0.1-r3152:=
	chromeos-base/minijail:=
	chromeos-base/session_manager-client:=
	dev-cpp/abseil-cpp:=
	sys-apps/rootdev:=
	sys-apps/util-linux:=
"

RDEPEND="
	${COMMON_DEPEND}
	net-fs/sshfs
	sys-fs/dosfstools
	sys-fs/exfat-utils
	sys-fs/fuse-archive
	sys-fs/fuse-exfat
	sys-fs/mount-zip
	sys-fs/ntfs3g
	sys-fs/rar2fs
	virtual/udev
"

DEPEND="
	${COMMON_DEPEND}
	chromeos-base/system_api:=[fuzzer?]
"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
	chromeos-base/minijail
"

pkg_preinst() {
	enewuser "cros-disks"
	enewgroup "cros-disks"

	enewuser "ntfs-3g"
	enewgroup "ntfs-3g"

	enewuser "fuse-archivemount"
	enewgroup "fuse-archivemount"

	enewuser "fuse-exfat"
	enewgroup "fuse-exfat"

	enewuser "fuse-fusebox"
	enewgroup "fuse-fusebox"

	enewuser "fuse-rar2fs"
	enewgroup "fuse-rar2fs"

	enewuser "fuse-zip"
	enewgroup "fuse-zip"

	enewuser "fuse-sshfs"
	enewgroup "fuse-sshfs"

	enewuser "fuse-drivefs"
	enewgroup "fuse-drivefs"

	enewuser "mkfs"
	enewgroup "mkfs"
}

src_install() {
	platform_src_install
	local fuzzers=(
		filesystem_label_fuzzer
		mount_info_fuzzer
	)

	local fuzzer
	for fuzzer in "${fuzzers[@]}"; do
		# fuzzer_component_id is unknown/unlisted
		platform_fuzzer_install "${S}"/OWNERS "${OUT}/${PN}_${fuzzer}"
	done
}
