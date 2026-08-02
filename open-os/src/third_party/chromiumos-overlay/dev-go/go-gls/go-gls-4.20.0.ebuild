# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_GO_SOURCE=(
	"github.com/jtolio/gls:github.com/jtolds/gls v${PV}"
)

CROS_GO_PACKAGES=(
	"github.com/jtolds/gls"
)

CROS_GO_TEST=(
	"${CROS_GO_PACKAGES[@]}"
)

inherit cros-go

DESCRIPTION="Goroutine local storage implementation"
HOMEPAGE="https://github.com/jtolio/gls"
SRC_URI="$(cros-go_src_uri)"

LICENSE="MIT"
KEYWORDS="*"
IUSE=""
SLOT="0"

DEPEND="
	dev-go/sync
"
RDEPEND="
	${DEPEND}
	!<=dev-util/android-provision-0.0.1-r114
"
