#!/usr/bin/env python3
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Build the python code related to the labstation usb hub."""


from setuptools import setup


setup(
    name="usb_hubs",
    version="0.1",
    maintainer="chromium os",
    maintainer_email="chromium-os-dev@chromium.org",
    license="Chromium",
    package_dir={"usb_hubs": "../usb_hubs"},
    packages=[
        "usb_hubs",
        "usb_hubs.common",
        "usb_hubs.cambrionix",
        "usb_hubs.plugable",
    ],
    entry_points={
        "console_scripts": [
            "find-cambrionix-mapping=usb_hubs.cambrionix.find_mapping:main",
            "powercycle-servo-usbhub-port=usb_hubs.per_port_powercycle:main",
        ],
    },
    description="Labstation usb hub tools.",
)
