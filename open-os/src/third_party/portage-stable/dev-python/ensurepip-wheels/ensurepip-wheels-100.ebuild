# Copyright 2022-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Shared wheels for ensurepip Python module"
HOMEPAGE="https://wiki.gentoo.org/wiki/No_homepage"

LICENSE="metapackage"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	dev-python/ensurepip-pip
	dev-python/ensurepip-setuptools
"
