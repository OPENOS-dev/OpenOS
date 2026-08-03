# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT="59ffe2dba8039845925c96e10ce6c8ac0608063b"
CROS_WORKON_TREE="538271ad9dc000a8ab725048bce2da7c52893b2e"
CROS_WORKON_PROJECT="chromiumos/third_party/scx"

inherit cros-workon meson cros-toolchain-funcs

DESCRIPTION="A collection of eBPF schedulers and tools for sched_ext"
HOMEPAGE="https://github.com/sched-ext/scx"
LICENSE="GPL-2"
KEYWORDS="*"
IUSE="+clang asan"

DEPEND="
	>=dev-libs/libbpf-1.4.0
	app-arch/zstd
	sys-libs/zlib
"

BDEPEND="
	>=dev-util/bpftool-7.4
"

PATCHES=(
	"${FILESDIR}"/scx-libbpf_header_paths.patch
)

src_configure() {
	clang-setup-env
	emesonargs+=(
		-Dbpf_clang=/usr/bin/bpf-clang
		-Dbpftool="disabled"
		-Dlibbpf_a="disabled"
		-Dlibbpf_h="${SYSROOT}/usr/include,${SYSROOT}/usr/include/bpf/uapi"
		-Doffline=true
		-Denable_rust=false
	)
	meson_src_configure
}
