# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
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
KEYWORDS="~*"
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
