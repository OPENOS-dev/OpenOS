# Copyright 2018 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2.

EAPI=7

CROS_GO_SOURCE="github.com/xrash/smetrics 039620a656736e6ad994090895784a7af15e0b80"

CROS_GO_PACKAGES=(
	"github.com/xrash/smetrics"
)

CROS_GO_TEST=(
	"${CROS_GO_PACKAGES[@]}"
)

inherit cros-go

DESCRIPTION="String metrics library written in Go"
HOMEPAGE="https://github.com/xrash/smetrics"
SRC_URI="$(cros-go_src_uri)"

LICENSE="MIT"
SLOT="0"
KEYWORDS="*"
IUSE=""

DEPEND=""
RDEPEND="${DEPEND}"
