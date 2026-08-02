# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Virtual package for listing toolchain packages for the purpose of
determining whether to run the toolchain CQ. All packages which should be
considered part of the toolchain must be directly listed as dependencies of this
package. The common features of packages that should be in this virtual are:

- Packages that consumed by many other packages (to the point of needing to
rebuild a significant chunk of the world to properly test changes).

- Packages that don't fit nicely in to Portage's dependency model (for
bootstrap problems/circular dependencies/other reasons).

- Packages that are significant build-time inputs to either of the above.

Packages which are not tested via CQ (e.g., due to being listed as -nobdeps)
and won't update until a new SDK lands should *not* be listed here.  For
example, at this time of writing, this happens to include firmware toolchains."
HOMEPAGE="http://dev.chromium.org/chromium-os"

LICENSE="metapackage"
SLOT="0"
KEYWORDS="*"
IUSE=""

DEPEND="
	dev-lang/go
	dev-lang/rust
	dev-lang/rust-bootstrap
	dev-lang/rust-host
	dev-libs/elfutils
	sys-devel/autofdo
	sys-devel/binutils
	sys-devel/crossdev
	sys-devel/gcc
	sys-devel/llvm
	sys-kernel/linux-headers
	sys-libs/compiler-rt
	sys-libs/glibc
	sys-libs/libcxx
	sys-libs/libxcrypt
	sys-libs/llvm-libunwind
	sys-libs/newlib
"


src_compile() {
	die "This package is for information only and should never be installed."
}
