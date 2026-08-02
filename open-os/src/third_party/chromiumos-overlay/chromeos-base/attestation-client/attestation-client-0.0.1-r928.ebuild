# Copyright 2019 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "04936ed8ac043d33b6494b11f61c5efe12c71d03" "0fd688d96a8918e3c1f12cc58820e27e1eddcffa" "a329563ab6729d1062f1016c116293bfb8f91a63" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk attestation/client attestation/common/dbus_bindings attestation/pca_agent/dbus_bindings .gn"

PLATFORM_SUBDIR="attestation/client"
WANT_LIBCHROME="no"

inherit cros-workon platform

DESCRIPTION="Attestation D-Bus client library for Chromium OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/attestation/client/"

LICENSE="BSD-Google"
KEYWORDS="*"
# Disable unittesting for client bindings.
RESTRICT="test"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
"

# Workaround to rebuild this package on the chromeos-dbus-bindings update.
# Please find the comment in chromeos-dbus-bindings for its background.
DEPEND="
	chromeos-base/chromeos-dbus-bindings:=
	chromeos-base/system_api:=[fuzzer?]
"

# Note that for RDEPEND, we conflict with attestation package older than
# 0.0.1 because this client is incompatible with daemon older than version
# 0.0.1. We didn't RDEPEND on attestation version 0.0.1 or greater because
# we don't want to create circular dependency in case the package attestation
# depends on some package foo that also depend on this package.
RDEPEND="
	!<chromeos-base/attestation-0.0.1
"

src_install() {
	platform_src_install

	# Install D-Bus client library.
	platform_install_dbus_client_lib "attestation"
	platform_install_dbus_client_lib "pca_agent"
}
