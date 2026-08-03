# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# This is a template for chromeos-base/factory-board package.
# User should copy this file into
# private-overlays/overlay-${BOARD}-private/chromeos-base/factory-board and then
# make further modifications on it. Check
# https://chromeos.google.com/partner/dlm/docs/factory/factorycodelab.html#factory-board

EAPI=7

inherit cros-cpfe cros-factory

DESCRIPTION="Board-specific file for factory software (chromeos-base/factory)."
HOMEPAGE="http://src.chromium.org"
SRC_URI=""
LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"

RESTRICT="mirror strip"
S="${WORKDIR}"

# To install script or other files into toolkit:
# 1. Add files into files/ directory. For example, files/sh/my_script.sh will
#    appear at /usr/local/factory/sh/my_script.sh.

# To install tarballs into toolkit:
# 1. Follow
#    https://chromeos.google.com/partner/dlm/docs/dlm/bcs-binary-upload.html to
#    upload tarballs. Set "Source Upload Path" to "chromeos-base/factory-board".
# 2. Add the `SRC_URI+=" ${CROS_CPFE_URL}/${TARBALL_NAME}"` line.

# Example:
#  SRC_URI=""
#  SRC_URI+=" ${CROS_CPFE_URL}/xxx.tar.gz"

# 3. Run "ebuild-$BOARD <ebuild-name> manifest" to update Manifest file.
# 4. Add `factory_create_resource` into src_install

# Example:
#  Assume that you have a tarball 'xxx.tar.gz' and the contents are

#  xxx-1.0
#  xxx-1.0/xxx-dir
#  xxx-1.0/xxx-dir/xxx.txt
#  xxx-1.0/xxx-file.txt

#  and you want to install xxx-dir/xxx.txt and xxx-file.txt into
#  /usr/local/factory/third_party then add the following line:
#
#  src_install() {
#    cros-factory-board_src_install
#    factory_create_resource "factory-xxx" "${WORKDIR}/xxx-1.0" "third_party" \
#      xxx-dir xxx-file.txt
#  }
