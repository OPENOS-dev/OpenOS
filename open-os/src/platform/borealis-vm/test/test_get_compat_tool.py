# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Tests for /usr/bin/get_compat_tool.py."""

import os
import subprocess
import tempfile
import unittest


EXAMPLE_CONFIG_VDF = """
"Layer"
{
	"Layer2"
	{
		"Layer3"
		{
			"Layer4"
			{
				"CompatToolMapping"
				{
					"413150"
					{
						"name"		"proton_8"
						"config"		""
						"priority"		"250"
					}
					"1145360"
					{
						"name"		"proton_9"
						"config"		""
						"priority"		"250"
					}
					"2379780"
					{
						"name"		"steamlinuxruntime"
						"config"		""
						"priority"		"250"
					}
					"227300"
					{
						"name"		"proton-stable"
						"config"		""
						"priority"		"250"
					}
				}
			}
		}
	}
}
"""


def _run_get_compat_tool(game_id: int, config_path: str) -> str:
    return (
        subprocess.run(
            [
                "/usr/bin/get_compat_tool.py",
                "--game-id",
                str(game_id),
                "--config-path",
                config_path,
            ],
            check=False,
            capture_output=True,
        )
        .stdout.decode()
        .strip()
    )


class TestGetCompatTool(unittest.TestCase):
    """Test get_compat_tool.py"""

    def test_empty_file(self):
        """Test parsing empty file"""
        with tempfile.NamedTemporaryFile(mode="w") as f:
            f.write("")
            f.flush()
            self.assertEqual(_run_get_compat_tool(0, f.name), "")

    def test_invalid_game(self):
        """Test getting compat mapping for a game that is not present"""
        with tempfile.NamedTemporaryFile(mode="w") as f:
            f.write(EXAMPLE_CONFIG_VDF)
            f.flush()
            self.assertEqual(_run_get_compat_tool(620, f.name), "")

    def test_valid_game(self):
        """Test getting compat mapping for a game that are present"""
        with tempfile.NamedTemporaryFile(mode="w") as f:
            f.write(EXAMPLE_CONFIG_VDF)
            f.flush()
            self.assertEqual(_run_get_compat_tool(413150, f.name), "proton_8")
            self.assertEqual(_run_get_compat_tool(1145360, f.name), "proton_9")
            self.assertEqual(
                _run_get_compat_tool(2379780, f.name), "steamlinuxruntime"
            )
            self.assertEqual(
                _run_get_compat_tool(227300, f.name), "proton-stable"
            )
