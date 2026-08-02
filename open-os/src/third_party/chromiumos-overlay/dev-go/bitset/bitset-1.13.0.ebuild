# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2.

EAPI=7

CROS_GO_SOURCE="github.com/bits-and-blooms/bitset v${PV}"

CROS_GO_PACKAGES=(
	"github.com/bits-and-blooms/bitset"
)

CROS_GO_TEST=(
	"${CROS_GO_PACKAGES[@]}"
)

inherit cros-go

DESCRIPTION="Go package implementing bitsets "
HOMEPAGE="https://github.com/bits-and-blooms/bitset"
SRC_URI="$(cros-go_src_uri)"

LICENSE="BSD"
SLOT="0"
KEYWORDS="*"
IUSE=""
