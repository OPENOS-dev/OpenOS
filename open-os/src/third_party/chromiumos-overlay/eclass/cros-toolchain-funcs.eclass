# Copyright 2002-2019 Gentoo Authors
# Copyright 2025 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# @ECLASS: cros-toolchain-funcs.eclass
# @MAINTAINER:
# ChromeOS toolchain team <chromeos-toolchain@google.com>
# @BLURB: wrapper and extension for toolchain-funcs
# @DESCRIPTION:
# This eclass provides a wrapper for toolchain-funcs.
# It includes some CrOS-specific extensions.

inherit toolchain-funcs multilib flag-o-matic

# @FUNCTION: tc-get-BUILD_compiler-type
# @RETURN: keyword identifying the compiler for the build machine: gcc, clang, pathcc, unknown
tc-get-BUILD_compiler-type() {
	local code='
#if defined(__PATHSCALE__)
	HAVE_PATHCC
#elif defined(__clang__)
	HAVE_CLANG
#elif defined(__GNUC__)
	HAVE_GCC
#endif
'
	local res=$($(tc-getBUILD_CPP "$@") -E -P - <<<"${code}")

	case ${res} in
		*HAVE_PATHCC*)	echo pathcc;;
		*HAVE_CLANG*)	echo clang;;
		*HAVE_GCC*)		echo gcc;;
		*)				echo unknown;;
	esac
}

# @FUNCTION: tc-getDWP
# @USAGE: [toolchain prefix]
# @RETURN: name of the DWARF package builder
tc-getDWP() { tc-getPROG DWP dwp "$@"; }

# @FUNCTION: tc-getGCOV
# @USAGE: [toolchain prefix]
# @RETURN: name of the test coverage program
tc-getGCOV() { tc-getPROG GCOV gcov "$@"; }

# @FUNCTION: tc-getGO
# @USAGE: [toolchain prefix]
# @RETURN: name of the Go compiler
tc-getGO() { tc-getPROG GO go "$@"; }

# @FUNCTION: tc-getBUILD_DWP
# @USAGE: [toolchain prefix]
# @RETURN: name of the DWARF package builder to run on the build machine
tc-getBUILD_DWP() { tc-getBUILD_PROG DWP dwp "$@"; }

# @FUNCTION: tc-getBUILD_GCOV
# @USAGE: [toolchain prefix]
# @RETURN: name of the test coverage program to run on the build machine
tc-getBUILD_GCOV() { tc-getBUILD_PROG GCOV gcov "$@"; }

# @FUNCTION: tc-getBUILD_OBJDUMP
# @USAGE: [toolchain prefix]
# @RETURN: name of the object dumper to run on the build machine
tc-getBUILD_OBJDUMP() { tc-getBUILD_PROG OBJDUMP objdump "$@"; }

# @FUNCTION: tc-getBUILD_GO
# @USAGE: [toolchain prefix]
# @RETURN: name of the Go compiler for building binaries to run on the build machine
tc-getBUILD_GO() { tc-getBUILD_PROG GO go "$@"; }

tc-getTARGET_PROG() {
	local CTARGET="${CTARGET:-${CHOST}}"
	_tc-getPROG CTARGET "TARGET_$1 $1_FOR_TARGET" "${2#${CHOST}-}" "${@:3}"
}

# @FUNCTION: tc-getTARGET_CC
# @USAGE: [toolchain prefix]
# @RETURN: name of the C compiler for building binaries to run on the target machine
tc-getTARGET_CC() { tc-getTARGET_PROG CC "$(tc-getCC)" "$@"; }
# @FUNCTION: tc-getTARGET_CXX
# @USAGE: [toolchain prefix]
# @RETURN: name of the C++ compiler for building binaries to run on the target machine
tc-getTARGET_CXX() { tc-getTARGET_PROG CXX "$(tc-getCXX)" "$@"; }

# Returns true if gcc builds PIEs
# For ARM, readelf -h | grep Type always has REL instead of EXEC.
# That is why we have to read the flags one by one and check them instead
# of test-compiling a small program.
gcc-pie() {
	for flag in $(echo "void f(){char a[100];}" | \
	${CTARGET}-gcc -v -xc -c -o /dev/null - 2>&1 | \
	grep cc1 | \
	tr " " "\n" | \
	tac)
	do
		if [[ $flag == "-fPIE" || $flag == "-fPIC" ]]
		then
			return 0
		elif [[ $flag == "-fno-PIE" || $flag == "-fno-PIC" ]]
		then
			return 1
		fi
	done
	return 1
}

# Returns true if gcc builds with the stack protector
gcc-ssp() {
	local obj=$(mktemp)
	echo "void f(){char a[100];}" | ${CTARGET}-gcc -xc -c -o ${obj} -
	return $(${CTARGET}-readelf -sW ${obj} | grep -q stack_chk_fail)
}

# Sets up environment variables required to build with Clang
# This should be replaced with a sysroot wrapper ala GCC if/when
# we get serious about building with Clang.
clang-setup-env() {
	use clang || return 0
	# There is no wrapper for host clang.
	if [[ "${CHOST}" == "x86_64-pc-linux-gnu" ]] ; then
		return 0
	fi
	case ${ARCH} in
	amd64|x86|arm|arm64)
		export CC="${CHOST}-clang" CXX="${CHOST}-clang++"
		;;
	*) die "Clang is not yet supported for ${ARCH}"
	esac

	if use asan; then
		local asan_flags=(
			-fsanitize=address
			-fsanitize=alignment
			-fsanitize=shift
		)
		append-flags "${asan_flags[@]}"
		append-ldflags "${asan_flags[@]}"
	fi
}

# @FUNCTION: gen_usr_ldscript
# @USAGE: [-a] <list of libs to create linker scripts for>
# @DESCRIPTION:
# This function is deprecated. Use the version from
# usr-ldscript.eclass instead.
gen_usr_ldscript() {
	ewarn "${FUNCNAME}: Please migrate to usr-ldscript.eclass"

	local lib libdir=$(get_libdir) output_format="" auto=false suffix=$(get_libname)
	[[ -z ${ED+set} ]] && local ED=${D%/}${EPREFIX}/

	tc-is-static-only && return

	# We only care about stuffing / for the native ABI. #479448
	if [[ $(type -t multilib_is_native_abi) == "function" ]] ; then
		multilib_is_native_abi || return 0
	fi

	# Eventually we'd like to get rid of this func completely #417451
	case ${CTARGET:-${CHOST}} in
	*-darwin*) ;;
	*-android*) return 0 ;;
	*linux*|*-freebsd*|*-openbsd*|*-netbsd*)
		use prefix && return 0 ;;
	*) return 0 ;;
	esac

	# Just make sure it exists
	dodir /usr/${libdir}

	if [[ $1 == "-a" ]] ; then
		auto=true
		shift
		dodir /${libdir}
	fi

	# OUTPUT_FORMAT gives hints to the linker as to what binary format
	# is referenced ... makes multilib saner
	local flags=( ${CFLAGS} ${LDFLAGS} -Wl,--verbose )
	if $(tc-getLD) --version | grep -q 'GNU gold' ; then
		# If they're using gold, manually invoke the old bfd. #487696
		local d="${T}/bfd-linker"
		mkdir -p "${d}"
		ln -sf $(which ${CHOST}-ld.bfd) "${d}"/ld
		flags+=( -B"${d}" )
	fi
	output_format=$($(tc-getCC) "${flags[@]}" 2>&1 | sed -n 's/^OUTPUT_FORMAT("\([^"]*\)",.*/\1/p')
	[[ -n ${output_format} ]] && output_format="OUTPUT_FORMAT ( ${output_format} )"

	for lib in "$@" ; do
		local tlib
		if ${auto} ; then
			lib="lib${lib}${suffix}"
		else
			# Ensure /lib/${lib} exists to avoid dangling scripts/symlinks.
			# This especially is for AIX where $(get_libname) can return ".a",
			# so /lib/${lib} might be moved to /usr/lib/${lib} (by accident).
			[[ -r ${ED}/${libdir}/${lib} ]] || continue
			#TODO: better die here?
		fi

		case ${CTARGET:-${CHOST}} in
		*-darwin*)
			if ${auto} ; then
				tlib=$(scanmacho -qF'%S#F' "${ED}"/usr/${libdir}/${lib})
			else
				tlib=$(scanmacho -qF'%S#F' "${ED}"/${libdir}/${lib})
			fi
			[[ -z ${tlib} ]] && die "unable to read install_name from ${lib}"
			tlib=${tlib##*/}

			if ${auto} ; then
				mv "${ED}"/usr/${libdir}/${lib%${suffix}}.*${suffix#.} "${ED}"/${libdir}/ || die
				# some install_names are funky: they encode a version
				if [[ ${tlib} != ${lib%${suffix}}.*${suffix#.} ]] ; then
					mv "${ED}"/usr/${libdir}/${tlib%${suffix}}.*${suffix#.} "${ED}"/${libdir}/ || die
				fi
				rm -f "${ED}"/${libdir}/${lib}
			fi

			# Mach-O files have an id, which is like a soname, it tells how
			# another object linking against this lib should reference it.
			# Since we moved the lib from usr/lib into lib this reference is
			# wrong.  Hence, we update it here.  We don't configure with
			# libdir=/lib because that messes up libtool files.
			# Make sure we don't lose the specific version, so just modify the
			# existing install_name
			if [[ ! -w "${ED}/${libdir}/${tlib}" ]] ; then
				chmod u+w "${ED}${libdir}/${tlib}" # needed to write to it
				local nowrite=yes
			fi
			install_name_tool \
				-id "${EPREFIX}"/${libdir}/${tlib} \
				"${ED}"/${libdir}/${tlib} || die "install_name_tool failed"
			[[ -n ${nowrite} ]] && chmod u-w "${ED}${libdir}/${tlib}"
			# Now as we don't use GNU binutils and our linker doesn't
			# understand linker scripts, just create a symlink.
			pushd "${ED}/usr/${libdir}" > /dev/null
			ln -snf "../../${libdir}/${tlib}" "${lib}"
			popd > /dev/null
			;;
		*)
			if ${auto} ; then
				tlib=$(scanelf -qF'%S#F' "${ED}"/usr/${libdir}/${lib})
				[[ -z ${tlib} ]] && die "unable to read SONAME from ${lib}"
				mv "${ED}"/usr/${libdir}/${lib}* "${ED}"/${libdir}/ || die
				# some SONAMEs are funky: they encode a version before the .so
				if [[ ${tlib} != ${lib}* ]] ; then
					mv "${ED}"/usr/${libdir}/${tlib}* "${ED}"/${libdir}/ || die
				fi
				rm -f "${ED}"/${libdir}/${lib}
			else
				tlib=${lib}
			fi
			cat > "${ED}/usr/${libdir}/${lib}" <<-END_LDSCRIPT
			/* GNU ld script
			   Since Gentoo has critical dynamic libraries in /lib, and the static versions
			   in /usr/lib, we need to have a "fake" dynamic lib in /usr/lib, otherwise we
			   run into linking problems.  This "fake" dynamic lib is a linker script that
			   redirects the linker to the real lib.  And yes, this works in the cross-
			   compiling scenario as the sysroot-ed linker will prepend the real path.

			   See bug https://bugs.gentoo.org/4411 for more info.
			 */
			${output_format}
			GROUP ( ${EPREFIX}/${libdir}/${tlib} )
			END_LDSCRIPT
			;;
		esac
		fperms a+x "/usr/${libdir}/${lib}" || die "could not change perms on ${lib}"
	done
}
