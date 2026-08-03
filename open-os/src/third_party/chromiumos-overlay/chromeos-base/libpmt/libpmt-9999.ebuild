# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
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
KEYWORDS="~*"
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
