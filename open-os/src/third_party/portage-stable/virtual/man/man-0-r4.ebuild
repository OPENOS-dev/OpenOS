# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Virtual for man"

LICENSE="metapackage"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	|| (
		sys-apps/man-db
		>=app-text/mandoc-1.14.5-r1[system-man]
	)
"
