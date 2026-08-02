#!/usr/bin/env python3
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Temporary smoke test to run before uploading FirmwareSDK changes to Gerrit.

We will remove this script when FirmwareSDK has a CQ builder.
"""

from pathlib import Path
import subprocess


def main():
    cros_checkout = Path(__file__).resolve().parents[3]
    ec_dir = cros_checkout / "src" / "platform" / "ec"

    subprocess.run(
        [
            cros_checkout / "chromite/bin/bazel",
            "--project",
            "fwsdk",
            "build",
            "//platform/ec:krabby",
        ],
        check=True,
    )
    subprocess.run(
        ["./twister", "-T", "zephyr/test/hooks"], cwd=ec_dir, check=True
    )


if __name__ == "__main__":
    main()
