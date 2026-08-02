# Copyright 1999-2021 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

DESCRIPTION="Virtual for command-line pagers"
SLOT="0"
KEYWORDS="*"

RDEPEND="|| ( sys-apps/less
	sys-apps/most
	sys-apps/util-linux[ncurses]
	app-text/lv
	app-editors/vim[vim-pager] )"
