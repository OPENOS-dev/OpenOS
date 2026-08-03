# Copyright 1999-2021 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit multilib-minimal

DESCRIPTION="Pthread functions stubs for platforms missing them"
HOMEPAGE="https://www.x.org/wiki/ https://gitlab.freedesktop.org/xorg/lib/pthread-stubs"
SRC_URI="https://xcb.freedesktop.org/dist/${P}.tar.bz2"

LICENSE="MIT"
SLOT="0"
KEYWORDS="*"
IUSE=""

multilib_src_configure() {
	ECONF_SOURCE="${S}" econf
}

# there is nothing to compile for this package, all its contents are produced by
# configure. the only make job that matters is make install
multilib_src_compile() { true; }
