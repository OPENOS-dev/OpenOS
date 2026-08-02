# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit autotools flag-o-matic

DESCRIPTION="Serial Communication Program"
HOMEPAGE="https://salsa.debian.org/minicom-team/minicom"
SRC_URI="
	https://salsa.debian.org/${PN}-team/${PN}/-/archive/${PV}/${P}.tar.gz
	https://dev.gentoo.org/~ceamac/${CATEGORY}/${PN}/${PN}-m4-${PV}.tar.bz2
"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="*"
IUSE="nls"

DEPEND="sys-libs/ncurses:="

RDEPEND="
	${DEPEND}
	net-dialup/lrzsz
"

BDEPEND="
	virtual/pkgconfig
	nls? ( sys-devel/gettext )
"

PATCHES=(
	"${FILESDIR}"/${PN}-2.8-gentoo-runscript.patch
	"${FILESDIR}"/${PN}-2.8-lockdir.patch
	"${FILESDIR}"/${PN}-2.8-enable-large-file.patch
)

src_prepare() {
	default

	# 912459
	# Embed the needed m4 macros if gettext is not installed
	mv "${WORKDIR}"/m4 . || die

	eautoreconf
}

src_configure() {
	# ChromeOS: fix LFS support (https://bugs.gentoo.org/912680)
	append-lfs-flags

	# Lockdir must exist if not manually specified.
	# '/var/lock' is created by OpenRC.
	local myeconfargs=(
		# See bug #788142
		--sysconfdir="${EPREFIX}"/etc/${PN}

		--disable-rpath
		--enable-lock-dir="/var/lock"
		$(use_enable nls)
	)

	econf "${myeconfargs[@]}"
}

src_install() {
	default

	# Needs to match --sysconfdir above
	insinto /etc/minicom
	doins "${FILESDIR}"/minirc.dfl
}
