# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Test bdt commands."""

import contextlib
import datetime
import os
import signal
import sys
import tempfile
import threading
import time
import unittest
from unittest import mock

from . import commands


class TestLogConfig(unittest.TestCase):
    """Test of LogConfig class."""

    def setUp(self):
        """Test set up."""
        with contextlib.ExitStack() as stack:
            self.config_dir = stack.enter_context(tempfile.TemporaryDirectory())
            self.cache_dir = stack.enter_context(tempfile.TemporaryDirectory())
            self.mock_config_root = stack.enter_context(
                mock.patch.object(commands, "CONFIG_ROOT", self.config_dir)
            )
            self.mock_cache_root = stack.enter_context(
                mock.patch.object(commands, "CACHE_ROOT", self.cache_dir)
            )
            self.addCleanup(stack.pop_all().close)

        # component1 and component2 are configured with persistent logs
        components = ["component1", "component2"]
        for component in components:
            log_config_path = os.path.join(self.config_dir, component)
            os.mkdir(log_config_path)
            with open(
                os.path.join(log_config_path, "persistent-logs"),
                "w",
                encoding="utf-8",
            ):
                pass
        # component3 and component4 have a config directory
        # but are not configured with persistent logs
        os.mkdir(os.path.join(self.config_dir, "component3"))
        os.mkdir(os.path.join(self.config_dir, "component4"))

    def test_persistent_logs_enabled(self):
        """Test enabled persistent-logs configurations"""
        log_config = commands.LogConfig()
        self.assertEqual(
            log_config.get_path_in_log_dir("component1", "test"),
            os.path.join(self.cache_dir, "component1", "test"),
        )
        self.assertEqual(
            log_config.get_path_in_log_dir("component2", "test"),
            os.path.join(self.cache_dir, "component2", "test"),
        )

    def test_persistent_logs_disabled(self):
        """Test disabled persistent-logs configurations"""
        log_config = commands.LogConfig()
        self.assertEqual(
            log_config.get_path_in_log_dir("component3", "test"),
            "/tmp/test",
        )
        self.assertEqual(
            log_config.get_path_in_log_dir("component4", "test"),
            "/tmp/test",
        )

    def test_persistent_logs_mixed(self):
        """Test a mix of persistent-logs configurations"""
        log_config = commands.LogConfig()
        self.assertEqual(
            log_config.get_path_in_log_dir("component1", "test"),
            os.path.join(self.cache_dir, "component1", "test"),
        )
        self.assertEqual(
            log_config.get_path_in_log_dir("component3", "test"),
            "/tmp/test",
        )

    def test_no_component_config_dir(self):
        """Test when components lack a configuration directory"""
        log_config = commands.LogConfig()
        self.assertEqual(
            log_config.get_path_in_log_dir("component5", "test"),
            "/tmp/test",
        )
        self.assertEqual(
            log_config.get_path_in_log_dir("component6", "test"),
            "/tmp/test",
        )


class TestLaunchOptions(unittest.TestCase):
    """Test of LaunchOptions class."""

    def setUp(self):
        """Test set up."""
        self.opts = commands.LaunchOptions(read=False)
        self.opts.lines = [
            "FOO=1\n",
            "BAR=0\n",
            " BAZ = 2\n",
            "FOOBAR := 3\n",
        ]

    def test_set_add(self):
        """Test set with newly added option."""
        self.opts.set("FLUFF", "three")
        self.assertEqual(
            self.opts.lines,
            [
                "FOO=1\n",
                "BAR=0\n",
                " BAZ = 2\n",
                "FOOBAR := 3\n",
                "FLUFF=three\n",
            ],
        )

    def test_set_replace(self):
        """Test set with replacing pre-existing option."""
        self.opts.set("FOO", "three")
        self.assertEqual(
            self.opts.lines,
            [
                "BAR=0\n",
                " BAZ = 2\n",
                "FOOBAR := 3\n",
                "FOO=three\n",
            ],
        )

    def test_enable_true(self):
        """Test enable with True."""
        self.opts.enable("FOO", True)
        self.assertEqual(
            self.opts.lines,
            [
                "BAR=0\n",
                " BAZ = 2\n",
                "FOOBAR := 3\n",
                "FOO=1\n",
            ],
        )

    def test_enable_false(self):
        """Test enable with False."""
        self.opts.enable("FOO", False)
        self.assertEqual(
            self.opts.lines,
            [
                "BAR=0\n",
                " BAZ = 2\n",
                "FOOBAR := 3\n",
                "FOO=0\n",
            ],
        )


class TestLaunchLogWatcher(unittest.TestCase):
    """Test of LaunchLogWatcher class."""

    def test_open(self):
        """Test open."""
        with tempfile.NamedTemporaryFile("w") as f:
            print("pre-open", file=f, flush=True)
            watcher = commands.LaunchLogWatcher(filename=f.name)
            self.assertEqual(watcher.filename, f.name)
            self.assertIsNotNone(watcher.file)
            print("post-open", file=f, flush=True)
            self.assertEqual(watcher.file.readline(), "post-open\n")

    def test_open_again(self):
        """Test open repeatedly."""
        with tempfile.TemporaryDirectory() as d:
            filename = os.path.join(d, "late")
            watcher = commands.LaunchLogWatcher(filename=filename)
            self.assertEqual(watcher.filename, filename)
            self.assertIsNone(watcher.file)
            with open(filename, "w", encoding="utf-8"):
                self.assertTrue(watcher.open(seek=True))
                self.assertIsNotNone(watcher.file)
                self.assertFalse(watcher.open(seek=True))
                self.assertIsNotNone(watcher.file)

    def test_open_failure(self):
        """Test open with non-existent file."""
        with tempfile.TemporaryDirectory() as d:
            watcher = commands.LaunchLogWatcher(
                filename=os.path.join(d, "invalid")
            )
            self.assertEqual(watcher.filename, os.path.join(d, "invalid"))
            self.assertIsNone(watcher.file)

    def test_await_log(self):
        """Test await_log."""
        with tempfile.NamedTemporaryFile("w") as f:
            print("pre-open", file=f, flush=True)
            watcher = commands.LaunchLogWatcher(filename=f.name)
            # Reduce timeouts.
            watcher.POLL_TIME_S = 0.05
            self.assertEqual(watcher.filename, f.name)
            self.assertIsNotNone(watcher.file)
            print("no-match", file=f, flush=True)
            print("postxx", file=f, flush=True)
            self.assertEqual(watcher.await_log("post", timeout=0.5), "postxx")
            self.assertEqual(watcher.await_log("final", timeout=0.1), None)
            print("final", file=f, flush=True)
            self.assertEqual(watcher.await_log("final", timeout=0.1), "final")

    def test_await_log_late(self):
        """Test await_log with file that is created after watcher."""
        with tempfile.TemporaryDirectory() as d:
            filename = os.path.join(d, "late")
            watcher = commands.LaunchLogWatcher(filename=filename)
            self.assertEqual(watcher.filename, filename)
            self.assertIsNone(watcher.file)
            with open(filename, "w", encoding="utf-8") as f:
                print("initial", file=f, flush=True)
                print("final", file=f, flush=True)
                self.assertEqual(
                    watcher.await_log(match=lambda x: True, timeout=0.5),
                    "initial",
                )
                self.assertIsNotNone(watcher.file)
                self.assertEqual(
                    watcher.await_log(match=lambda x: True, timeout=0.5),
                    "final",
                )


class TestHelpers(unittest.TestCase):
    """Test helper functions and defaults."""

    DEFAULTS = {"xwindump": "X11 window info", "sigint_handler": signal.SIG_DFL}

    @staticmethod
    def pgrep(cmd, capture_output, check):
        """pgrep mock for find_children."""
        del capture_output, check
        pid = cmd[2]
        pid_map = {
            "1": "2\n3\n5\n",
            "2": "7\n",
            "3": "9\n",
            "7": "11\n",
            "11": "13\n",
        }
        result = mock.Mock()
        result.returncode = 0 if pid in pid_map else 1
        result.stdout = pid_map.get(pid, "").encode()
        return result

    @staticmethod
    def xwindump(cmd, env, stdout, check):
        """xwindump mock for window_info"""
        del cmd, env, check
        stdout.write(TestHelpers.DEFAULTS["xwindump"])

    def test_get_ext(self):
        """Test get_ext()."""
        self.assertEqual(commands.get_ext("foo"), "")
        self.assertEqual(commands.get_ext("foo.tar"), "")
        self.assertEqual(commands.get_ext("foo.bz2"), "bz2")
        self.assertEqual(commands.get_ext("foo.tbz2"), "bz2")
        self.assertEqual(commands.get_ext("foo.gz"), "gz")
        self.assertEqual(commands.get_ext("foo.tgz"), "gz")

    @mock.patch("bdt_lib.commands.determine_last_game")
    def test_get_game_id(self, last_game):
        """Test get_game_id()."""
        self.assertEqual(commands.get_game_id("234"), "234")
        self.assertEqual(commands.get_game_id("deadcells"), "588650")
        self.assertEqual(commands.get_game_id("Deadcells"), "588650")
        self.assertEqual(commands.get_game_id("foobar"), None)
        last_game.return_value = None, None
        self.assertEqual(commands.get_game_id(""), None)
        self.assertEqual(commands.get_game_id(None), None)
        last_game.return_value = "27", None
        self.assertEqual(commands.get_game_id(""), "27")
        self.assertEqual(commands.get_game_id(None), "27")

    @mock.patch("subprocess.run")
    def test_find_children(self, run):
        """Test find_children()."""
        run.side_effect = TestHelpers.pgrep
        self.assertEqual(commands.find_children("11", True), ["13"])
        self.assertEqual(commands.find_children("12", True), [])
        self.assertEqual(commands.find_children("1", False), ["2", "3", "5"])
        self.assertEqual(
            commands.find_children("1", True),
            ["2", "3", "5", "7", "11", "13", "9"],
        )

    @mock.patch("builtins.print")
    @mock.patch("os.getpid")
    @mock.patch("builtins.open")
    @mock.patch("datetime.datetime", wraps=datetime.datetime)
    def test_log(self, datetime_mock, open_mock, getpid, print_mock):
        """Test log()."""
        pacific_time = datetime.timezone(datetime.timedelta(hours=-7))
        datetime_mock = mock.Mock(wraps=datetime.datetime)
        datetime_mock.now.return_value = datetime.datetime(
            2022, 4, 12, 15, 8, 39, tzinfo=pacific_time
        )
        with mock.patch("datetime.datetime", new=datetime_mock):
            open_mock.return_value.__enter__.return_value = 27
            getpid.return_value = 8
            commands.log("testing", tz=pacific_time)
            self.assertEqual(open_mock.call_count, 1)
            self.assertEqual(print_mock.call_count, 2)
            print_mock.assert_any_call(
                f"2022-04-12T15:08:39-07:00 INFO {sys.argv[0]}/BDT[8]: testing",
                file=27,
            )
            print_mock.assert_any_call("testing")

    @mock.patch("builtins.print")
    @mock.patch("os.getpid")
    @mock.patch("builtins.open")
    @mock.patch("datetime.datetime", wraps=datetime.datetime)
    def test_log_stderr(self, datetime_mock, open_mock, getpid, print_mock):
        """Test log() to stderr."""
        pacific_time = datetime.timezone(datetime.timedelta(hours=-7))
        datetime_mock = mock.Mock(wraps=datetime.datetime)
        datetime_mock.now.return_value = datetime.datetime(
            2022, 4, 12, 15, 8, 39, tzinfo=pacific_time
        )
        with mock.patch("datetime.datetime", new=datetime_mock):
            open_mock.return_value.__enter__.return_value = 27
            getpid.return_value = 8
            commands.log("testing", tz=pacific_time, stderr=True)
            self.assertEqual(open_mock.call_count, 1)
            self.assertEqual(print_mock.call_count, 2)
            print_mock.assert_any_call(
                f"2022-04-12T15:08:39-07:00 INFO {sys.argv[0]}/BDT[8]: testing",
                file=27,
            )
            print_mock.assert_any_call("testing", file=sys.stderr)

    @mock.patch("builtins.print")
    @mock.patch("os.getpid")
    @mock.patch("builtins.open")
    @mock.patch("datetime.datetime", wraps=datetime.datetime)
    def test_log_exit(self, datetime_mock, open_mock, getpid, print_mock):
        """Test log() with exit."""
        pacific_time = datetime.timezone(datetime.timedelta(hours=-7))
        datetime_mock = mock.Mock(wraps=datetime.datetime)
        datetime_mock.now.return_value = datetime.datetime(
            2022, 4, 12, 15, 8, 39, tzinfo=pacific_time
        )
        with mock.patch("datetime.datetime", new=datetime_mock):
            open_mock.return_value.__enter__.return_value = 27
            getpid.return_value = 8
            with self.assertRaises(SystemExit):
                commands.log(
                    "testing", tz=pacific_time, stderr=True, also_exit=True
                )
            self.assertEqual(open_mock.call_count, 1)
            self.assertEqual(print_mock.call_count, 1)
            print_mock.assert_any_call(
                f"2022-04-12T15:08:39-07:00 INFO {sys.argv[0]}/BDT[8]: testing",
                file=27,
            )

    @mock.patch("datetime.datetime", wraps=datetime.datetime)
    def test_calculate_end_time(self, datetime_mock):
        """Test calculate_end_time()."""
        datetime_mock = mock.Mock(wraps=datetime.datetime)
        datetime_mock.now.return_value = datetime.datetime(
            2022, 4, 12, 15, 8, 39
        )
        with mock.patch("datetime.datetime", new=datetime_mock):
            self.assertEqual(
                commands.calculate_end_time(0),
                datetime.datetime(2022, 4, 12, 15, 8, 39),
            )
            self.assertEqual(
                commands.calculate_end_time(5),
                datetime.datetime(2022, 4, 12, 15, 8, 44),
            )
            self.assertEqual(
                commands.calculate_end_time(6.5),
                datetime.datetime(2022, 4, 12, 15, 8, 45, 500000),
            )
            self.assertEqual(
                commands.calculate_end_time(datetime.timedelta(hours=3)),
                datetime.datetime(2022, 4, 12, 18, 8, 39),
            )

    def test_determine_last_game(self):
        """Test determine_last_game()."""
        lines = [
            "2022-04-13T01:54:45+00:00 INFO /usr/bin/launch-wrap.sh/12[34]:",
            "2022-04-13T01:54:45+00:00 INFO /usr/bin/launch-wrap.sh/56[78]:",
        ]
        with tempfile.NamedTemporaryFile("w", buffering=1) as t:
            for l in lines:
                print(l, file=t)
            with mock.patch("bdt_lib.commands.LAUNCH_LOG_FILENAME", t.name):
                self.assertEqual(commands.determine_last_game(), ("56", "78"))

    def test_determine_last_game_no_log(self):
        """Test determine_last_game() without a log file."""
        with mock.patch(
            "bdt_lib.commands.LAUNCH_LOG_FILENAME", "/tmp/DOES-NOT-EXIST-EVER"
        ):
            self.assertEqual(commands.determine_last_game(), (None, None))

    def test_determine_last_game_no_match(self):
        """Test determine_last_game() without a matching lines."""
        lines = [
            "2022-04-13T01:54:45+00:00 INFO foo/12[34]:",
            "2022-04-13T01:54:45+00:00 INFO foo/56[78]:",
        ]
        with tempfile.NamedTemporaryFile("w", buffering=1) as t:
            for l in lines:
                print(l, file=t)
            with mock.patch("bdt_lib.commands.LAUNCH_LOG_FILENAME", t.name):
                self.assertEqual(commands.determine_last_game(), (None, None))

    def test_is_steam_running(self):
        """Test is_steam_running()."""
        self.assertTrue(commands.is_steam_running("python"))

    def test_read_game_env(self):
        """Test read_game_env()."""
        lines = [
            "STEAMVIDEOTOKEN=500",
            "32f5h290g53047gv5034nbvt923b",
            "",
            "HOST_LC_ALL=en_US.utf8",
            "MESA_DISK_CACHE_SINGLE_FILE=1",
            "SteamAppId=588650",
            "PWD=/home/chronos/.local/share/Steam/steamapps/common/Dead Cells",
        ]
        with tempfile.NamedTemporaryFile("w", buffering=1) as t:
            for l in lines:
                print(l, file=t)
            d = commands.read_game_env(t.name)
            self.assertEqual(len(d), len(lines) - 2)
            self.assertEqual(d["HOST_LC_ALL"], "en_US.utf8")
            self.assertEqual(d["SteamAppId"], "588650")
            self.assertEqual(
                d["PWD"],
                "/home/chronos/.local/share/Steam/steamapps/common/Dead Cells",
            )


class TestCommands(unittest.TestCase):
    """Test top-level commands."""

    def setUp(self):
        """Test set up."""
        # pylint: disable=consider-using-with
        self.tmp_dir = tempfile.TemporaryDirectory()

        # pylint: enable=consider-using-with
        def create(f):
            with open(f, "w", encoding="utf-8"):
                pass

        tmp = os.path.join(self.tmp_dir.name, "tmp")
        os.mkdir(tmp)
        create(os.path.join(tmp, "game.env"))
        create(os.path.join(tmp, "foo"))
        create(os.path.join(tmp, "timing-trace_set_0"))
        create(os.path.join(tmp, "timing-trace_set_2"))
        create(os.path.join(tmp, "launch-log.txt"))
        os.mkdir(os.path.join(tmp, "proton_crashreports"))
        create(os.path.join(tmp, "proton_crashreports", "meow"))

    def tearDown(self):
        """Test tear down."""
        self.tmp_dir.cleanup()

    def test_enable_options_empty(self):
        """Test enable_options starting from empty."""
        with tempfile.NamedTemporaryFile() as t:
            with tempfile.TemporaryDirectory() as d:
                with mock.patch.object(commands, "CONFIG_ROOT", d):
                    commands.enable_options(
                        [
                            "env",
                            "strace",
                            "proton-debug",
                            "wayland-debug-exo",
                            "persistent-logs",
                        ],
                        file=t.name,
                    )
                    self.assertEqual(
                        t.read(), b"ENV_DUMP=1\nSTRACE=1\nPROTON_DEBUG=1\n"
                    )
                    for option, components in commands.CONFIG_ALIASES.items():
                        for component in components:
                            config = os.path.join(d, component, option)
                            if option in (
                                "wayland-debug-exo",
                                "persistent-logs",
                            ):
                                self.assertTrue(os.path.isfile(config))
                            else:
                                self.assertFalse(os.path.isfile(config))

    def test_disable_options_empty(self):
        """Test disable_options starting from empty."""
        with tempfile.NamedTemporaryFile() as t:
            with tempfile.TemporaryDirectory() as d:
                with mock.patch.object(commands, "CONFIG_ROOT", d):
                    commands.disable_options(
                        [
                            "env",
                            "strace",
                            "proton-debug",
                            "wayland-debug-exo",
                            "persistent-logs",
                        ],
                        file=t.name,
                    )
                    self.assertEqual(
                        t.read(), b"ENV_DUMP=0\nSTRACE=0\nPROTON_DEBUG=0\n"
                    )
                    for option, components in commands.CONFIG_ALIASES.items():
                        for component in components:
                            config = os.path.join(d, component, option)
                            self.assertFalse(os.path.isfile(config))

    def test_enable_disable_options(self):
        """Test mixing enable_options and disable_options."""
        with tempfile.NamedTemporaryFile() as t:
            with tempfile.TemporaryDirectory() as d:
                with mock.patch.object(commands, "CONFIG_ROOT", d):
                    commands.enable_options(
                        [
                            "env",
                            "strace",
                            "wayland-debug-exo",
                            "persistent-logs",
                        ],
                        file=t.name,
                    )
                    commands.disable_options(
                        ["strace", "apitrace", "persistent-logs"], file=t.name
                    )
                    self.assertEqual(
                        t.read(), b"ENV_DUMP=1\nSTRACE=0\nAPITRACE=0\n"
                    )
                    for option, components in commands.CONFIG_ALIASES.items():
                        for component in components:
                            config = os.path.join(d, component, option)
                            if option == "wayland-debug-exo":
                                self.assertTrue(os.path.isfile(config))
                            else:
                                self.assertFalse(os.path.isfile(config))

    def test_set_options_empty(self):
        """Test set_options starting from empty."""
        with tempfile.NamedTemporaryFile() as t:
            with tempfile.TemporaryDirectory() as d:
                with mock.patch.object(commands, "CONFIG_ROOT", d):
                    commands.set_options(
                        [
                            "env",
                            "strace",
                            "proton-debug",
                            "wayland-debug-exo",
                            "persistent-logs",
                        ],
                        file=t.name,
                    )
                    self.assertEqual(
                        t.read(), b"ENV_DUMP=1\nSTRACE=1\nPROTON_DEBUG=1\n"
                    )
                    for option, components in commands.CONFIG_ALIASES.items():
                        for component in components:
                            config = os.path.join(d, component, option)
                            if option in (
                                "wayland-debug-exo",
                                "persistent-logs",
                            ):
                                self.assertTrue(os.path.isfile(config))
                            else:
                                self.assertFalse(os.path.isfile(config))

    def test_set_options_non_empty(self):
        """Test set_options starting from non-empty."""
        with tempfile.NamedTemporaryFile() as t:
            with tempfile.TemporaryDirectory() as d:
                with mock.patch.object(commands, "CONFIG_ROOT", d):
                    commands.disable_options(
                        ["strace", "apitrace", "persistent-logs"], file=t.name
                    )
                    commands.enable_options(
                        ["wayland-debug-xwayland", "no-quirks"]
                    )
                    commands.set_options(
                        [
                            "env",
                            "strace",
                            "proton-debug",
                            "wayland-debug-exo",
                            "persistent-logs",
                        ],
                        file=t.name,
                        component=None,
                    )
                    self.assertEqual(
                        t.read(), b"ENV_DUMP=1\nSTRACE=1\nPROTON_DEBUG=1\n"
                    )
                    for option, components in commands.CONFIG_ALIASES.items():
                        for component in components:
                            config = os.path.join(d, component, option)
                            if option in (
                                "wayland-debug-exo",
                                "persistent-logs",
                            ):
                                self.assertTrue(os.path.isfile(config))
                            else:
                                self.assertFalse(os.path.isfile(config))

    @mock.patch("tarfile.open")
    @mock.patch("glob.glob")
    @mock.patch("os.path.exists")
    @mock.patch("bdt_lib.commands.determine_last_game")
    def test_report(self, last_game, exists, glob_mock, tar_open):
        """Test report()."""

        # determine_last_game: Steam ID 27, no PID.
        last_game.return_value = "27", None

        # Fake up a /tmp folder full of files to include in a report.
        # Pretend that everything is present, and any patterns with a
        # '*' match two files.
        exists.return_value = True

        def glob_func(x):
            return ["/tmp/a", "/tmp/b"] if x.find("*") > 0 else [x]

        glob_mock.side_effect = glob_func
        expected_file_count = (
            # Everything in REPORT_FILES.
            sum(2 if "*" in pattern else 1 for pattern in commands.REPORT_FILES)
            # Plus /home/chronos/steam-{steam_id}.log.
            + 1
        )

        # Mock tarfile.open, to track add() calls.
        mock_t = mock.MagicMock()
        tar_open.return_value.__enter__.return_value = mock_t

        commands.report("/tmp/foo.tgz")

        # report() should call os.path.exists() on everything
        # it finds with glob.glob(), then try to add it all
        # to its tarball.
        self.assertEqual(exists.call_count, expected_file_count)
        exists.assert_any_call("/tmp/launch-options.sh")
        exists.assert_called_with("/home/chronos/steam-27.log")
        tar_open.assert_called_once_with("/tmp/foo.tgz", "w:gz")
        self.assertEqual(mock_t.add.call_count, expected_file_count)
        mock_t.add.assert_any_call("/tmp/launch-options.sh")
        mock_t.add.assert_any_call("/tmp/a")
        mock_t.add.assert_any_call("/tmp/b")
        mock_t.add.assert_any_call("/home/chronos/steam-27.log")

    @mock.patch("subprocess.call")
    @mock.patch("bdt_lib.commands.determine_last_game")
    @mock.patch("bdt_lib.commands.is_steam_running")
    def test_start_game(self, steam_running, last_game, sub_call):
        """Test start_game()."""
        steam_running.return_value = True
        last_game.return_value = "27", None
        commands.start_game(None, wait=None)
        self.assertEqual(steam_running.call_count, 1)
        self.assertEqual(last_game.call_count, 1)
        sub_call.assert_called_once_with(["steam", "steam://rungameid/27"])

    @mock.patch("subprocess.call")
    @mock.patch("bdt_lib.commands.determine_last_game")
    @mock.patch("bdt_lib.commands.is_steam_running")
    def test_start_game_no_steam(self, steam_running, last_game, sub_call):
        """Test start_game() without Steam running."""
        steam_running.return_value = False
        last_game.return_value = "27", None
        with self.assertRaises(SystemExit):
            commands.start_game(None, wait=None)
        self.assertEqual(steam_running.call_count, 1)
        last_game.assert_not_called()
        sub_call.assert_not_called()

    @mock.patch("subprocess.call")
    @mock.patch("bdt_lib.commands.determine_last_game")
    @mock.patch("bdt_lib.commands.is_steam_running")
    def test_start_game_no_game(self, steam_running, last_game, sub_call):
        """Test start_game() without game."""
        steam_running.return_value = True
        last_game.return_value = None, None
        with self.assertRaises(SystemExit):
            commands.start_game(None, wait=None)
        self.assertEqual(steam_running.call_count, 1)
        self.assertEqual(last_game.call_count, 1)
        sub_call.assert_not_called()

    @mock.patch("bdt_lib.commands.LaunchLogWatcher")
    @mock.patch("subprocess.call")
    @mock.patch("bdt_lib.commands.determine_last_game")
    @mock.patch("bdt_lib.commands.is_steam_running")
    def test_start_game_wait(self, steam_running, last_game, sub_call, watcher):
        """Test start_game() with wait."""
        steam_running.return_value = True
        last_game.return_value = "27", None
        await_log_mock = mock.Mock()
        watcher.return_value.await_log = await_log_mock
        await_log_mock.return_value = "foo"
        commands.start_game(None, wait="start", timeout=3)
        self.assertEqual(steam_running.call_count, 1)
        self.assertEqual(last_game.call_count, 1)
        sub_call.assert_called_once_with(["steam", "steam://rungameid/27"])
        self.assertEqual(await_log_mock.call_count, 1)
        await_log_mock.assert_called_once_with(
            pattern="Running command: ", timeout=3
        )

    @mock.patch("bdt_lib.commands.LaunchLogWatcher")
    @mock.patch("subprocess.call")
    @mock.patch("bdt_lib.commands.determine_last_game")
    @mock.patch("bdt_lib.commands.is_steam_running")
    def test_start_game_wait_timeout(
        self, steam_running, last_game, sub_call, watcher
    ):
        """Test start_game() with wait timeout."""
        steam_running.return_value = True
        last_game.return_value = "27", None
        await_log_mock = mock.Mock()
        watcher.return_value.await_log = await_log_mock
        await_log_mock.return_value = None
        commands.start_game(None, wait="start", timeout=3)
        self.assertEqual(steam_running.call_count, 1)
        self.assertEqual(last_game.call_count, 1)
        sub_call.assert_called_once_with(["steam", "steam://rungameid/27"])
        self.assertEqual(await_log_mock.call_count, 1)
        await_log_mock.assert_called_once_with(
            pattern="Running command: ", timeout=3
        )

    @mock.patch("bdt_lib.commands.run_steam_console_command")
    @mock.patch("bdt_lib.commands.determine_last_game")
    def test_stop_game(self, last_game, console_command):
        """Test stop_game()."""
        last_game.return_value = "7", None
        commands.stop_game(game=None)
        self.assertEqual(last_game.call_count, 1)
        console_command.assert_called_once_with("app_stop 7")

    @mock.patch("bdt_lib.commands.run_steam_console_command")
    @mock.patch("glob.glob")
    def test_shader_prune(self, fake_glob, console_command):
        """Test shader_prune()."""
        fake_glob.return_value = []
        commands.shader_prune("400", 1, ["/tmp/*"])
        fake_glob.assert_called_once_with("/tmp/*/400/**/replay_cache*")
        self.assertEqual(fake_glob.call_count, 1)
        console_command.assert_called_once_with("shader_prune 400")

    @mock.patch("bdt_lib.commands.run_steam_console_command")
    @mock.patch("glob.glob")
    def test_shader_prune_with_wait(self, fake_glob, console_command):
        """Test shader_prune()."""
        fake_glob.return_value = ["/tmp/replay_cache42"]
        timeout = 10

        thread = threading.Thread(
            target=lambda: commands.shader_prune(
                "400", timeout, ["/tmp/test/*"]
            )
        )
        call_time = datetime.datetime.now()
        thread.start()

        delete_file_time = call_time + datetime.timedelta(seconds=1)
        while datetime.datetime.now() < delete_file_time:
            time.sleep(0.2)
            self.assertTrue(thread.is_alive())
        fake_glob.return_value = []
        thread.join()

        self.assertGreaterEqual(
            timeout, (datetime.datetime.now() - call_time).total_seconds()
        )

        self.assertGreaterEqual(fake_glob.call_count, 1)
        fake_glob.assert_called_with("/tmp/test/*/400/**/replay_cache*")
        console_command.assert_called_once_with("shader_prune 400")

    @mock.patch("subprocess.call")
    @mock.patch("bdt_lib.commands.determine_last_game")
    @mock.patch("bdt_lib.commands.is_steam_running")
    def test_stop_game_no_steam(self, steam_running, last_game, sub_call):
        """Test start_game() without Steam running."""
        steam_running.return_value = False
        last_game.return_value = "7", None
        with self.assertRaises(SystemExit):
            commands.stop_game(game=None)
            commands.start_game(None, wait=None)
        self.assertEqual(steam_running.call_count, 1)
        self.assertEqual(last_game.call_count, 1)
        sub_call.assert_not_called()

    @mock.patch("subprocess.call")
    @mock.patch("bdt_lib.commands.determine_last_game")
    def test_stop_game_no_game(self, last_game, sub_call):
        """Test stop_game() without game."""
        last_game.return_value = None, None
        with self.assertRaises(SystemExit):
            commands.stop_game(None)
        self.assertEqual(last_game.call_count, 1)
        sub_call.assert_not_called()

    @mock.patch("os.kill")
    @mock.patch("subprocess.run")
    @mock.patch("bdt_lib.commands.determine_last_game")
    @mock.patch("bdt_lib.commands.is_steam_running")
    def test_kill_game(self, steam_running, last_game, pgrep, kill):
        """Test kill_game()."""
        steam_running.return_value = True
        last_game.return_value = None, "7"
        pgrep.side_effect = TestHelpers.pgrep
        commands.kill_game(pid=None, sig="SIGHUP", kill_wrapper=False)
        self.assertEqual(steam_running.call_count, 1)
        self.assertEqual(last_game.call_count, 1)
        self.assertEqual(kill.call_count, 2)
        kill.assert_has_calls(
            [mock.call(11, signal.SIGHUP), mock.call(13, signal.SIGHUP)]
        )

    @mock.patch("os.kill")
    @mock.patch("subprocess.run")
    @mock.patch("bdt_lib.commands.determine_last_game")
    @mock.patch("bdt_lib.commands.is_steam_running")
    def test_kill_game_no_steam(self, steam_running, last_game, pgrep, kill):
        """Test kill_game() with no steam."""
        steam_running.return_value = False
        last_game.return_value = None, "7"
        pgrep.side_effect = TestHelpers.pgrep
        with self.assertRaises(SystemExit):
            commands.kill_game(pid=None, sig="SIGHUP", kill_wrapper=False)
        self.assertEqual(steam_running.call_count, 1)
        self.assertEqual(last_game.call_count, 0)
        self.assertEqual(kill.call_count, 0)

    @mock.patch("os.kill")
    @mock.patch("subprocess.run")
    @mock.patch("bdt_lib.commands.determine_last_game")
    @mock.patch("bdt_lib.commands.is_steam_running")
    def test_kill_game_no_pid(self, steam_running, last_game, pgrep, kill):
        """Test kill_game() with no pid."""
        steam_running.return_value = True
        last_game.return_value = None, None
        pgrep.side_effect = TestHelpers.pgrep
        args = mock.Mock()
        args.pid = None
        args.signal = "SIGHUP"
        args.kill_wrapper = False
        with self.assertRaises(SystemExit):
            commands.kill_game(pid=None, sig="SIGHUP", kill_wrapper=False)
        self.assertEqual(steam_running.call_count, 1)
        self.assertEqual(last_game.call_count, 1)
        self.assertEqual(kill.call_count, 0)

    @mock.patch("os.kill")
    @mock.patch("subprocess.run")
    @mock.patch("bdt_lib.commands.determine_last_game")
    @mock.patch("bdt_lib.commands.is_steam_running")
    def test_kill_game_and_wrapper(self, steam_running, last_game, pgrep, kill):
        """Test kill_game(), killing wrapper."""
        steam_running.return_value = True
        last_game.return_value = None, "7"
        pgrep.side_effect = TestHelpers.pgrep
        commands.kill_game(pid=None, sig="SIGINT", kill_wrapper=True)
        self.assertEqual(steam_running.call_count, 1)
        self.assertEqual(last_game.call_count, 1)
        self.assertEqual(kill.call_count, 3)
        kill.assert_has_calls(
            [
                mock.call(11, signal.SIGINT),
                mock.call(13, signal.SIGINT),
                mock.call(7, signal.SIGINT),
            ]
        )

    @mock.patch("bdt_lib.commands.run_steam_console_command")
    def test_set_compat(self, console_command):
        """Test set_compat()."""
        commands.set_compat("portal", "5.13")
        self.assertEqual(console_command.call_count, 1)
        console_command.assert_called_once_with(
            "app_change_compat_tool 400 proton_513 250"
        )

    @mock.patch("subprocess.call")
    @mock.patch("bdt_lib.commands.is_steam_running")
    def test_set_compat_no_steam(self, steam_running, sub_call):
        """Test set_compat() with no steam."""
        steam_running.return_value = False
        with self.assertRaises(SystemExit):
            commands.set_compat("portal", "5.13")
        self.assertEqual(steam_running.call_count, 1)
        self.assertEqual(sub_call.call_count, 0)

    @mock.patch("subprocess.call")
    @mock.patch("bdt_lib.commands.determine_last_game")
    def test_set_compat_no_game(self, last_game, sub_call):
        """Test set_compat() with no game."""
        last_game.return_value = None, None
        with self.assertRaises(SystemExit):
            commands.set_compat(None, "5.13")
        self.assertEqual(last_game.call_count, 1)
        self.assertEqual(sub_call.call_count, 0)

    @mock.patch("os.kill")
    @mock.patch("subprocess.check_output")
    def test_dump_timing_log(self, check_output, kill):
        """Test dump_timing_log()."""
        check_output.return_value = b"8765\n"
        commands.dump_timing_log(pid=None, sig="SIGUSR1")
        self.assertEqual(kill.call_count, 1)
        kill.assert_called_once_with(8765, signal.SIGUSR1)

    @mock.patch("os.kill")
    @mock.patch("subprocess.check_output")
    def test_dump_timing_log_no_pid(self, check_output, kill):
        """Test dump_timing_log()."""
        check_output.return_value = b""
        with self.assertRaises(SystemExit):
            commands.dump_timing_log(pid=None, sig="SIGUSR1")
        self.assertEqual(kill.call_count, 0)

    @mock.patch("subprocess.call")
    @mock.patch("bdt_lib.commands.is_steam_running")
    def test_run_steam_console_command_no_args(self, steam_running, sub_call):
        """Test run_steam_console_command with argument-less command."""
        steam_running.return_value = True
        commands.run_steam_console_command("quit")
        self.assertEqual(steam_running.call_count, 1)
        self.assertEqual(sub_call.call_count, 1)
        sub_call.assert_called_once_with(
            ["steam", "steam://open/console/", "+quit "]
        )

    @mock.patch("subprocess.call")
    @mock.patch("bdt_lib.commands.is_steam_running")
    def test_run_steam_console_command_with_args(self, steam_running, sub_call):
        """Test run_steam_console_command with a command with argument."""
        steam_running.return_value = True
        commands.run_steam_console_command("quit now")
        self.assertEqual(steam_running.call_count, 1)
        self.assertEqual(sub_call.call_count, 1)
        sub_call.assert_called_once_with(
            ["steam", "steam://open/console/", "+quit now"]
        )

    @mock.patch("subprocess.call")
    @mock.patch("bdt_lib.commands.set_compat_layer")
    @mock.patch("bdt_lib.commands.determine_last_game")
    @mock.patch("bdt_lib.commands.is_steam_running")
    def test_install_game(self, steam_running, last_game, compat, sub_call):
        """Test install_game()."""
        steam_running.return_value = True
        last_game.return_value = "27", None
        commands.install_game("deadcells", layer=None)
        self.assertEqual(steam_running.call_count, 1)
        self.assertEqual(last_game.call_count, 0)
        self.assertEqual(compat.call_count, 0)
        sub_call.assert_called_once_with(["steam", "steam://install/588650"])

    @mock.patch("subprocess.call")
    @mock.patch("bdt_lib.commands.set_compat_layer")
    @mock.patch("bdt_lib.commands.determine_last_game")
    @mock.patch("bdt_lib.commands.is_steam_running")
    def test_install_game_no_steam(
        self, steam_running, last_game, compat, sub_call
    ):
        """Test install_game() with no steam."""
        steam_running.return_value = False
        last_game.return_value = "27", None
        with self.assertRaises(SystemExit):
            commands.install_game("deadcells", layer=None)
        self.assertEqual(steam_running.call_count, 1)
        self.assertEqual(last_game.call_count, 0)
        self.assertEqual(compat.call_count, 0)
        self.assertEqual(sub_call.call_count, 0)

    @mock.patch("subprocess.call")
    @mock.patch("bdt_lib.commands.set_compat_layer")
    @mock.patch("bdt_lib.commands.determine_last_game")
    @mock.patch("bdt_lib.commands.is_steam_running")
    def test_install_game_no_game(
        self, steam_running, last_game, compat, sub_call
    ):
        """Test install_game() with no game."""
        steam_running.return_value = True
        last_game.return_value = None, None
        with self.assertRaises(SystemExit):
            commands.install_game(None, layer=None)
        self.assertEqual(steam_running.call_count, 1)
        self.assertEqual(last_game.call_count, 0)
        self.assertEqual(compat.call_count, 0)
        self.assertEqual(sub_call.call_count, 0)

    @mock.patch("subprocess.call")
    @mock.patch("bdt_lib.commands.set_compat_layer")
    @mock.patch("bdt_lib.commands.determine_last_game")
    @mock.patch("bdt_lib.commands.is_steam_running")
    def test_install_game_layer(
        self, steam_running, last_game, compat, sub_call
    ):
        """Test install_game()."""
        steam_running.return_value = True
        last_game.return_value = "27", None
        commands.install_game("96", layer="7")
        self.assertEqual(steam_running.call_count, 1)
        self.assertEqual(last_game.call_count, 0)
        compat.assert_called_once_with("96", "7")
        sub_call.assert_called_once_with(["steam", "steam://install/96"])

    @mock.patch("bdt_lib.commands.LaunchLogWatcher")
    def test_await_log(self, watcher_mock):
        """Test await_log()."""
        await_log_mock = mock.Mock()
        watcher_mock.return_value.await_log = await_log_mock
        await_log_mock.return_value = "foo"
        commands.await_log("start", pattern="cats", timeout=2)
        self.assertEqual(await_log_mock.call_count, 1)
        await_log_mock.assert_called_once_with(
            pattern="Running command: ", timeout=2
        )

    @mock.patch("bdt_lib.commands.LaunchLogWatcher")
    def test_await_log_pattern(self, watcher):
        """Test await_log() with pattern."""
        await_log_mock = mock.Mock()
        watcher.return_value.await_log = await_log_mock
        await_log_mock.return_value = None
        commands.await_log(None, pattern="cats", timeout=None)
        self.assertEqual(await_log_mock.call_count, 1)
        await_log_mock.assert_called_once_with(pattern="cats", timeout=None)

    def test_launch_option_set(self):
        """Test launch_options_set()."""
        with tempfile.NamedTemporaryFile("r+") as f:
            commands.launch_option_set("env", True, f.name)
            self.assertEqual(f.readlines(), ["ENV_DUMP=1\n"])

    def test_config_option_set(self):
        """Test config_options_set()."""
        with tempfile.TemporaryDirectory() as d:
            with mock.patch.object(commands, "CONFIG_ROOT", d):
                commands.config_option_set("persistent-logs", True, None)
                sommelier_config = os.path.join(
                    d, "sommelier", "persistent-logs"
                )
                launch_wrap_config = os.path.join(
                    d, "launch-wrap", "persistent-logs"
                )
                self.assertTrue(os.path.isfile(sommelier_config))
                self.assertTrue(os.path.isfile(launch_wrap_config))

    def test_config_option_set_component(self):
        """Test config_options_set() with a component filter."""
        with tempfile.TemporaryDirectory() as d:
            with mock.patch.object(commands, "CONFIG_ROOT", d):
                commands.config_option_set("persistent-logs", True, "sommelier")
                sommelier_config = os.path.join(
                    d, "sommelier", "persistent-logs"
                )
                launch_wrap_config = os.path.join(
                    d, "launch-wrap", "persistent-logs"
                )
                self.assertTrue(os.path.isfile(sommelier_config))
                self.assertFalse(os.path.isfile(launch_wrap_config))

    @mock.patch("os.unlink")
    @mock.patch("shutil.rmtree")
    def test_clean_files(self, rmtree, unlink):
        """Test clean_files()."""
        self.assertIsNone(commands.clean_files(basedir=self.tmp_dir.name))
        self.assertEqual(unlink.call_count, 2)
        self.assertEqual(rmtree.call_count, 1)

    @mock.patch("os.unlink")
    @mock.patch("shutil.rmtree")
    def test_clean_files_all(self, rmtree, unlink):
        """Test clean_files() with all_files=True."""
        self.assertIsNone(
            commands.clean_files(all_files=True, basedir=self.tmp_dir.name)
        )
        self.assertEqual(unlink.call_count, 3)
        self.assertEqual(rmtree.call_count, 1)

    @mock.patch("tarfile.open")
    @mock.patch("bdt_lib.commands.determine_last_game")
    def test_save_compat(self, last_game, tar_open):
        """Test save_compat()."""
        last_game.return_value = "27", None
        mock_t = mock.MagicMock()
        tar_open.return_value.__enter__.return_value = mock_t
        with tempfile.TemporaryDirectory() as d:
            env_file = os.path.join(d, "game.env")
            compat_dir = os.path.join(d, "compat")
            compat_file = os.path.join(d, "compat.tar")
            with open(env_file, "w", encoding="utf-8") as f:
                print(f"STEAM_COMPAT_DATA_PATH={compat_dir}", file=f)
            os.mkdir(compat_dir)
            commands.save_compat(env=env_file, output=compat_file)
            tar_open.assert_called_once_with(compat_file, "w:")
            self.assertEqual(mock_t.add.call_count, 1)
            mock_t.add.assert_called_once_with(compat_dir)
            self.assertTrue(os.path.exists(compat_dir))

    @mock.patch("tarfile.open")
    @mock.patch("os.path.isdir")
    @mock.patch("bdt_lib.commands.determine_last_game")
    def test_save_compat_no_game(self, last_game, isdir, tar_open):
        """Test save_compat() via non-determinable game."""
        last_game.return_value = None, None
        isdir.return_value = False
        mock_t = mock.MagicMock()
        tar_open.return_value.__enter__.return_value = mock_t
        with tempfile.TemporaryDirectory() as d:
            compat_file = os.path.join(d, "compat.tar")
            with self.assertRaises(SystemExit):
                commands.save_compat(output=compat_file)
            self.assertEqual(isdir.call_count, 0)
            self.assertEqual(tar_open.call_count, 0)

    @mock.patch("tarfile.open")
    @mock.patch("bdt_lib.commands.determine_last_game")
    def test_save_compat_delete(self, last_game, tar_open):
        """Test save_compat() with delete."""
        last_game.return_value = "27", None
        mock_t = mock.MagicMock()
        tar_open.return_value.__enter__.return_value = mock_t
        with tempfile.TemporaryDirectory() as d:
            env_file = os.path.join(d, "game.env")
            compat_dir = os.path.join(d, "compat")
            compat_file = os.path.join(d, "compat.tar")
            with open(env_file, "w", encoding="utf-8") as f:
                print(f"STEAM_COMPAT_DATA_PATH={compat_dir}", file=f)
            os.mkdir(compat_dir)
            with open(
                os.path.join(compat_dir, "test-file"), "w", encoding="utf-8"
            ):
                pass
            commands.save_compat(
                path=None,
                env=env_file,
                game=None,
                output=compat_file,
                delete=True,
            )
            tar_open.assert_called_once_with(compat_file, "w:")
            self.assertEqual(mock_t.add.call_count, 1)
            mock_t.add.assert_called_once_with(compat_dir)
            self.assertFalse(os.path.exists(compat_dir))

    @mock.patch("bdt_lib.commands.save_compat")
    @mock.patch("bdt_lib.commands.report")
    @mock.patch("bdt_lib.commands.stop_game")
    @mock.patch("bdt_lib.commands.start_game")
    @mock.patch("bdt_lib.commands.LaunchLogWatcher")
    def test_script_attempt_start(
        self, watcher, start, stop, report_mock, compat
    ):
        """Test script_attempt_start()."""
        await_log_mock = mock.Mock()
        watcher.return_value.await_log = await_log_mock
        await_log_mock.return_value = "foo"
        commands.script_attempt_start("27")
        self.assertEqual(start.call_count, 1)
        self.assertEqual(stop.call_count, 0)
        self.assertEqual(await_log_mock.call_count, 2)
        self.assertEqual(report_mock.call_count, 1)
        self.assertEqual(compat.call_count, 0)

    @mock.patch("bdt_lib.commands.save_compat")
    @mock.patch("bdt_lib.commands.report")
    @mock.patch("bdt_lib.commands.stop_game")
    @mock.patch("bdt_lib.commands.start_game")
    @mock.patch("bdt_lib.commands.LaunchLogWatcher")
    def test_script_attempt_start_compat(
        self, watcher, start, stop, report_mock, compat
    ):
        """Test script_attempt_start()."""
        await_log_mock = mock.Mock()
        watcher.return_value.await_log = await_log_mock
        await_log_mock.return_value = "foo"
        commands.script_attempt_start("27", save_compat_post=True)
        self.assertEqual(start.call_count, 1)
        self.assertEqual(stop.call_count, 0)
        self.assertEqual(await_log_mock.call_count, 2)
        self.assertEqual(report_mock.call_count, 1)
        self.assertEqual(compat.call_count, 1)

    @mock.patch("bdt_lib.commands.report")
    @mock.patch("bdt_lib.commands.stop_game")
    @mock.patch("bdt_lib.commands.start_game")
    @mock.patch("bdt_lib.commands.LaunchLogWatcher")
    def test_script_attempt_start_no_start(
        self, watcher, start, stop, report_mock
    ):
        """Test script_attempt_start() which doesn't start."""
        await_log_mock = mock.Mock()
        watcher.return_value.await_log = await_log_mock
        await_log_mock.return_value = None
        commands.script_attempt_start("27")
        self.assertEqual(start.call_count, 1)
        self.assertEqual(stop.call_count, 0)
        self.assertEqual(await_log_mock.call_count, 1)
        self.assertEqual(report_mock.call_count, 1)

    @mock.patch("bdt_lib.commands.report")
    @mock.patch("bdt_lib.commands.stop_game")
    @mock.patch("bdt_lib.commands.start_game")
    @mock.patch("bdt_lib.commands.LaunchLogWatcher")
    def test_script_attempt_start_stop(self, watcher, start, stop, report_mock):
        """Test script_attempt_start() with stopping."""
        await_log_mock = mock.Mock()
        watcher.return_value.await_log = await_log_mock
        await_log_mock.side_effect = ["started", None, "stopped"]
        commands.script_attempt_start("27")
        self.assertEqual(start.call_count, 1)
        self.assertEqual(stop.call_count, 1)
        self.assertEqual(await_log_mock.call_count, 3)
        self.assertEqual(report_mock.call_count, 1)

    @mock.patch("bdt_lib.commands.report")
    @mock.patch("bdt_lib.commands.stop_game")
    @mock.patch("bdt_lib.commands.start_game")
    @mock.patch("bdt_lib.commands.LaunchLogWatcher")
    def test_script_attempt_start_no_stop(
        self, watcher, start, stop, report_mock
    ):
        """Test script_attempt_start() which doesn't stop."""
        await_log_mock = mock.Mock()
        watcher.return_value.await_log = await_log_mock
        await_log_mock.side_effect = ["started", None, None]
        commands.script_attempt_start("27")
        self.assertEqual(start.call_count, 1)
        self.assertEqual(stop.call_count, 1)
        self.assertEqual(await_log_mock.call_count, 3)
        self.assertEqual(report_mock.call_count, 1)

    @mock.patch("bdt_lib.commands.script_attempt_start")
    @mock.patch("bdt_lib.commands.save_compat")
    @mock.patch("bdt_lib.commands.set_compat_layer")
    @mock.patch("bdt_lib.commands.set_options")
    @mock.patch("bdt_lib.commands.clean_files")
    def test_script_no_start(
        self, clean, options, compat_mock, save_compat_mock, attempt_start
    ):
        """Test script_no_start()."""
        stages_to_test = 10
        options_set = {}

        def count_options(opts):
            for o in opts:
                options_set[o] = options_set.get(o, 0) + 1

        options.side_effect = count_options
        compat_set = {}

        def count_compat(_, compat):
            compat_set[compat] = compat_set.get(compat, 0) + 1

        compat_mock.side_effect = count_compat
        for i in range(stages_to_test):
            commands.script_no_start(str(i + 1), "27")
        self.assertEqual(clean.call_count, stages_to_test)
        self.assertEqual(options.call_count, stages_to_test)
        self.assertEqual(options_set["env"], stages_to_test)
        self.assertEqual(options_set["output"], stages_to_test)
        self.assertGreaterEqual(options_set["strace"], 2)
        self.assertGreaterEqual(options_set["proton-debug"], 1)
        self.assertGreaterEqual(options_set["pressure-vessel-debug"], 1)
        self.assertEqual(compat_mock.call_count, stages_to_test - 2)
        self.assertGreaterEqual(compat_set["proton-stable"], 1)
        self.assertGreaterEqual(compat_set["proton-exp"], 1)
        self.assertGreaterEqual(save_compat_mock.call_count, 1)
        self.assertEqual(attempt_start.call_count, stages_to_test)
        attempt_start.assert_any_call("27", None, True)
        attempt_start.assert_any_call("27", None, False)

    @mock.patch("bdt_lib.commands.script_attempt_start")
    @mock.patch("bdt_lib.commands.set_compat_layer")
    @mock.patch("bdt_lib.commands.set_options")
    @mock.patch("bdt_lib.commands.clean_files")
    @mock.patch("bdt_lib.commands.determine_last_game")
    def test_script_no_start_no_game(
        self, last_game, clean, options, compat_mock, attempt_start
    ):
        """Test script_no_start() with no game."""
        last_game.return_value = None, None
        with self.assertRaises(SystemExit):
            commands.script_no_start("1", None)
        self.assertEqual(clean.call_count, 0)
        self.assertEqual(options.call_count, 0)
        self.assertEqual(compat_mock.call_count, 0)
        self.assertEqual(attempt_start.call_count, 0)

    @mock.patch("bdt_lib.commands.script_attempt_start")
    @mock.patch("bdt_lib.commands.set_compat_layer")
    @mock.patch("bdt_lib.commands.set_options")
    @mock.patch("bdt_lib.commands.clean_files")
    def test_script_no_start_final(
        self, clean, options, compat_mock, attempt_start
    ):
        """Test script_no_start() on final stage."""
        with self.assertRaises(SystemExit):
            commands.script_no_start("12", "27")
        self.assertEqual(clean.call_count, 1)
        self.assertEqual(options.call_count, 0)
        self.assertEqual(compat_mock.call_count, 0)
        self.assertEqual(attempt_start.call_count, 0)

    @mock.patch("subprocess.run")
    def test_window_info(self, run_mock):
        """Test window_info()."""
        run_mock.side_effect = TestHelpers.xwindump
        with tempfile.NamedTemporaryFile() as tmp:
            commands.window_info(tmp.name)
            self.assertEqual(run_mock.call_count, 1)
            run_mock.assert_called_with(
                ["xwindump.py", "--verbose"],
                env=os.environ,
                stdout=mock.ANY,
                check=True,
            )
            with open(tmp.name, encoding="utf-8") as f:
                expected = TestHelpers.DEFAULTS["xwindump"]
                self.assertEqual([expected], f.readlines())

    @mock.patch("subprocess.Popen")
    @mock.patch("signal.signal")
    @mock.patch("signal.pause")
    def test_metrics(self, pause_mock, signal_mock, popen_mock):
        """Test window_info()."""
        signal_mock.side_effect = [
            TestHelpers.DEFAULTS["sigint_handler"],
            mock.ANY,
        ]
        with tempfile.NamedTemporaryFile() as tmp:
            commands.metrics(tmp.name)
            self.assertEqual(popen_mock.call_count, 1)
            popen_mock.assert_called_with(
                ["detailed_metrics", "--quick-stats", "-o", tmp.name, "--quiet"]
            )
            self.assertEqual(signal_mock.call_count, 2)
            signal_mock.assert_has_calls(
                [
                    mock.call(signal.SIGINT, mock.ANY),
                    mock.call(
                        signal.SIGINT, TestHelpers.DEFAULTS["sigint_handler"]
                    ),
                ]
            )
            self.assertEqual(pause_mock.call_count, 1)
