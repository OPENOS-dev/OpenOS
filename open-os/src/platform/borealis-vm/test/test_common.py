# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Common code shared by Borealis tests."""

import subprocess


DEBUG = False


def execute_launch_wrapper(*args, env=None, steamid=None, tmp_dir_path=None):
    """Run launch-wrap.py and return the contents of stdout."""
    if not env:
        env = {}
    if steamid:
        env["SteamGameId"] = str(steamid)
    if tmp_dir_path:
        env["LAUNCH_WRAP_TMP"] = tmp_dir_path
    try:
        stdout = subprocess.check_output(
            ["/usr/bin/launch-wrap.sh"] + list(args),
            env=env,
            stderr=subprocess.STDOUT,
        ).decode()
    except subprocess.CalledProcessError as e:
        raise Exception(f"{e}  Command output: {e.stdout}") from e
    if DEBUG:
        print(stdout)
    return stdout
