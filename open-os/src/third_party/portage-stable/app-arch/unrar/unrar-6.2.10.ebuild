# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit multilib toolchain-funcs

MY_PN="${PN}src"

DESCRIPTION="Uncompress rar files"
HOMEPAGE="https://www.rarlab.com/rar_add.htm"
SRC_URI="https://www.rarlab.com/rar/${MY_PN}-${PV}.tar.gz -> ${P}.tar.gz"
S="${WORKDIR}/unrar"

LICENSE="unRAR"
SLOT="0/6" # subslot = soname version
KEYWORDS="*"

PATCHES=( "${FILESDIR}/${PN}-6.2.6-honor-flags.patch" )

src_prepare() {
	default

	local sed_args=( -e "/libunrar/s:.so:$(get_libname ${PV%.*.*}):" )
	if [[ ${CHOST} == *-darwin* ]] ; then
		sed_args+=( -e "s:-shared:-dynamiclib -install_name ${EPREFIX}/usr/$(get_libdir)/libunrar$(get_libname ${PV%.*.*}):" )
	else
		sed_args+=( -e "s:-shared:& -Wl,-soname -Wl,libunrar$(get_libname ${PV%.*.*}):" )
	fi
	sed -i "${sed_args[@]}" makefile || die
}

src_configure() {
	mkdir -p build-{lib,bin} || die
	printf 'VPATH = ..\ninclude ../makefile' > build-lib/Makefile || die
	cp build-{lib,bin}/Makefile || die
}

src_compile() {
	unrar_make() {
		emake AR="$(tc-getAR)" CXX="$(tc-getCXX)" CXXFLAGS="${CXXFLAGS}" STRIP=true "$@"
	}

	unrar_make CXXFLAGS+=" -fPIC" -C build-lib lib
	ln -s libunrar$(get_libname ${PV%.*.*}) build-lib/libunrar$(get_libname) || die
	ln -s libunrar$(get_libname ${PV%.*.*}) build-lib/libunrar$(get_libname ${PV}) || die

	unrar_make -C build-bin
}

src_install() {
	dobin build-bin/unrar
	dodoc readme.txt

	dolib.so build-lib/libunrar*

	insinto /usr/include/libunrar${PV%.*.*}
	doins *.hpp
	dosym libunrar${PV%.*.*} /usr/include/libunrar

	find "${ED}" -type f -name "*.a" -delete || die
}
