# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2.

EAPI=7

# The necessary changes havn't been tagged as a release yet.
CROS_GO_SOURCE="github.com/electricbubble/gadb 97c5a1a929a9512dd613a88f4e16afa5fcfa58a2"

CROS_GO_PACKAGES=(
	"github.com/electricbubble/gadb"
)

inherit cros-go

DESCRIPTION="ADB Client in pure Golang."
HOMEPAGE="https://github.com/electricbubble/gadb"
SRC_URI="$(cros-go_src_uri)"

LICENSE="BSD"
SLOT="0"
KEYWORDS="*"
IUSE=""
RESTRICT="binchecks strip"

DEPEND="
"
RDEPEND="${DEPEND}"
