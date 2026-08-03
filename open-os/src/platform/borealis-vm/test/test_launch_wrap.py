# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Tests for /usr/bin/launch-wrap.py."""

import os
import tempfile
import unittest

import test_common


class TestLaunchOptions(unittest.TestCase):
    """Test overriding things using launch-options.sh."""

    def test_output_dir(self):
        """Test that setting OUTPUT_DIR moves all output files."""
        with tempfile.TemporaryDirectory() as launch_tmp:
            with tempfile.TemporaryDirectory() as output_dir:
                # Enable game output redirection, and specify an output dir.
                with open(
                    f"{launch_tmp}/launch-options.sh", "w", encoding="utf-8"
                ) as f:
                    f.write(f"OUTPUT_DUMP=1\nOUTPUT_DIR={output_dir}\n")
                test_common.execute_launch_wrapper(
                    "echo this should be redirected", tmp_dir_path=launch_tmp
                )
                # Make sure a log message about reading launch options was written.
                with open(
                    f"{output_dir}/launch-log.txt", encoding="utf-8"
                ) as f:
                    file_contents = f.read()
                    self.assertIn(
                        f"Read launch options from {launch_tmp}", file_contents
                    )
                    self.assertIn(
                        f"Launch options read-in:\nOUTPUT_DUMP=1\nOUTPUT_DIR={output_dir}\n",
                        file_contents,
                    )
                # Make sure game output was logged to the specified output dir.
                with open(f"{output_dir}/game.log", encoding="utf-8") as f:
                    self.assertIn("this should be redirected", f.read())

    def test_output_dump(self):
        """Test that OUTPUT_DUMP=1 redirects game output."""
        with tempfile.TemporaryDirectory() as launch_tmp:
            # Enable game output redirection.
            with open(
                f"{launch_tmp}/launch-options.sh", "w", encoding="utf-8"
            ) as f:
                f.write("OUTPUT_DUMP=1\n")
            test_common.execute_launch_wrapper(
                "echo this should be redirected", tmp_dir_path=launch_tmp
            )
            # Make sure game output was logged.
            game_log_path = f"{launch_tmp}/game.log"
            self.assertTrue(os.path.exists(game_log_path))
            with open(game_log_path, encoding="utf-8") as f:
                self.assertIn("this should be redirected", f.read())

    def test_paths_needing_escaping(self):
        """Test that spaces, single quotes, and double quotes in paths are handled correctly."""
        for config, output in [
            ("STRACE=1", "game.strace"),
            ("APITRACE=1", "game.trace"),
            ("OUTPUT_DUMP=1", "game.log"),
            ("ENV_DUMP=1", "game.env"),
        ]:
            with self.subTest(
                msg=f"Running with {config} and checking for {output}"
            ):
                with tempfile.TemporaryDirectory(
                    suffix=" \"needs 'quoting"
                ) as launch_tmp:
                    # Enable one output file.
                    with open(
                        f"{launch_tmp}/launch-options.sh", "w", encoding="utf-8"
                    ) as f:
                        f.write(f"{config}\n")
                    log = test_common.execute_launch_wrapper(
                        "/bin/bash -c echo hello", tmp_dir_path=launch_tmp
                    )
                    # Dump output if this resulted in "error: failed to execute:"
                    self.assertNotIn("error", log)
                    # Check that all expected files were created.
                    if config == "APITRACE=1":
                        pass  # apitrace won't write anything for non-graphical commands
                    else:
                        self.assertTrue(
                            os.path.exists(f"{launch_tmp}/{output}")
                        )


class TestLaunchWrap(unittest.TestCase):
    """Test /usr/bin/launch-wrap.py internals."""

    def test_trivial(self):
        """Test that launch-wrap.sh is runnable."""
        self.assertIn(
            "\nhello\n", test_common.execute_launch_wrapper("echo hello")
        )

    def test_add_cmd_suffix_if_not_present(self):
        """Test the add_cmd_suffix_if_not_present function."""
        # Fake SteamGameID that causes launch-wrap.sh to run a test wrapper.
        FAKE_GAME = "fake_test_steamid"
        FAKE_GAME_PATH = f"/etc/launch-wrapper.d/{FAKE_GAME}.sh"
        try:
            # Write test wrapper
            with open(FAKE_GAME_PATH, "w", encoding="utf-8") as f:
                f.write(
                    r"""
log "Applying test game override."

# Append +gl unless +gl or any of the regexes following are present.
add_cmd_suffix_if_not_present "+gl" "-vulkan" "\+hello world" "abc[def]+ghi"
"""
                )
            # Make sure +gl gets added
            self.assertIn(
                "\nfoo +gl\n",
                test_common.execute_launch_wrapper(
                    "echo foo", steamid=FAKE_GAME
                ),
            )
            # Make sure -vulkan causes +gl to not get added
            self.assertIn(
                "\nfoo -vulkan\n",
                test_common.execute_launch_wrapper(
                    "echo foo -vulkan", steamid=FAKE_GAME
                ),
            )
            # Make sure +gl isn't added if it's already there
            self.assertIn(
                "\nfoo +gl\n",
                test_common.execute_launch_wrapper(
                    "echo foo +gl", steamid=FAKE_GAME
                ),
            )
            # Make sure the word boundary match is working
            self.assertIn(
                "\nfoo +gll +gl\n",
                test_common.execute_launch_wrapper(
                    "echo foo +gll", steamid=FAKE_GAME
                ),
            )
            self.assertIn(
                "\nfoo x+gl +gl\n",
                test_common.execute_launch_wrapper(
                    "echo foo x+gl", steamid=FAKE_GAME
                ),
            )
            # Make sure patterns with spaces are working
            self.assertIn(
                "\nfoo +hello world\n",
                test_common.execute_launch_wrapper(
                    "echo foo +hello world", steamid=FAKE_GAME
                ),
            )
            # Make sure regex patterns are working
            self.assertIn(
                "\nfoo abcdefdefdefghi\n",
                test_common.execute_launch_wrapper(
                    "echo foo abcdefdefdefghi", steamid=FAKE_GAME
                ),
            )
        finally:
            if os.path.exists(FAKE_GAME_PATH):
                os.unlink(FAKE_GAME_PATH)


class TestGameOverrides(unittest.TestCase):
    """Test overrides for specific games."""

    def test_doom(self):
        """Test Doom (2016) overrides."""
        # Make sure +r_renderAPI 1 gets added
        self.assertIn(
            "\nfoo +r_renderAPI 1\n",
            test_common.execute_launch_wrapper("echo foo", steamid=379720),
        )
        # Make sure +r_renderAPI 0 causes +r_renderAPI 1 to not get added
        self.assertIn(
            "\nfoo +r_renderAPI 0\n",
            test_common.execute_launch_wrapper(
                "echo foo +r_renderAPI 0", steamid=379720
            ),
        )


if __name__ == "__main__":
    unittest.main()
