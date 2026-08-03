# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "8690cf34530625a393e76d599b01742a250bdb7b" "b8c09b0737d26e92e8c1543f785a92a112de09cc" "6a9a79d4054ab44a049d1756dc7e3cce63bc2cf8" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk libarc-attestation libhwsec-foundation libhwsec metrics .gn"

PLATFORM_SUBDIR="libarc-attestation"

inherit cros-workon platform cros-protobuf

DESCRIPTION="Utility for ARC Keymintd to perform Android Attestation and Remote Key Provisioning"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/libarc-attestation/"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="test"

RDEPEND="
	chromeos-base/libhwsec:=[test?]
	>=chromeos-base/metrics-0.0.1-r3152:=
	chromeos-base/system_api:=
"

DEPEND="
	${RDEPEND}
	"
