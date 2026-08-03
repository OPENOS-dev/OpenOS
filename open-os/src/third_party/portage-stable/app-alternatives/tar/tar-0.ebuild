# Copyright 2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

ALTERNATIVES=(
	"gnu:>=app-arch/tar-1.34-r2"
	libarchive:app-arch/libarchive
)

inherit app-alternatives

DESCRIPTION="Tar symlink"
KEYWORDS="*"
IUSE="split-usr"

RDEPEND="
	!<app-arch/tar-1.34-r2
"

src_install() {
	local usr_prefix=
	use split-usr && usr_prefix=../usr/bin/

	case $(get_alternative) in
		gnu)
			dosym gtar /bin/tar
			newman - tar.1 <<<".so gtar.1"
			;;
		libarchive)
			dosym "${usr_prefix}bsdtar" /bin/tar
			newman - tar.1 <<<".so bsdtar.1"
			;;
	esac
}
