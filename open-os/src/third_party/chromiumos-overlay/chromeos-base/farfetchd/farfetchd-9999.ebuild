EAPI=7

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
KEYWORDS="~*"
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
