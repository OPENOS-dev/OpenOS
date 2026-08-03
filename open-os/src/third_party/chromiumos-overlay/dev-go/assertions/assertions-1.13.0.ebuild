# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_GO_SOURCE=(
	"github.com/smartystreets/assertions v${PV}"
)

CROS_GO_PACKAGES=(
	"github.com/smartystreets/assertions"
	"github.com/smartystreets/assertions/internal/go-diff/diffmatchpatch"
	"github.com/smartystreets/assertions/internal/go-render/render"
	"github.com/smartystreets/assertions/internal/oglematchers"
)

CROS_GO_TEST=(
	"${CROS_GO_PACKAGES[@]}"
)

inherit cros-go

DESCRIPTION="Fluent assertion-style functions used by goconvey and gunit."
HOMEPAGE="https://github.com/smarty/assertions"
SRC_URI="$(cros-go_src_uri)"

LICENSE="MIT"
KEYWORDS="*"
IUSE=""
SLOT="0"

DEPEND=""
RDEPEND="
	${DEPEND}
	!<=dev-util/android-provision-0.0.1-r114
"
