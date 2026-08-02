# Copyright 1999-2021 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="wrapper for autoconf to manage multiple autoconf versions"
HOMEPAGE="https://wiki.gentoo.org/wiki/No_homepage"

LICENSE="GPL-2"
SLOT="0"
[[ "${PV}" == *_pre* ]] || \
KEYWORDS="*"

S="${WORKDIR}"

src_install() {
	exeinto /usr/$(get_libdir)/misc
	newexe "${FILESDIR}"/ac-wrapper-${PV}.sh ac-wrapper.sh

	dodir /usr/bin
	local x=
	for x in auto{conf,header,m4te,reconf,scan,update} ifnames ; do
		dosym ../$(get_libdir)/misc/ac-wrapper.sh /usr/bin/${x}
	done
}
