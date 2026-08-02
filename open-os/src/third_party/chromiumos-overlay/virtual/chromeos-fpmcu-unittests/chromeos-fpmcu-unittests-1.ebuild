# Copyright 2020 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.makefile file.

EAPI=7

DESCRIPTION="ChromeOS fingerprint MCU unittest binaries"

LICENSE="metapackage"
SLOT="0"
KEYWORDS="*"
IUSE=""

# TODO(yichengli): Add other FPMCU boards once the test lab has them.
# NOTE: Any changes here must also be reflected in
# platform/ec-legacy/firmware_builder.py which is used for the ec cq
RDEPEND="
	chromeos-base/chromeos-fpmcu-bloonchipper-unittests
	chromeos-base/chromeos-fpmcu-dartmonkey-unittests
	chromeos-base/chromeos-fpmcu-helipilot-unittests
"
