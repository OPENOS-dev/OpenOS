# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "55903d6d672e8bdad40486765d522b08aa485668" "ec4b5444e6dc2c34dd46ced229cea8d9eae5f4ef" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
# TODO(allenvic): Remove libpasswordprovider from here once crbug.com/833675 is resolved.
CROS_WORKON_SUBTREE="common-mk libpasswordprovider smbprovider .gn"

PLATFORM_SUBDIR="smbprovider"

inherit cros-workon platform cros-protobuf user

DESCRIPTION="Provides access to Samba file share"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/smbprovider/"

LICENSE="BSD-Google"
SLOT="0/0"
KEYWORDS="*"
IUSE="fuzzer"

COMMON_DEPEND="
	>=net-fs/samba-4.5.3-r6
	sys-apps/dbus:=
"

RDEPEND="${COMMON_DEPEND}"

DEPEND="
	${COMMON_DEPEND}
	chromeos-base/protofiles:=
	chromeos-base/system_api:=[fuzzer?]
	chromeos-base/libpasswordprovider:=
"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
	chromeos-base/minijail
"

pkg_setup() {
	# Has to be done in pkg_setup() instead of pkg_preinst() since
	# src_install() needs smbproviderd:smbproviderd.
	enewuser "smbproviderd"
	enewgroup "smbproviderd"
	cros-workon_pkg_setup
}

src_install() {
	platform_src_install

	# fuzzer_component_id is unknown/unlisted
	platform_fuzzer_install "${S}"/OWNERS "${OUT}"/netbios_packet_fuzzer

	local daemon_store="/etc/daemon-store/smbproviderd"
	dodir "${daemon_store}"
	fperms 0700 "${daemon_store}"
	fowners smbproviderd:smbproviderd "${daemon_store}"
}
