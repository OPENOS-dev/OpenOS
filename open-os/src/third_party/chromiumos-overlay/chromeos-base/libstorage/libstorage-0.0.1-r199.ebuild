# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "8f43dfd7edbac2a1aa65be54dfb9e8a923f456dd" "b8c09b0737d26e92e8c1543f785a92a112de09cc" "00e60203a732c85c12f77c0e13be1a50a6819c91" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "b321807c72a196bf119568150e9689a03838d860" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
# TODO(crbug.com/809389): Avoid directly including headers from other packages.
CROS_WORKON_SUBTREE="common-mk libcrossystem libhwsec-foundation libstorage metrics secure_erase_file .gn"

PLATFORM_NATIVE_TEST="yes"
PLATFORM_SUBDIR="libstorage"

inherit cros-workon platform

DESCRIPTION="Library to get Chromium OS specific storage access"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/libstorage"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="default_key_stateful device-mapper selinux test fuzzer"

COMMON_DEPEND="
	selinux? (
		sys-libs/libselinux:=
	)
	chromeos-base/libcrossystem:=
	chromeos-base/libhwsec-foundation:=
	chromeos-base/secure-erase-file:=
	>=chromeos-base/metrics-0.0.1-r3152:=
	dev-cpp/abseil-cpp:=
	dev-libs/openssl:=
	sys-apps/rootdev:=
	sys-apps/keyutils:=
	sys-fs/ecryptfs-utils:=
	sys-fs/e2fsprogs:=
"

RDEPEND="
	${COMMON_DEPEND}
"

DEPEND="
	${COMMON_DEPEND}
"
