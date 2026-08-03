# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2
# shellcheck disable=SC2034,SC2016,SC1007,SC2102,SC2164,SC2046,SC2086

EAPI="7"
WANT_LIBTOOL="none"

# CrOS: Python breaks with this in ebuilds when they run unittests.
# See examples of breakages in b/433670501.
CROS_SANITIZER_DISABLE_UBSAN_FUNCTION=1

inherit autotools check-reqs flag-o-matic multiprocessing pax-utils
inherit prefix python-utils-r1 cros-toolchain-funcs verify-sig
inherit cros-sanitizers

MY_PV=${PV/_rc/rc}
MY_P="Python-${MY_PV%_p*}"
PYVER=$(ver_cut 1-2)
PATCHSET="python-gentoo-patches-${MY_PV}"

DESCRIPTION="An interpreted, interactive, object-oriented programming language"
HOMEPAGE="
	https://www.python.org/
	https://github.com/python/cpython/
"
SRC_URI="
	https://www.python.org/ftp/python/${PV%%_*}/${MY_P}.tar.xz
	https://dev.gentoo.org/~mgorny/dist/python/${PATCHSET}.tar.xz
	verify-sig? (
		https://www.python.org/ftp/python/${PV%%_*}/${MY_P}.tar.xz.asc
	)
"
S="${WORKDIR}/${MY_P}"

LICENSE="PSF-2"
SLOT="${PYVER}"
KEYWORDS="*"
# CrOS: ensurepip has uses outside of the chromiumos-overlay repository's CQ.
# See e.g. b/378878747 and b/378723931.
IUSE="
	bluetooth build debug +ensurepip examples gdbm libedit
	+ncurses pgo +readline +sqlite +ssl test tk valgrind
"
RESTRICT="!test? ( test )"

# Do not add a dependency on dev-lang/python to this ebuild.
# If you need to apply a patch which requires python for bootstrapping, please
# run the bootstrap code on your dev box and include the results in the
# patchset. See bug 447752.

RDEPEND="
	app-arch/bzip2:=
	app-arch/xz-utils:=
	app-crypt/libb2
	>=dev-libs/expat-2.1:=
	dev-libs/libffi:=
	>=sys-libs/zlib-1.1.3:=
	virtual/libcrypt:=
	virtual/libintl
	ensurepip? ( dev-python/ensurepip-wheels )
	gdbm? ( sys-libs/gdbm:=[berkdb] )
	kernel_linux? ( sys-apps/util-linux:= )
	ncurses? ( >=sys-libs/ncurses-5.2:= )
	readline? (
		!libedit? ( >=sys-libs/readline-4.1:= )
		libedit? ( dev-libs/libedit:= )
	)
	sqlite? ( >=dev-db/sqlite-3.3.8:3= )
	ssl? ( >=dev-libs/openssl-1.1.1:= )
	tk? (
		>=dev-lang/tcl-8.0:=
		>=dev-lang/tk-8.0:=
		dev-tcltk/blt:=
		dev-tcltk/tix
	)
"
# bluetooth requires headers from bluez
DEPEND="
	${RDEPEND}
	bluetooth? ( net-wireless/libbluez:= )
	test? ( app-arch/xz-utils )
	valgrind? ( dev-debug/valgrind )
"
BDEPEND="
	sys-devel/autoconf-archive
	app-alternatives/awk
	virtual/pkgconfig
	verify-sig? ( sec-keys/openpgp-keys-python )
"
RDEPEND+="
	!build? ( app-misc/mime-types )
"
if [[ ${PV} != *_alpha* ]]; then
	RDEPEND+="
		dev-lang/python-exec[python_targets_python${PYVER/./_}(-)]
	"
fi

VERIFY_SIG_OPENPGP_KEY_PATH=/usr/share/openpgp-keys/python.org.asc

QA_PKGCONFIG_VERSION=${PYVER}
# false positives -- functions specific to *BSD
QA_CONFIG_IMPL_DECL_SKIP=( chflags lchflags )

# Google-specific PGO bits
#
# NOTE: If you're looking here because python-*-profile.tar.xz is missing:
# - thanks for upgrading Python!
# - files/python3_gen_pgo.sh should be able to generate a PGO profile for you.
# - only new minor versions of Python should require a new PGO profile.
#
# Generally, PGO profile generation should be done as one of the last steps of
# a Python upgrade, so feel free to turn pgo_use off as a default until things
# are pretty finalized.
SRC_URI+=" pgo_use? ( gs://chromeos-localmirror/distfiles/python-$(ver_cut 1-2)-profile.tar.xz )"
IUSE+=" pgo_generate +pgo_use"
REQUIRED_USE+=" pgo_generate? ( !pgo_use )"

# CrOS: We use the SDK's Python for --with-build-python when cross-compiling.
IUSE+=" cros_host"
BDEPEND+=" !cros_host? ( dev-lang/python:${SLOT} )"

src_unpack() {
	if use verify-sig; then
		verify-sig_verify_detached "${DISTDIR}"/${MY_P}.tar.xz{,.asc}
	fi
	default
}

src_prepare() {
	# Ensure that internal copies of expat and libffi are not used.
	rm -r Modules/expat || die
	rm -r Modules/_ctypes/libffi* || die

	local PATCHES=(
		"${WORKDIR}/${PATCHSET}"
	)

	default

	# START: ChromiumOS
	if tc-is-cross-compiler ; then
		eapply "${FILESDIR}/python-3.11-cross-hack-compiler.patch"
	fi
	eapply "${FILESDIR}/python-3.11-cross-python-config.patch"
	eapply "${FILESDIR}/python-3.11-cross-setup-sysroot.patch"
	eapply "${FILESDIR}/python-3.11-cross-distutils.patch"
	eapply "${FILESDIR}/python-3.11-cross-sysconfig.patch"
	eapply "${FILESDIR}/python-3.11-cross-fix-dictobject-ubsan-failure.patch"
	eapply "${FILESDIR}/python-3.6.5-ldshared.patch"
	eapply "${FILESDIR}/python-3.8-system-libffi.patch"
	eapply "${FILESDIR}/python-3.8-hash-compile-invalidation.patch"
	eapply "${FILESDIR}/python-3.11-CVE-2025-13836.patch"
	eapply "${FILESDIR}/python-3.11-CVE-2025-13462.patch"

	# Support searching /usr/local for Python on dev/test images.
	sed -i -e "s:sys.exec_prefix]:sys.exec_prefix, '/usr/local']:g" \
		Lib/site.py || die "sed failed to add /usr/local to prefixes"

	# Speed up the build and workaround b/240951449 by removing self tests.
	if ! use test ; then
		find "${S}/Lib/" -type f -name 'test_*.py' -delete || die
		rm -rf "${S}/Lib/test" || die
		rm -rf "${S}/Lib/"test_* || die
	fi
	# END: ChromiumOS

	# https://bugs.gentoo.org/850151
	sed -i -e "s:@@GENTOO_LIBDIR@@:$(get_libdir):g" setup.py || die

	# force the correct number of jobs
	# https://bugs.gentoo.org/737660
	local jobs=$(makeopts_jobs)
	sed -i -e "s:-j0:-j${jobs}:" Makefile.pre.in || die
	sed -i -e "/self\.parallel/s:True:${jobs}:" setup.py || die

	eautoreconf
}

src_configure() {
	# disable automagic bluetooth headers detection
	if ! use bluetooth; then
		local -x ac_cv_header_bluetooth_bluetooth_h=no
	fi

	append-flags -fwrapv
	filter-flags -malign-double

	# Export CXX so it ends up in /usr/lib/python3.X/config/Makefile.
	# PKG_CONFIG needed for cross.
	tc-export CXX PKG_CONFIG

	local dbmliborder=
	if use gdbm; then
		dbmliborder+="${dbmliborder:+:}gdbm"
	fi

	if use pgo; then
		local profile_task_flags=(
			-m test
			"-j$(makeopts_jobs)"
			--pgo-extended
			-u-network

			# We use a timeout because of how often we've had hang issues
			# here. It also matches the default upstream PROFILE_TASK.
			--timeout 1200

			-x test_gdb
			-x test_dtrace

			# All of these seem to occasionally hang for PGO inconsistently
			# They'll even hang here but be fine in src_test sometimes.
			# bug #828535 (and related: bug #788022)
			-x test_asyncio
			-x test_concurrent_futures
			-x test_httpservers
			-x test_logging
			-x test_multiprocessing_fork
			-x test_socket
			-x test_xmlrpc

			# Hangs (actually runs indefinitely executing itself w/ many cpython builds)
			# bug #900429
			-x test_tools
		)

		if has_version "app-arch/rpm" ; then
			# Avoid sandbox failure (attempts to write to /var/lib/rpm)
			profile_task_flags+=(
				-x test_distutils
			)
		fi
		local -x PROFILE_TASK="${profile_task_flags[*]}"
	fi

	local myeconfargs=(
		--enable-shared
		--without-static-libpython
		--enable-ipv6
		--infodir='${prefix}/share/info'
		--mandir='${prefix}/share/man'
		--with-computed-gotos
		--with-dbmliborder="${dbmliborder}"
		--with-libc=
		--enable-loadable-sqlite-extensions
		--without-ensurepip
		--without-lto
		--with-system-expat
		--with-system-ffi
		--with-platlibdir=lib
		--with-pkg-config=yes
		--with-wheel-pkg-dir="${EPREFIX}"/usr/lib/python/ensurepip

		$(use_with debug assertions)
		$(use_enable pgo optimizations)
		$(use_with readline readline "$(usex libedit editline readline)")
		$(use_with valgrind)
	)

	if use pgo_use && ! use coverage; then
		append-cppflags "-fprofile-use=${WORKDIR}/code.profclangd"
	elif use pgo_generate; then
		myeconfargs+=(
			"LLVM_PROFDATA=$(which llvm-profdata)"
			--enable-optimizations
		)
	fi

	# disable implicit optimization/debugging flags
	local -x OPT=

	if tc-is-cross-compiler ; then
		myeconfargs+=(
			# CrOS: Use Python from the SDK.
			--with-build-python="/usr/bin/python${PYVER}"
		)
	fi

	# pass system CFLAGS & LDFLAGS as _NODIST, otherwise they'll get
	# propagated to sysconfig for built extensions
	local -x CFLAGS_NODIST=${CFLAGS}
	local -x LDFLAGS_NODIST=${LDFLAGS}
	local -x CFLAGS= LDFLAGS=

	# CrOS: When building for sanitizers, we must preserve the sanitizer flags
	# in CFLAGS/LDFLAGS.  autoconf is unaware of CFLAGS_NODIST and
	# LDFLAGS_NODIST, and will try to link against a sanitizer-enabled version
	# of OpenSSL, which will fail.
	sanitizers-setup-env

	# Fix implicit declarations on cross and prefix builds. Bug #674070.
	if use ncurses; then
		append-cppflags -I"${ESYSROOT}"/usr/include/ncursesw
	fi

	hprefixify setup.py
	econf "${myeconfargs[@]}"

	if grep -q "#define POSIX_SEMAPHORES_NOT_ENABLED 1" pyconfig.h; then
		eerror "configure has detected that the sem_open function is broken."
		eerror "Please ensure that /dev/shm is mounted as a tmpfs with mode 1777."
		die "Broken sem_open function (bug 496328)"
	fi

	# force-disable modules we don't want built
	local disable_modules=( NIS )
	use gdbm || disable_modules+=( _GDBM _DBM )
	use sqlite || disable_modules+=( _SQLITE3 )
	use ssl || disable_modules+=( _HASHLIB _SSL )
	use ncurses || disable_modules+=( _CURSES _CURSES_PANEL )
	use readline || disable_modules+=( READLINE )
	use tk || disable_modules+=( _TKINTER )

	local mod
	for mod in "${disable_modules[@]}"; do
		echo "MODULE_${mod}_STATE=disabled"
	done >> Makefile || die

	# install epython.py as part of stdlib
	echo "EPYTHON='python${PYVER}'" > Lib/epython.py || die
}

src_compile() {
	# Ensure sed works as expected
	# https://bugs.gentoo.org/594768
	local -x LC_ALL=C
	# Prevent using distutils bundled by setuptools.
	# https://bugs.gentoo.org/823728
	export SETUPTOOLS_USE_DISTUTILS=stdlib
	export PYTHONSTRICTEXTENSIONBUILD=1

	# Save PYTHONDONTWRITEBYTECODE so that 'has_version' doesn't
	# end up writing bytecode & violating sandbox.
	# bug #831897
	local -x _PYTHONDONTWRITEBYTECODE=${PYTHONDONTWRITEBYTECODE}

	if use pgo ; then
		# bug 660358
		local -x COLUMNS=80
		local -x PYTHONDONTWRITEBYTECODE=

		addpredict "/usr/lib/python${PYVER}/site-packages"
	fi

	# also need to clear the flags explicitly here or they end up
	# in _sysconfigdata*
	emake CPPFLAGS= CFLAGS= LDFLAGS=

	# Restore saved value from above.
	local -x PYTHONDONTWRITEBYTECODE=${_PYTHONDONTWRITEBYTECODE}

	# Work around bug 329499. See also bug 413751 and 457194.
	if has_version dev-libs/libffi[pax-kernel]; then
		pax-mark E python
	else
		pax-mark m python
	fi
}

src_install() {
	local libdir=${ED}/usr/lib/python${PYVER}
	local python="python${PYVER}"

	# -j1 hack for now for bug #843458
	emake -j1 DESTDIR="${D}" altinstall

	local python="python${PYVER}"
	if ! tc-is-cross-compiler; then
		export LD_LIBRARY_PATH="."
		python="./python"
	fi

	# Patch in our sysconfig.py after install.
	local sysconfig="${libdir}/sysconfig.py"
	cp "${FILESDIR}/sysconfig.py" "${sysconfig}" || die
	"${python}" -m compileall "${sysconfig}" || die

	# Fix collisions between different slots of Python.
	rm "${ED}/usr/$(get_libdir)/libpython3.so" || die

	# Cheap hack to get version with ABIFLAGS
	local abiver=$(cd "${ED}/usr/include"; echo python*)
	if [[ ${abiver} != python${PYVER} ]]; then
		# Replace python3.X with a symlink to python3.Xm
		rm "${ED}/usr/bin/python${PYVER}" || die
		dosym "${abiver}" "/usr/bin/python${PYVER}"
		# Create python3.X-config symlink
		dosym "${abiver}-config" "/usr/bin/python${PYVER}-config"
		# Create python-3.5m.pc symlink
		dosym "python-${PYVER}.pc" "/usr/$(get_libdir)/pkgconfig/${abiver/${PYVER}/-${PYVER}}.pc"
	fi

	# python seems to get rebuilt in src_install (bug 569908)
	# Work around it for now.
	if has_version dev-libs/libffi[pax-kernel]; then
		pax-mark E "${ED}/usr/bin/${abiver}"
	else
		pax-mark m "${ED}/usr/bin/${abiver}"
	fi

	rm -r "${libdir}"/ensurepip/_bundled || die
	if ! use ensurepip; then
		rm -r "${libdir}"/ensurepip || die
	fi
	if ! use sqlite; then
		rm -r "${libdir}/"sqlite3 || die
	fi
	if ! use tk; then
		rm -r "${ED}/usr/bin/idle${PYVER}" || die
		rm -r "${libdir}/"{idlelib,tkinter} || die
	fi

	if use examples; then
		docinto examples
		find Tools -name __pycache__ -exec rm -fr {} + || die
		dodoc -r Tools
	fi
	insinto /usr/share/gdb/auto-load/usr/$(get_libdir) #443510
	local libname=$(
		printf 'e:\n\t@echo $(INSTSONAME)\ninclude Makefile\n' |
		emake --no-print-directory -s -f - 2>/dev/null
	)
	newins Tools/gdb/libpython.py "${libname}"-gdb.py

	newconfd "${FILESDIR}/pydoc.conf" pydoc-${PYVER}
	newinitd "${FILESDIR}/pydoc.init" pydoc-${PYVER}
	sed \
		-e "s:@PYDOC_PORT_VARIABLE@:PYDOC${PYVER/./_}_PORT:" \
		-e "s:@PYDOC@:pydoc${PYVER}:" \
		-i "${ED}/etc/conf.d/pydoc-${PYVER}" \
		"${ED}/etc/init.d/pydoc-${PYVER}" || die "sed failed"

	# python-exec wrapping support
	local pymajor=${PYVER%.*}
	local EPYTHON=python${PYVER}
	local scriptdir=${D}$(python_get_scriptdir)
	mkdir -p "${scriptdir}" || die
	# python and pythonX
	ln -s "../../../bin/${abiver}" "${scriptdir}/python${pymajor}" || die
	ln -s "python${pymajor}" "${scriptdir}/python" || die
	# python-config and pythonX-config
	# note: we need to create a wrapper rather than symlinking it due
	# to some random dirname(argv[0]) magic performed by python-config
	cat > "${scriptdir}/python${pymajor}-config" <<-EOF || die
		#!/bin/sh
		exec "${abiver}-config" "\${@}"
	EOF
	chmod +x "${scriptdir}/python${pymajor}-config" || die
	ln -s "python${pymajor}-config" "${scriptdir}/python-config" || die
	# 2to3, pydoc
	ln -s "../../../bin/2to3-${PYVER}" "${scriptdir}/2to3" || die
	ln -s "../../../bin/pydoc${PYVER}" "${scriptdir}/pydoc" || die
	# idle
	if use tk; then
		ln -s "../../../bin/idle${PYVER}" "${scriptdir}/idle" || die
	fi
}
