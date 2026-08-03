# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="d57fba8f85abc0d29979909ae7685f2f262518dd"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "d5905ec1398baf43249e878c6be265550d8e6c2c" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk libsegmentation .gn"

PLATFORM_SUBDIR="libsegmentation/tools"

CROS_PROTOBUF_APPLY_DEFAULT_DEPS=0
inherit cros-workon platform cros-protobuf

DESCRIPTION="Test for Library to get ChromiumOS system properties"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/libsegmentation"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="feature_management"
# This package has no unittests.
RESTRICT="test"

RDEPEND="
	chromeos-base/libsegmentation:=
"

DEPEND="${RDEPEND}
	chromeos-base/feature-management-data:=
"

BDEPEND="
	${CROS_PROTOC_DEPS}
"
