# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Tests for /usr/bin/get_compat_tool_versions.py."""

from datetime import datetime
from datetime import timezone
import math
import os
import re
import subprocess
import tempfile
import unittest

from test_common import execute_launch_wrapper


DEBUG = False


def execute_get_compat_tool_versions(game_session_log=None, since=None):
    """Run get_compat_tool_versions.py and return the contents of stdout."""
    cmd = ["/usr/bin/get_compat_tool_versions.py"]
    if game_session_log:
        cmd += [f"--game-session-log={game_session_log}"]
    if since is not None:
        cmd += [f"--since={since}"]
    try:
        stdout = subprocess.check_output(cmd, stderr=subprocess.STDOUT).decode()
    except subprocess.CalledProcessError as e:
        raise Exception(f"{e}  Command output: {e.stdout}") from e
    if DEBUG:
        print(stdout)
    return stdout


# pylint: disable=line-too-long
class TestGetCompatToolVersions(unittest.TestCase):
    """Test /usr/bin/get_compat_tool_versions.py internals."""

    def test_no_launch_log(self):
        """Test for no launch log found"""
        stdout = execute_get_compat_tool_versions("/path/does/not/exist")
        self.assertEqual(stdout, "")

    def test_standalone_junk_lines(self):
        """Test for invalid lines ignored"""
        with tempfile.NamedTemporaryFile() as launch_log:
            log_line = b"This is random text that does not resemble launch wrapper log lines."
            launch_log.write(log_line)
            launch_log.flush()
            stdout = execute_get_compat_tool_versions(launch_log.name, 0)
            self.assertEqual(stdout, "")

    def test_standalone_basic(self):
        """Test for expected result from a predefined launch wrapper log line"""
        with tempfile.NamedTemporaryFile() as launch_log:
            log_line = b"2022-11-09T10:46:08-06:00 INFO /usr/bin/launch-wrap.sh/292030[2110]: Running command: /home/chronos/.local/share/Steam/ubuntu12_32/reaper SteamLaunch AppId=292030 -- /home/chronos/.local/share/Steam/ubuntu12_32/steam-launch-wrapper -- '/mnt/external/0/SteamLibrary/steamapps/common/SteamLinuxRuntime_soldier'/_v2-entry-point --verb=waitforexitandrun -- '/mnt/external/0/SteamLibrary/steamapps/common/Proton 7.0'/proton waitforexitandrun  '/mnt/external/0/SteamLibrary/steamapps/common/The Witcher 3/bin/x64/witcher3.exe'"
            launch_log.write(log_line)
            launch_log.flush()
            stdout = execute_get_compat_tool_versions(launch_log.name, 0)
            expected = (
                "GameID: 292030, "
                "Proton: Proton 7.0, "
                "SLR: SteamLinuxRuntime_soldier, "
                "Timestamp: 2022-11-09 10:46:08-06:00"
            ) + "\n"
            self.assertEqual(stdout, expected)

    def test_standalone_at_least_one_printed(self):
        """Test for at least one session info printed despite exceeding the cutoff time"""
        with tempfile.NamedTemporaryFile() as launch_log:
            log_line = b"2022-11-09T10:46:08-06:00 INFO /usr/bin/launch-wrap.sh/292030[2110]: Running command: /home/chronos/.local/share/Steam/ubuntu12_32/reaper SteamLaunch AppId=292030 -- /home/chronos/.local/share/Steam/ubuntu12_32/steam-launch-wrapper -- '/mnt/external/0/SteamLibrary/steamapps/common/SteamLinuxRuntime_soldier'/_v2-entry-point --verb=waitforexitandrun -- '/mnt/external/0/SteamLibrary/steamapps/common/Proton 7.0'/proton waitforexitandrun  '/mnt/external/0/SteamLibrary/steamapps/common/The Witcher 3/bin/x64/witcher3.exe'"
            launch_log.write(log_line)
            launch_log.write(b"\n")
            log_line = b"2022-11-09T15:37:35-06:00 INFO /usr/bin/launch-wrap.sh/489830[6497]: Running command: /home/chronos/.local/share/Steam/ubuntu12_32/reaper SteamLaunch AppId=489830 -- /home/chronos/.local/share/Steam/ubuntu12_32/steam-launch-wrapper -- '/mnt/external/0/SteamLibrary/steamapps/common/SteamLinuxRuntime_soldier'/_v2-entry-point --verb=waitforexitandrun -- '/mnt/external/0/SteamLibrary/steamapps/common/Proton - Experimental'/proton waitforexitandrun  '/mnt/external/0/SteamLibrary/steamapps/common/Skyrim Special Edition/SkyrimSELauncher.exe'"
            launch_log.write(log_line)
            launch_log.flush()
            stdout = execute_get_compat_tool_versions(launch_log.name)
            expected = (
                "GameID: 489830, "
                "Proton: Proton - Experimental, "
                "SLR: SteamLinuxRuntime_soldier, "
                "Timestamp: 2022-11-09 15:37:35-06:00"
            ) + "\n"
            self.assertEqual(stdout, expected)

    def test_standalone_cutoff_time(self):
        """Test for the cutoff time honored"""
        with tempfile.NamedTemporaryFile() as launch_log:
            log_line = b"2022-11-09T10:46:08-06:00 INFO /usr/bin/launch-wrap.sh/292030[2110]: Running command: /home/chronos/.local/share/Steam/ubuntu12_32/reaper SteamLaunch AppId=292030 -- /home/chronos/.local/share/Steam/ubuntu12_32/steam-launch-wrapper -- '/mnt/external/0/SteamLibrary/steamapps/common/SteamLinuxRuntime_soldier'/_v2-entry-point --verb=waitforexitandrun -- '/mnt/external/0/SteamLibrary/steamapps/common/Proton 7.0'/proton waitforexitandrun  '/mnt/external/0/SteamLibrary/steamapps/common/The Witcher 3/bin/x64/witcher3.exe'"
            launch_log.write(log_line)
            launch_log.write(b"\n")
            log_line = b"2022-11-09T15:37:35-06:00 INFO /usr/bin/launch-wrap.sh/489830[6497]: Running command: /home/chronos/.local/share/Steam/ubuntu12_32/reaper SteamLaunch AppId=489830 -- /home/chronos/.local/share/Steam/ubuntu12_32/steam-launch-wrapper -- '/mnt/external/0/SteamLibrary/steamapps/common/SteamLinuxRuntime_soldier'/_v2-entry-point --verb=waitforexitandrun -- '/mnt/external/0/SteamLibrary/steamapps/common/Proton - Experimental'/proton waitforexitandrun  '/mnt/external/0/SteamLibrary/steamapps/common/Skyrim Special Edition/SkyrimSELauncher.exe'"
            launch_log.write(log_line)
            launch_log.write(b"\n")
            log_line = b"2022-11-11T10:07:23-06:00 INFO /usr/bin/launch-wrap.sh/271590[982]: Running command: /home/chronos/.local/share/Steam/ubuntu12_32/reaper SteamLaunch AppId=271590 -- /home/chronos/.local/share/Steam/ubuntu12_32/steam-launch-wrapper -- '/mnt/external/0/SteamLibrary/steamapps/common/SteamLinuxRuntime_soldier'/_v2-entry-point --verb=waitforexitandrun -- '/mnt/external/0/SteamLibrary/steamapps/common/Proton - Experimental'/proton waitforexitandrun  '/mnt/external/0/SteamLibrary/steamapps/common/Grand Theft Auto V/PlayGTAV.exe'"
            launch_log.write(log_line)
            launch_log.flush()
            now = datetime.now(timezone.utc)
            second_last_timestamp = datetime.fromisoformat(
                "2022-11-09T15:37:35-06:00"
            )
            minutes_since = math.ceil(
                (now - second_last_timestamp).total_seconds() / 60
            )
            stdout = execute_get_compat_tool_versions(
                launch_log.name, minutes_since
            )
            expected = (
                "GameID: 271590, "
                "Proton: Proton - Experimental, "
                "SLR: SteamLinuxRuntime_soldier, "
                "Timestamp: 2022-11-11 10:07:23-06:00" + "\n"
                "GameID: 489830, "
                "Proton: Proton - Experimental, "
                "SLR: SteamLinuxRuntime_soldier, "
                "Timestamp: 2022-11-09 15:37:35-06:00" + "\n"
            )
            self.assertEqual(stdout, expected)

    def test_standalone_no_cutoff_time(self):
        """Test for lack of cutoff time honored"""
        with tempfile.NamedTemporaryFile() as launch_log:
            log_line = b"2022-11-09T10:46:08-06:00 INFO /usr/bin/launch-wrap.sh/292030[2110]: Running command: /home/chronos/.local/share/Steam/ubuntu12_32/reaper SteamLaunch AppId=292030 -- /home/chronos/.local/share/Steam/ubuntu12_32/steam-launch-wrapper -- '/mnt/external/0/SteamLibrary/steamapps/common/SteamLinuxRuntime_soldier'/_v2-entry-point --verb=waitforexitandrun -- '/mnt/external/0/SteamLibrary/steamapps/common/Proton 7.0'/proton waitforexitandrun  '/mnt/external/0/SteamLibrary/steamapps/common/The Witcher 3/bin/x64/witcher3.exe'"
            launch_log.write(log_line)
            launch_log.write(b"\n")
            log_line = b"2022-11-09T15:37:35-06:00 INFO /usr/bin/launch-wrap.sh/489830[6497]: Running command: /home/chronos/.local/share/Steam/ubuntu12_32/reaper SteamLaunch AppId=489830 -- /home/chronos/.local/share/Steam/ubuntu12_32/steam-launch-wrapper -- '/mnt/external/0/SteamLibrary/steamapps/common/SteamLinuxRuntime_soldier'/_v2-entry-point --verb=waitforexitandrun -- '/mnt/external/0/SteamLibrary/steamapps/common/Proton - Experimental'/proton waitforexitandrun  '/mnt/external/0/SteamLibrary/steamapps/common/Skyrim Special Edition/SkyrimSELauncher.exe'"
            launch_log.write(log_line)
            launch_log.write(b"\n")
            log_line = b"2022-11-11T10:07:23-06:00 INFO /usr/bin/launch-wrap.sh/271590[982]: Running command: /home/chronos/.local/share/Steam/ubuntu12_32/reaper SteamLaunch AppId=271590 -- /home/chronos/.local/share/Steam/ubuntu12_32/steam-launch-wrapper -- '/mnt/external/0/SteamLibrary/steamapps/common/SteamLinuxRuntime_soldier'/_v2-entry-point --verb=waitforexitandrun -- '/mnt/external/0/SteamLibrary/steamapps/common/Proton - Experimental'/proton waitforexitandrun  '/mnt/external/0/SteamLibrary/steamapps/common/Grand Theft Auto V/PlayGTAV.exe'"
            launch_log.write(log_line)
            launch_log.flush()
            stdout = execute_get_compat_tool_versions(launch_log.name, 0)
            expected = (
                "GameID: 271590, "
                "Proton: Proton - Experimental, "
                "SLR: SteamLinuxRuntime_soldier, "
                "Timestamp: 2022-11-11 10:07:23-06:00" + "\n"
                "GameID: 489830, "
                "Proton: Proton - Experimental, "
                "SLR: SteamLinuxRuntime_soldier, "
                "Timestamp: 2022-11-09 15:37:35-06:00" + "\n"
                "GameID: 292030, "
                "Proton: Proton 7.0, "
                "SLR: SteamLinuxRuntime_soldier, "
                "Timestamp: 2022-11-09 10:46:08-06:00" + "\n"
            )
            self.assertEqual(stdout, expected)

    def test_native_game_session(self):
        """Test that native non-SLR game sessions are parsed correctly"""
        with tempfile.TemporaryDirectory() as launch_tmp:
            FAKE_GAME = "111"
            execute_launch_wrapper(
                "echo game_on", steamid=FAKE_GAME, tmp_dir_path=launch_tmp
            )
            launch_log = os.path.join(launch_tmp, "launch-log.txt")
            stdout = execute_get_compat_tool_versions(launch_log)
            expected = (
                f"GameID: {FAKE_GAME}, Proton: None, SLR: None, " "Timestamp: "
            )
            pattern = expected + r"(.+)\n"
            m = re.fullmatch(pattern, stdout)
            self.assertTrue(m, "Non-SLR native game session format mismatch!")
            try:
                datetime.fromisoformat(m.group(1))
            except ValueError:
                self.fail("Timestamp not found!")

    def test_slr_game_session(self):
        """Test that SLR game sessions are parsed correctly"""
        with tempfile.TemporaryDirectory() as launch_tmp:
            FAKE_GAME = "111"
            FAKE_SLR = "SteamLinuxRuntime_version"
            execute_launch_wrapper(
                f"echo {FAKE_SLR}/run -- game_on",
                steamid=FAKE_GAME,
                tmp_dir_path=launch_tmp,
            )
            launch_log = os.path.join(launch_tmp, "launch-log.txt")
            stdout = execute_get_compat_tool_versions(launch_log)
            expected = (
                f"GameID: {FAKE_GAME}, Proton: None, SLR: {FAKE_SLR}, "
                "Timestamp: "
            )
            pattern = expected + r"(.+)\n"
            m = re.fullmatch(pattern, stdout)
            self.assertTrue(m, "SLR game session format mismatch")
            try:
                datetime.fromisoformat(m.group(1))
            except ValueError:
                self.fail("Timestamp not found!")

    def test_proton_game_session(self):
        """Test that Proton game session are parsed correctly"""
        with tempfile.TemporaryDirectory() as launch_tmp:
            FAKE_GAME = "111"
            FAKE_SLR = "SteamLinuxRuntime_version"
            FAKE_PROTON = "Proton Version"
            execute_launch_wrapper(
                f"echo {FAKE_SLR}/run -- {FAKE_PROTON}/run " "-- game_on.exe",
                steamid=FAKE_GAME,
                tmp_dir_path=launch_tmp,
            )
            launch_log = os.path.join(launch_tmp, "launch-log.txt")
            stdout = execute_get_compat_tool_versions(launch_log)
            expected = (
                f"GameID: {FAKE_GAME}, Proton: {FAKE_PROTON}, "
                f"SLR: {FAKE_SLR}, Timestamp: "
            )
            pattern = expected + r"(.+)\n"
            m = re.fullmatch(pattern, stdout)
            self.assertTrue(m, "Proton game session format mismatch")
            try:
                datetime.fromisoformat(m.group(1))
            except ValueError:
                self.fail("Timestamp not found!")

    def test_multiple_game_session(self):
        """Test that multiple game sessions are parsed correctly"""
        with tempfile.TemporaryDirectory() as launch_tmp:
            FAKE_GAME1 = "111"
            execute_launch_wrapper(
                "echo game_on", steamid=FAKE_GAME1, tmp_dir_path=launch_tmp
            )
            FAKE_GAME2 = "222"
            execute_launch_wrapper(
                "echo game_on", steamid=FAKE_GAME2, tmp_dir_path=launch_tmp
            )
            launch_log = os.path.join(launch_tmp, "launch-log.txt")
            stdout = execute_get_compat_tool_versions(launch_log).splitlines()

            # Second game session should come first (sorted by most recent)
            expected = (
                f"GameID: {FAKE_GAME2}, Proton: None, SLR: None, " "Timestamp: "
            )
            pattern = expected + r"(.+)"
            m = re.fullmatch(pattern, stdout[0])
            self.assertTrue(m, "First game session format mismatch!")
            try:
                datetime.fromisoformat(m.group(1))
            except ValueError:
                self.fail("Timestamp not found in first game session!")

            # First game session should come second (sorted by most recent)
            expected = (
                f"GameID: {FAKE_GAME1}, Proton: None, SLR: None, " "Timestamp: "
            )
            pattern = expected + r"(.+)"
            m = re.fullmatch(pattern, stdout[1])
            self.assertTrue(m, "Second game session format mismatch!")
            try:
                datetime.fromisoformat(m.group(1))
            except ValueError:
                self.fail("Timestamp not found in second game session!")
