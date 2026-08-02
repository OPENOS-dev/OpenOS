# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_SUBTREE=".gn common-mk bpf-mons"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="bpf-mons"

inherit cros-workon platform

DESCRIPTION="Collection of BPF monitoring programs for in-depth tracing"

LICENSE="BSD-Google"
KEYWORDS="~*"
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
