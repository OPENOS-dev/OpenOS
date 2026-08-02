# Copyright 2018 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2
#
# NOTE:
# This file is maintained by the ChromiumOS SDK builder. It should, generally
# speaking, not be uprevved manually. It's used to force rebuilds on board
# packages when the Rust toolchain is updated, and these updates are only made
# live by the SDK builder. See b/333764881 for more info.
#
# If you do update this file, please **do not symlink** to it. The SDK builder
# `mv`s the most recent version.
#
# DO NOT MODIFY THE NEXT LINE: it's owned by the SDK builder (b/372676968)
# Corresponding package version: dev-lang/rust-1.90.0-r26

EAPI="7"

DESCRIPTION="Virtual for the Rust language compiler"
HOMEPAGE=""

LICENSE="metapackage"
SLOT="1/${PVR}"
KEYWORDS="*"
IUSE=""
