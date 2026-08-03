# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

# We're only a cros_workon project so src_test gets run. Use the
# canonical empty project.
CROS_WORKON_COMMIT="d2d95e8af89939f893b1443135497c1f5572aebc"
CROS_WORKON_TREE="776139a53bc86333de8672a51ed7879e75909ac9"
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-workon

DESCRIPTION="CrOS test for tar --hole-detection=raw"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/overlays/portage-stable/+/refs/heads/main/app-arch/tar/cros/tar-1.34-fix-hole-detection-raw.patch"

LICENSE="BSD-Google"
KEYWORDS="*"
RDEPEND="app-arch/tar"

# Disable default install rules (e.g. docs).
src_install() { :; }

src_test() {
	cd "${T}" || die

	# Create a 1MB file that's sparse but the OS doesn't know it.
	dd if=/dev/zero of=zero.bin bs=1M count=1 || die

	# Use tar to compress and tell it to look for sparseness that
	# the OS doesn't know about by using --hole-detection=raw.
	tar cvf zero.tar zero.bin --hole-detection=raw || die

	# The tarball should be much smaller. We'll give an error if it's
	# greater than ~100K.
	local tar_bytes="$(du --block-size=1 zero.tar | awk '{print $1}')"
	if [[ "${tar_bytes}" -gt 100000 ]]; then
		die "Tarball too big (${tar_bytes} bytes); probably not sparse."
	fi

	# Extract the tar. The resulting file should take up 0 bytes.
	rm -f zero.bin || die
	tar xvf zero.tar --sparse || die
	local bin_bytes="$(du --block-size=1 zero.bin | awk '{print $1}')"
	if [[ "${bin_bytes}" -gt 0 ]]; then
		die "Not sparse after extracting."
	fi
}
