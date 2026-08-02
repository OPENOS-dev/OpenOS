# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_GO_SOURCE=(
	"github.com/smartystreets/goconvey v${PV}"
)

CROS_GO_PACKAGES=(
	"github.com/smartystreets/goconvey/convey"
	"github.com/smartystreets/goconvey/convey/gotest"
	"github.com/smartystreets/goconvey/convey/reporting"
)

CROS_GO_TEST=(
	"${CROS_GO_PACKAGES[@]}"
)

inherit cros-go

DESCRIPTION="Go testing in the browser. Integrates with go test."
HOMEPAGE="https://github.com/smartystreets/goconvey"
SRC_URI="$(cros-go_src_uri)"

LICENSE="MIT"
KEYWORDS="*"
IUSE=""
SLOT="0"

DEPEND="
	dev-go/assertions
	dev-go/go-gls
	dev-go/go-tools
"
RDEPEND="
	${DEPEND}
	!<=dev-util/android-provision-0.0.1-r114
"
