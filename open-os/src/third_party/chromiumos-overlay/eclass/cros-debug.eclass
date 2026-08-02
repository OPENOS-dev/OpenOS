# Copyright 2010 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# @ECLASS: cros-debug.eclass
# @MAINTAINER:
# ChromiumOS Build Team
# @BUGREPORTS:
# Please report bugs via
# https://issuetracker.google.com/issues/new?component=1037860
# @VCSURL: https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/eclass/@ECLASS@
# @BLURB: Common USE=cros-debug setup logic.
# @DESCRIPTION:
# The cros-debug USE flag controls extra lightweight developer-oriented debug
# features.  It is enabled by default in the CQ and local developer builds, and
# disabled in release builds.  We never enable this flag on shipped devices.
#
# Typically packages inherit this eclass and call `cros-debug-add-NDEBUG` in
# their src_configure.  That adds -DNDEBUG to CPPFLAGS when USE=cros-debug is
# enabled.  C++ packages typically respect this define to compile out APIs like
# assert (standard C library) & DCHECK (libchrome/libbase/libbrillo).
#
# Packages in general should respect USE=cros-debug where it makes sense, even
# if it doesn't check the NDEBUG preprocessor define.
#
# Some general principles for what makes sense for USE=cros-debug:
# * The logic should be "lightweight" at runtime.
# * Extra dependencies (i.e. libraries) should be minimized.
# * Extra logging can be compiled-in, but should be disabled by default, and
#   left to the runtime to turn on (e.g. so a developer can opt-in to).
#
# For everything else, there is the less common USE=debug flag.  These may be
# much more invasive & heavyweight at runtime, pull in extra dependencies, turn
# on extra logging by default, setup additional debug channels at runtime (e.g.
# listening on network ports for connections), etc...  This is never enabled by
# default in any build or package, so more freedom is granted to specific owners
# of packages.

if [[ -z "${_ECLASS_CROS_DEBUG}" ]]; then
_ECLASS_CROS_DEBUG=1

case ${EAPI:-0} in
[0123456]) die "Unsupported EAPI=${EAPI:-0} (too old) for ${ECLASS}" ;;
esac

inherit flag-o-matic

IUSE="cros-debug"

cros-debug-add-NDEBUG() {
	use cros-debug || append-cppflags -DNDEBUG
}

fi
