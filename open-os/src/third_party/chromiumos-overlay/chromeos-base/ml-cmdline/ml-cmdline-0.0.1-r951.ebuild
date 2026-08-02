# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="30a5abfd06297e0a86d2e4c4da08d98058df54d5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "bba4bef6c0743c6bedd60561a468afd36933b086" "6254ee88618b3a25710d425883a96ce5da6e351d" "ee9c0d9e5d2a98ba4313fae503ce3a73352e6580" "5697d5958a4e8c378cc194a89a2f34759bc417ec" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_DESTDIR="${S}/platform2"
CROS_WORKON_SUBTREE="common-mk libtouchraw ml ml_benchmark ml_core .gn"

PLATFORM_SUBDIR="ml/cmdline"

inherit cros-workon platform cros-protobuf

DESCRIPTION="Command line interface to machine learning service for Chromium OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/main/ml"

LICENSE="BSD-Google"
KEYWORDS="*"
SLOT="0/0"
IUSE="internal"

RDEPEND="
	chromeos-base/chrome-icu:=
	>=chromeos-base/metrics-0.0.1-r3152:=
	chromeos-base/minijail:=
	chromeos-base/ml:=
	chromeos-base/system_api:=
	dev-libs/ml-core:=
	sci-libs/tensorflow:=
	sys-libs/zlib:=
"

DEPEND="
	${RDEPEND}
	dev-libs/libutf:=
	dev-libs/marisa-aosp:=
"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
"
