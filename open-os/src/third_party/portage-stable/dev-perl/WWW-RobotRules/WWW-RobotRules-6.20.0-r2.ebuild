# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=GAAS
DIST_VERSION=6.02
inherit perl-module

DESCRIPTION="Parse /robots.txt file"

SLOT="0"
KEYWORDS="*"

RDEPEND="
	!<dev-perl/libwww-perl-6
	>=dev-perl/URI-1.10
"
BDEPEND="${RDEPEND}
"
