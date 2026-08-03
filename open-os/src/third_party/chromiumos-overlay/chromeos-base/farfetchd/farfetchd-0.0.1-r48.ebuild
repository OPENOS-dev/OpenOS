EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "da0ca7e4dedeac2f8b7a4595c69a68522f171749" "00e60203a732c85c12f77c0e13be1a50a6819c91" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD="1"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk farfetchd libstorage .gn"

PLATFORM_SUBDIR="farfetchd"

inherit cros-workon platform cros-protobuf user

DESCRIPTION="Generalizing readahead-as-a-service for Chrome OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/farfetchd/"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

RDEPEND="
	sys-apps/rootdev:=
"
DEPEND="
	${RDEPEND}
	chromeos-base/libstorage
"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
"
src_install() {
	platform_src_install
	platform_install_dbus_client_lib
}
