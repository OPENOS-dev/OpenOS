# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

DESCRIPTION="Virtual for OpenGLES implementations"

LICENSE="metapackage"
SLOT="0"
KEYWORDS="*"
IUSE="arm64_vm"

DEPEND="
	!arm64_vm? ( media-libs/mesa[egl,gles2] )
	arm64_vm? ( media-libs/mesa-llvmpipe[egl,gles2] )
"
RDEPEND="${DEPEND}"
