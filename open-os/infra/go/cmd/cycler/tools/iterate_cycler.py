#!/usr/bin/env python3

# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""A small script to use cycler to iterate through directories.

Some of directories in chromeos-bot and chromeos-int bot are very large. It can
help to list the subdirs and use this script to iterate through some of them.

This script is not productionized. Please use with caution.
Pairs well with upload_cycler_results.py
"""

# Ensure you're authed with gcloud auth application-default login.

import subprocess

# The bucket you are iterating.
BUCKET = 'YOUR-BUCKET'
# cycler rego policy.
RUN_CONFIG_PATH = 'examples/YOUR-POLICY'
# A new line separated file with the subdirs you want to iterate.
# e.g. `gcloud storage ls gs://chromeos-image-archive > my_subdirs.txt`
# You may also filter the results using `grep`.
DIR_LISTING_PATH = 'YOUR-SUBDIRS'

with open(DIR_LISTING_PATH, 'r', encoding='utf-8') as f:
    gs_dirs = f.readlines()

for gs_dir in gs_dirs:
    print(f'Evaluating: {gs_dir}')
    gs_dir_name = gs_dir.split('/')[-2]

    cycler_cmd = [
        './cycler',
        '-bucket',
        BUCKET,
        '-iUnderstandCyclerIsInEarlyDevelopment',
        '-runConfigPath',
        RUN_CONFIG_PATH,
        '-prefixRoot',
        gs_dir_name,
        '-jsonOutFile',
        f'cycler_results/{gs_dir_name}.json',
    ]
    cmd_str = ' '.join(cycler_cmd)
    print(f'Running {cmd_str}')
    result = subprocess.run(cycler_cmd,
                            check=True,
                            capture_output=True,
                            text=True)
    for l in result.stderr.splitlines():
        print(l)
