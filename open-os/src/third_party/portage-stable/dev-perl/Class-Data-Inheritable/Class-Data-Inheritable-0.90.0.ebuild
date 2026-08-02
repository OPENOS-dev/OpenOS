# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=RSHERER
DIST_VERSION=0.09
inherit perl-module

DESCRIPTION="Inheritable, overridable class data"
# License note: Artistic only for one file
# https://rt.cpan.org/Public/Bug/Display.html?id=132835
SLOT="0"
KEYWORDS="*"

PERL_RM_FILES=(
	t/pod.t
	t/pod-coverage.t
)
