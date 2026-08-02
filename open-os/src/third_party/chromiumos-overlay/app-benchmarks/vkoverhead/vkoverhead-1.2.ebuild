# Copyright 2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit meson

DESCRIPTION="Microbenchmark for evaluating CPU overhead of Vulkan drivers"
HOMEPAGE="https://github.com/zmike/vkoverhead"
SRC_URI="https://github.com/zmike/${PN}/archive/refs/tags/v${PV}.tar.gz -> ${P}.tar.gz"

LICENSE="MIT"
SLOT="0"
KEYWORDS="*"

src_install() {
	into /usr/local/${PN}
	dobin "${BUILD_DIR}"/vkoverhead
}
