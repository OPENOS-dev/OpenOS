// Copyright 2015 The ChromiumOS Authors
// Modified by Open-OS Project (2026)
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "update_engine/common/platform_constants.h"

namespace chromeos_update_engine {
namespace constants {

const char kOmahaDefaultProductionURL[] =
    "https://update.openos.org/service/update2";
const char kOmahaDefaultAUTestURL[] =
    "https://update-test.openos.org/service/update2";
const char kOmahaUpdaterID[] = "OpenOSUpdateEngine";
const char kOmahaPlatformName[] = "Open-OS";
const char kUpdatePayloadPublicKeyPath[] =
    "/usr/share/update_engine/update-payload-key.pub.pem";
const char kUpdateCertificatesPath[] = "";
const char kCACertificatesPath[] = "/usr/share/chromeos-ca-certificates";
// This directory is wiped during powerwash.
const char kNonVolatileDirectory[] = "/var/lib/update_engine";
const char kRecoveryKeyVersionFileName[] = "recovery_key_version";
const char kPostinstallMountOptions[] = "";

}  // namespace constants
}  // namespace chromeos_update_engine
