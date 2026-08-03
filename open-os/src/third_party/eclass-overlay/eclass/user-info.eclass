# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

# @ECLASS: user-info.eclass
# @MAINTAINER:
# base-system@gentoo.org (Linux)
# Michał Górny <mgorny@gentoo.org> (NetBSD)
# @SUPPORTED_EAPIS: 6 7 8
# @BLURB: Read-only access to user and group information

case ${EAPI} in
	6|7|8) ;;
	*) die "${ECLASS}: EAPI ${EAPI:-0} not supported" ;;
esac

if [[ -z ${_USER_INFO_ECLASS} ]]; then
_USER_INFO_ECLASS=1

# TODO(b/187790077): Move user eclass APIs here that align w/Gentoo.
inherit user

# @FUNCTION: egetusername
# @USAGE: <uid>
# @DESCRIPTION:
# Gets the username for given UID.
egetusername() {
	[[ $# -eq 1 ]] || die "usage: egetusername <uid>"

	egetent passwd "$1" | cut -d: -f1
}

# @FUNCTION: egetgroupname
# @USAGE: <gid>
# @DESCRIPTION:
# Gets the group name for given GID.
egetgroupname() {
	[[ $# -eq 1 ]] || die "usage: egetgroupname <gid>"

	egetent group "$1" | cut -d: -f1
}

# @FUNCTION: egethome
# @USAGE: <user>
# @DESCRIPTION:
# Gets the home directory for the specified user.
egethome() {
	local pos

	[[ $# -eq 1 ]] || die "usage: egethome <user>"

	case ${CHOST} in
	*-freebsd*|*-dragonfly*)
		pos=9
		;;
	*)	# Linux, NetBSD, OpenBSD, etc...
		pos=6
		;;
	esac

	egetent passwd "$1" | cut -d: -f${pos}
}

# @FUNCTION: egetshell
# @USAGE: <user>
# @DESCRIPTION:
# Gets the shell for the specified user.
egetshell() {
	local pos

	[[ $# -eq 1 ]] || die "usage: egetshell <user>"

	case ${CHOST} in
	*-freebsd*|*-dragonfly*)
		pos=10
		;;
	*)	# Linux, NetBSD, OpenBSD, etc...
		pos=7
		;;
	esac

	egetent passwd "$1" | cut -d: -f${pos}
}

# @FUNCTION: egetcomment
# @USAGE: <user>
# @DESCRIPTION:
# Gets the comment field for the specified user.
egetcomment() {
	local pos

	[[ $# -eq 1 ]] || die "usage: egetcomment <user>"

	case ${CHOST} in
	*-freebsd*|*-dragonfly*)
		pos=8
		;;
	*)	# Linux, NetBSD, OpenBSD, etc...
		pos=5
		;;
	esac

	egetent passwd "$1" | cut -d: -f${pos}
}

# @FUNCTION: egetgroups
# @USAGE: <user>
# @DESCRIPTION:
# Gets all the groups user belongs to.  The primary group is returned
# first, then all supplementary groups.  Groups are ','-separated.
egetgroups() {
	# Nothing uses this, so not clear we care.
	die "Not implemented: please contact go/cros-build-help"
}

fi
