#!/usr/bin/env python3

# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""A small script to upload cycler results to BQ.

Takes JSON results from cycler, formats according to what BQ requires, and
uploads to specified BQ table. Note that this script uses the particular
schema for chromeos-bot:chromeos_bot_storage.storage.

This script is not productionized. Please use with caution.
Pairs well with iterate_cycler.py
"""

import datetime
import os
import json
import subprocess

# Format of DATASET.TABLE
BQ_TABLE_ID = 'YOUR-BQ-TABLE' # e.g. chromeos_bot_storage.storage
# Dir containing results from iterate_cycler.py, e.g. cycler_results.
RESULTS_DIR_PATH= 'YOUR-RESULTS-DIR'
# Where to write out formatted results, e.g. formatted_cycler_results. Dir must exist.
FORMATTED_RESULTS_DIR_PATH= 'YOUR-FORMATTED-RESULTS-DIR'
# Used in results uploaded to BQ so we can aggregate across buckets.
GS_BUCKET= 'YOUR-GS-BUCKET'

for filename in os.listdir(RESULTS_DIR_PATH):
    # Read and format results to be JSON KVs with newline delimiters per BQ.
    file_path = os.path.join(RESULTS_DIR_PATH, filename)
    formatted = []
    with open(file_path, 'r', encoding='utf-8') as f:
        data = json.load(f)
        print(f"Formatting and uploading {file_path}")
        for k,v in data["PrefixStats"]["PrefixMapSizeBytes"].items():
            target = k.split('/')[0]
            # Skip aggregated prefixes.
            if target != k:
                formatted.append({
                  "target": target,
                  "prefix": k,
                  "bytes": v,
                  # Include upload date for grouping periodic storage snapshots.
                  "date": datetime.datetime.today().strftime('%Y-%m-%d'),
                  "bucket": GS_BUCKET
                })

    # Ensure we have results and aren't trying to upload data for an empty dir.
    if formatted:
        formatted_file_path= os.path.join(FORMATTED_RESULTS_DIR_PATH, filename)
        print(f"Writing out to {formatted_file_path}")
        with open(formatted_file_path, 'w', encoding='utf-8') as f:
            f.write("\n".join(json.dumps(obj) for obj in formatted))

        # Upload row to BQ
        cycler_cmd = [
            'bq',
            'load',
            '--autodetect',
            '--source_format=NEWLINE_DELIMITED_JSON',
            BQ_TABLE_ID,
            formatted_file_path
        ]
        cmd_str = ' '.join(cycler_cmd)
        print(f'Running {cmd_str}')
        result = subprocess.run(cycler_cmd, check=True, capture_output=True, text=True)
        print(result.stdout)
