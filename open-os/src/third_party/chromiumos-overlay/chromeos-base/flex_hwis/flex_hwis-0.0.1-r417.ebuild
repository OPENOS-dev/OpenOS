# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "06d960d2f96e426f18f4db7a4bb5c9245e502a2e" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169")
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_DESTDIR="${S}/platform2"
CROS_WORKON_SUBTREE="common-mk flex_hwis .gn metrics"

PLATFORM_SUBDIR="flex_hwis"

inherit cros-workon platform cros-protobuf

DESCRIPTION="Utility to collect/send Hardware Information for ChromeOS Flex"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/flex_hwis"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="flex_internal"

COMMON_DEPEND="
	chromeos-base/diagnostics:=
	chromeos-base/libbrillo:=
	chromeos-base/metrics:=
	chromeos-base/mojo_service_manager:=
	sys-apps/rootdev:=
"

RDEPEND="
	${COMMON_DEPEND}
	acct-group/flex_hwis
	acct-user/flex_hwis
"

DEPEND="
	${COMMON_DEPEND}
	flex_internal? ( chromeos-base/flex-hwis-private:= )
"
