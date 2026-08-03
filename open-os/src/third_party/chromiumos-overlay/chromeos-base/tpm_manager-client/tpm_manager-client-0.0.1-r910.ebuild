# Copyright 2019 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "b8c09b0737d26e92e8c1543f785a92a112de09cc" "6c758e6ee408412dc2833681b642c2d01bc2dab1" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk libhwsec-foundation tpm_manager .gn"

PLATFORM_SUBDIR="tpm_manager/client"

inherit cros-workon platform cros-protobuf

DESCRIPTION="TPM Manager D-Bus client library for Chromium OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/tpm_manager/client/"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="tpm tpm2 fuzzer"
# Disable unittesting for client bindings.
RESTRICT="test"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
"

COMMON_DEPEND="
	chromeos-base/system_api:=[fuzzer?]
	dev-libs/openssl:0=
"

# Workaround to rebuild this package on the chromeos-dbus-bindings update.
# Please find the comment in chromeos-dbus-bindings for its background.
DEPEND="${COMMON_DEPEND}
	chromeos-base/chromeos-dbus-bindings:=
"

# Note that for RDEPEND, we conflict with tpm_manager package older than
# 0.0.1 because this client is incompatible with daemon older than version
# 0.0.1. We didn't RDEPEND on tpm_manager version 0.0.1 or greater because
# we don't want to create circular dependency in case the package tpm_manager
# depends on some package foo that also depend on this package.
RDEPEND="${COMMON_DEPEND}
	!<chromeos-base/tpm_manager-0.0.1-r2238
"

src_install() {
	platform_src_install

	# Install D-Bus client library.
	platform_install_dbus_client_lib "tpm_manager"

	dobin "${OUT}"/tpm_manager_client
}
