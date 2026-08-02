# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit meson

DESCRIPTION="Wayland protocol files"
HOMEPAGE="https://wayland.freedesktop.org/"

if [[ ${PV} = 9999* ]]; then
	EGIT_REPO_URI="https://gitlab.freedesktop.org/wayland/${PN}.git/"
	inherit git-r3
else
	SRC_URI="https://gitlab.freedesktop.org/wayland/${PN}/-/releases/${PV}/downloads/${P}.tar.xz"
	KEYWORDS="*"
fi

LICENSE="MIT"
SLOT="0"
IUSE="test"
RESTRICT="!test? ( test )"

DEPEND="
	test? ( dev-libs/wayland )
"
BDEPEND="
	dev-util/wayland-scanner
	virtual/pkgconfig
"

src_configure() {
	local emesonargs=(
		$(meson_use test tests)
	)
	meson_src_configure
}
