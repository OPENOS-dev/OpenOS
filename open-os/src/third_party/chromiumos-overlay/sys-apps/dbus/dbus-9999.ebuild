# Copyright 1999-2021 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

PYTHON_COMPAT=( python3_11 )
VIRTUALX_REQUIRED=manual

CROS_WORKON_PROJECT="chromiumos/third_party/dbus"
CROS_WORKON_EGIT_BRANCH="chromeos-1.16.2"

inherit autotools cros-sanitizers eutils flag-o-matic linux-info meson-multilib python-any-r1 readme.gentoo-r1 systemd virtualx user multilib-minimal cros-workon

DESCRIPTION="A message bus system, a simple way for applications to talk to each other"
HOMEPAGE="https://dbus.freedesktop.org/"

LICENSE="|| ( AFL-2.1 GPL-2 )"
SLOT="0"
KEYWORDS="~*"
IUSE="debug default-dbus-timeout-250 doc kernel_linux selinux static-libs systemd test user-session X ubsan"
RESTRICT="
	!x86? ( !amd64? ( test ) )
	!test? ( test )
"

BDEPEND="
	sys-devel/autoconf-archive
	virtual/pkgconfig
	doc? (
		app-doc/doxygen
		app-text/xmlto
		app-text/docbook-xml-dtd:4.4
	)
	test? ( X? ( ${VIRTUALX_DEPEND} ) )
"
COMMON_DEPEND="
	>=dev-libs/expat-2.1.0
	selinux? ( sys-libs/libselinux )
	systemd? ( sys-apps/systemd:0= )
	X? (
		x11-libs/libX11
		x11-libs/libXt
	)
"
DEPEND="${COMMON_DEPEND}
	dev-libs/expat
	test? (
		${PYTHON_DEPS}
		>=dev-libs/glib-2.40:2
	)
"
RDEPEND="${COMMON_DEPEND}
	selinux? ( sec-policy/selinux-dbus )
"

DOC_CONTENTS="
	Some applications require a session bus in addition to the system
	bus. Please see \`man dbus-launch\` for more information.
"

# out of sources build dir for make check
TBD="${WORKDIR}/${P}-tests-build"

pkg_setup() {
	enewgroup messagebus
	enewuser messagebus -1 -1 -1 messagebus

	use test && python-any-r1_pkg_setup

	if use kernel_linux; then
		CONFIG_CHECK="~EPOLL"
		linux-info_pkg_setup
	fi
}

src_prepare() {
	# Increase timeouts for emulated systems that are really slow.
	if use default-dbus-timeout-250; then
		PATCHES+=("${FILESDIR}/${PN}-1.12.20-increase-timeout-by-10x.patch")
	fi

	default
}

src_configure() {
	local rundir=$(usex kernel_linux /run /var/run)
	sed -e "s;@rundir@;${EPREFIX}${rundir};g" "${FILESDIR}"/dbus.initd.in \
		> "${T}"/dbus.initd || die
	meson-multilib_src_configure
}

multilib_src_configure() {
	# Handle ubsan blocklist for https:://crbug.com/1016103
	# TODO: replace with sanitizers-setup-env after verifying VM builds.
	use ubsan && sanitizer-add-blocklist "ubsan"
	local docconf myconf testconf

	# so we can get backtraces from apps
	case ${CHOST} in
		*-mingw*)
			# error: unrecognized command line option '-rdynamic' wrt #488036
			;;
		*)
			append-flags -rdynamic
			;;
	esac

	cros_optimize_package_for_speed

	# libaudit is *only* used in DBus wrt SELinux support, so disable it, if
	# not on an SELinux profile.
	local emesonargs=(
		--localstatedir="${EPREFIX}/var"
		-Druntime_dir="${EPREFIX}/var${rundir}"

		-Ddefault_library=$(multilib_native_usex static-libs both shared)

		-Dapparmor=disabled
		-Dasserts=false # TODO
		-Dchecks=false # TODO
		-Dstats=true
		$(meson_use debug verbose_mode)
		-Ddbus_user=messagebus
		-Dkqueue=disabled
		$(meson_feature kernel_linux inotify)
		$(meson_native_use_feature doc doxygen_docs)
		$(meson_native_use_feature doc xml_docs) # Controls man pages

		-Dinstalled_tests=false
		$(meson_native_true message_bus) # TODO: USE=daemon?
		$(meson_feature test modular_tests)
		-Dqt_help=disabled

		$(meson_native_true tools)

		$(meson_native_use_feature systemd)
		$(meson_use systemd user_session)
		$(meson_native_use_feature X x11_autolaunch)

		# libaudit is *only* used in DBus wrt SELinux support, so disable it if
		# not on an SELinux profile.
		$(meson_native_use_feature selinux)
		$(meson_native_use_feature selinux libaudit)

		-Dsession_socket_dir="${EPREFIX}"/tmp
		-Dsystem_pid_file="${EPREFIX}${rundir}"/dbus.pid
		-Dsystem_socket="${EPREFIX}${rundir}"/dbus/system_bus_socket
		-Dsystemd_system_unitdir="$(systemd_get_systemunitdir)"
		-Dsystemd_user_unitdir="$(systemd_get_userunitdir)"
	)

	if [[ ${CHOST} == *-darwin* ]] ; then
		emesonargs+=(
			-Dlaunchd=enabled
			-Dlaunchd_agent_dir="${EPREFIX}"/Library/LaunchAgents
		)
	fi

	meson_src_configure
}

multilib_src_compile() {
	# After the compile, it uses a selinuxfs interface to
	# check if the SELinux policy has the right support
	use selinux && addwrite /selinux/access

	meson_src_compile
}

multilib_src_test() {
	# https://bugs.gentoo.org/836560
	addwrite /proc/self/oom_score_adj
	# DBUS_TEST_MALLOC_FAILURES=0 to avoid huge test logs
	# https://gitlab.freedesktop.org/dbus/dbus/-/blob/master/CONTRIBUTING.md#L231
	DBUS_TEST_MALLOC_FAILURES=0 DBUS_VERBOSE=1 virtualmake meson_src_test
}

multilib_src_install_all() {
	newinitd "${T}"/dbus.initd dbus

	if use X; then
		# dbus X session script (#77504)
		# turns out to only work for GDM (and startx). has been merged into
		# other desktop (kdm and such scripts)
		exeinto /etc/X11/xinit/xinitrc.d
		doexe "${FILESDIR}"/80-dbus
	fi

	# needs to exist for dbus sessions to launch
	keepdir /usr/share/dbus-1/services
	keepdir /etc/dbus-1/{session,system}.d
	# machine-id symlink from pkg_postinst()
	keepdir /var/lib/dbus
	# let the init script create the /var/run/dbus directory
	rm -rf "${ED}"/var/run

	# https://bugs.gentoo.org/761763
	rm -rf "${ED}"/usr/lib/sysusers.d

	dodoc AUTHORS NEWS README doc/TODO
	readme.gentoo_create_doc

	find "${ED}" -name '*.la' -delete || die
}

pkg_postinst() {
	readme.gentoo_print_elog

	# Ensure unique id is generated and put it in /etc wrt #370451 but symlink
	# for DBUS_MACHINE_UUID_FILE (see tools/dbus-launch.c) and reverse
	# dependencies with hardcoded paths (although the known ones got fixed already)
	# This is handled at runtime on Chrome OS.
	#dbus-uuidgen --ensure="${EROOT}"/etc/machine-id
	#ln -sf "${EPREFIX}"/etc/machine-id "${EROOT}"/var/lib/dbus/machine-id

	if [[ ${CHOST} == *-darwin* ]]; then
		local plist="org.freedesktop.dbus-session.plist"
		elog
		elog
		elog "For MacOS/Darwin we now ship launchd support for dbus."
		elog "This enables autolaunch of dbus at session login and makes"
		elog "dbus usable under MacOS/Darwin."
		elog
		elog "The launchd plist file ${plist} has been"
		elog "installed in ${EPREFIX}/Library/LaunchAgents."
		elog "For it to be used, you will have to do all of the following:"
		elog " + cd ~/Library/LaunchAgents"
		elog " + ln -s ${EPREFIX}/Library/LaunchAgents/${plist}"
		elog " + logout and log back in"
		elog
		elog "If your application needs a proper DBUS_SESSION_BUS_ADDRESS"
		elog "specified and refused to start otherwise, then export the"
		elog "the following to your environment:"
		elog " DBUS_SESSION_BUS_ADDRESS=\"launchd:env=DBUS_LAUNCHD_SESSION_BUS_SOCKET\""
	fi
}
