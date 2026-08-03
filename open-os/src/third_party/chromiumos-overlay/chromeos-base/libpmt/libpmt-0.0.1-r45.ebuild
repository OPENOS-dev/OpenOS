# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "b9456d2f3facf22d130c0d8aa5110c4962620af7" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk libpmt .gn"

PLATFORM_NATIVE_TEST="yes"
PLATFORM_SUBDIR="libpmt"

WANT_LIBBRILLO="yes"
WANT_LIBCHROME="yes"

inherit cros-workon platform cros-protobuf

DESCRIPTION="Library to sample and decode Intel PMT telemetry data"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/libpmt"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="selinux test"

COMMON_DEPEND="
	dev-libs/libxml2:=
	dev-libs/protobuf:=
	dev-libs/re2:=
"

RDEPEND="
	${COMMON_DEPEND}
"

DEPEND="
	${COMMON_DEPEND}
"
