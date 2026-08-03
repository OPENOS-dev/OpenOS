# Copyright 2018 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_GO_SOURCE="github.com/catapult-project/catapult e8367e6d6e835e21eb5e4f058a910dbf090212c5"

CROS_GO_BINARIES=(
	"github.com/catapult-project/catapult/web_page_replay_go/src/wpr.go"
	"github.com/catapult-project/catapult/web_page_replay_go/src/httparchive.go"
)

CROS_GO_TEST=(
	"github.com/catapult-project/catapult/web_page_replay_go/src"
	"github.com/catapult-project/catapult/web_page_replay_go/src/webpagereplay"
)

inherit cros-go
SRC_URI="$(cros-go_src_uri)"

DESCRIPTION="Web Page Replay (for testing)"
HOMEPAGE="https://github.com/catapult-project/catapult/tree/HEAD/web_page_replay_go"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE=""

DEPEND="
	dev-go/blackfriday
	dev-go/cli
	dev-go/go-md2man
	dev-go/godebug
	dev-go/net
"
RDEPEND="${DEPEND}"

src_install() {
	cros-go_src_install
	local wprg_src="${S}/src/github.com/catapult-project/catapult/web_page_replay_go"
	insinto "/usr/share/wpr"
	doins "${wprg_src}/ecdsa_cert.pem"
	doins "${wprg_src}/ecdsa_key.pem"
	doins "${wprg_src}/wpr_cert.pem"
	doins "${wprg_src}/wpr_key.pem"
	doins "${wprg_src}/deterministic.js"
}
