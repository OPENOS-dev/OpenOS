# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit flag-o-matic toolchain-funcs

DESCRIPTION="Small and fast Portage helper tools written in C"
HOMEPAGE="https://wiki.gentoo.org/wiki/Portage-utils"

if [[ ${PV} == *9999 ]]; then
	inherit git-r3 autotools
	EGIT_REPO_URI="https://anongit.gentoo.org/git/proj/portage-utils.git"
	KEYWORDS="~*"
else
	SRC_URI="https://dev.gentoo.org/~grobian/distfiles/${P}.tar.xz"
	KEYWORDS="*"
fi

LICENSE="GPL-2"
SLOT="0"
IUSE="openmp +qmanifest +qtegrity static"

RDEPEND="
	openmp? ( || (
		sys-devel/gcc:*[openmp]
		sys-libs/libomp
	) )
	qmanifest? (
		!static? (
			app-crypt/gpgme:=
			app-crypt/libb2:=
			dev-libs/openssl:=
			sys-libs/zlib:=
		)
	)
	qtegrity? (
		!static? (
			dev-libs/openssl:=
		)
	)"
DEPEND="${RDEPEND}
	qmanifest? (
		static? (
			app-crypt/gpgme[static-libs]
			app-crypt/libb2[static-libs]
			dev-libs/openssl[static-libs]
			sys-libs/zlib[static-libs]
		)
	)
	qtegrity? (
		static? (
			dev-libs/openssl[static-libs]
		)
	)"
BDEPEND="virtual/pkgconfig"

PATCHES=(
	"${FILESDIR}"/portage-utils-0.96.1-qmerge-Send-ewarn-etc-to-stderr.patch
)

pkg_setup() {
	[[ ${MERGE_TYPE} != binary ]] && use openmp && tc-check-openmp
}

src_prepare() {
	default
	[[ ${PV} == *9999 ]] && eautoreconf
}

src_configure() {
	use static && append-ldflags -static

	econf \
		--disable-maintainer-mode \
		--with-eprefix="${EPREFIX}" \
		$(use_enable qmanifest) \
		$(use_enable qtegrity) \
		$(use_enable openmp)
}
