#!/usr/bin/env python3
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Build the python code related to the labstation image downloader."""


from setuptools import setup


setup(
    name="image_downloader",
    version="0.1",
    maintainer="chromium os",
    maintainer_email="chromium-os-dev@chromium.org",
    license="Chromium",
    package_dir={"image_downloader": "../image_downloader"},
    packages=[
        "image_downloader",
    ],
    entry_points={
        "console_scripts": [
            "image_downloader=image_downloader.image_downloader:main",
        ],
    },
    description="Labstation image downloader tools.",
)
