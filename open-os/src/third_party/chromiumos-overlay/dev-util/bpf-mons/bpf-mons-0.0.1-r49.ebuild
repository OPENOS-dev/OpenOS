# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "fca12aba97787221369ded00a696050278b12c0a")
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_SUBTREE=".gn common-mk bpf-mons"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="bpf-mons"

inherit cros-workon platform

DESCRIPTION="Collection of BPF monitoring programs for in-depth tracing"

LICENSE="BSD-Google"
KEYWORDS="*"
# This package has no unittests.
RESTRICT="test"

RDEPEND="
	dev-libs/libbpf:=
	dev-rust/blazesym-c:=
"

DEPEND="${RDEPEND}
	virtual/linux-sources:=
	dev-cpp/abseil-cpp:=
	dev-libs/elfutils:=
"

BDEPEND="
	virtual/pkgconfig
	dev-util/bpftool
	dev-util/pahole
"
