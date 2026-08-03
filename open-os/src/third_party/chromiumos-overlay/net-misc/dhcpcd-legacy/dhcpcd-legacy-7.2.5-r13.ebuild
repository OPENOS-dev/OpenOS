# Copyright 1999-2015 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/net-misc/dhcpcd/dhcpcd-6.8.2.ebuild,v 1.1 2015/05/05 13:20:12 williamh Exp $

EAPI=7
CROS_WORKON_COMMIT="2694ea1dc30009a46a7ceb31e7c55be461edbbac"
CROS_WORKON_TREE="c1d460de925a1392461f81c5612a5913c1c5163a"
CROS_WORKON_PROJECT="chromiumos/third_party/dhcpcd"
CROS_WORKON_LOCALNAME="dhcpcd-legacy"
CROS_WORKON_EGIT_BRANCH="chromeos-7"

MY_P="${P/_alpha/-alpha}"
MY_P="${MY_P/_beta/-beta}"
MY_P="${MY_P/_rc/-rc}"
#SRC_URI not defined because we get our source locally
KEYWORDS="*"
S="${WORKDIR}/${MY_P}"

inherit cros-sanitizers cros-workon eutils systemd cros-toolchain-funcs user

DESCRIPTION="A fully featured, yet light weight RFC2131 compliant DHCP client"
HOMEPAGE="http://roy.marples.name/projects/dhcpcd/"
LICENSE="BSD-2"
SLOT="0"
IUSE="elibc_glibc +embedded kernel_linux +udev +dbus"

COMMON_DEPEND="udev? ( sys-fs/udev )
		dbus? ( sys-apps/dbus )
"
DEPEND="${COMMON_DEPEND}"
RDEPEND="${COMMON_DEPEND}"

src_prepare()
{
	default
}

src_configure()
{
	sanitizers-setup-env

	local dev hooks
	use udev || dev="--without-dev --without-udev"
	if ! use dbus ; then
		hooks="--with-hook=ntp.conf"
		use elibc_glibc && hooks="${hooks} --with-hook=yp.conf"
	fi
	econf \
		--prefix= \
		--libexecdir=/lib/dhcpcd7 \
		--dbdir=/var/lib/dhcpcd7 \
		--rundir=/run/dhcpcd7 \
		"$(use_enable embedded)" \
		"$(use_enable dbus)" \
		"${dev}" \
		--disable-inet6 \
		CC="$(tc-getCC)" \
		"${hooks}"
	# Update DUID file path so it is writable by dhcp user.
	echo '#define DUID DBDIR "/" PACKAGE ".duid"' >> "${S}/config.h"
}

src_install()
{
	default
}

pkg_preinst()
{
	enewuser "dhcp"
	enewgroup "dhcp"
}

pkg_postinst()
{
	# Upgrade the duid file to the new format if needed
	local old_duid="${ROOT}"/var/lib/dhcpcd/dhcpcd.duid
	local new_duid="${ROOT}"/etc/dhcpcd.duid
	if [ -e "${old_duid}" ] && ! grep -q '..:..:..:..:..:..' "${old_duid}"; then
		sed -i -e 's/\(..\)/\1:/g; s/:$//g' "${old_duid}"
	fi

	# Move the duid to /etc, a more sensible location
	if [[ -e "${old_duid}" && ! -e "${new_duid}" ]]; then
		cp -p "${old_duid}" "${new_duid}"
	fi

	if [ -z "${REPLACING_VERSIONS}" ]; then
		elog
		elog "dhcpcd has zeroconf support active by default."
		elog "This means it will always obtain an IP address even if no"
		elog "DHCP server can be contacted, which will break any existing"
		elog "failover support you may have configured in your net configuration."
		elog "This behaviour can be controlled with the noipv4ll configuration"
		elog "file option or the -L command line switch."
		elog "See the dhcpcd and dhcpcd.conf man pages for more details."

		elog
		elog "Dhcpcd has duid enabled by default, and this may cause issues"
		elog "with some dhcp servers. For more information, see"
		elog "https://bugs.gentoo.org/show_bug.cgi?id=477356"
	fi

	if ! has_version net-dns/bind-tools; then
		elog
		elog "If you activate the lookup-hostname hook to look up your hostname"
		elog "using the dns, you need to install net-dns/bind-tools."
	fi
}
