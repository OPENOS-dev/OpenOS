# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Tests for quirks.py"""

import unittest

import google.protobuf.text_format

from . import quirks
from . import quirks_pb2


def config_proto(text):
    """Return a Config proto parsed from `text`."""
    config = quirks_pb2.Config()
    google.protobuf.text_format.Parse(text, config)
    return config


class TestEditQuirksConfig(unittest.TestCase):
    """Tests for edit_quirks_config() in quirks.py"""

    def assertProtosEqual(self, actual, expected):
        """Helper function to print extra context for non-matching protos."""
        if actual != expected:
            print(f"Actual: {actual}\nExpected: {expected}")
        self.assertEqual(actual, expected)

    def test_reset_removes_all_game_rules(self):
        """reset=True removes all rules associated with the given game."""
        config = config_proto(
            """sommelier {
                 condition { steam_game_id: 123 }
                 enable: FEATURE_X11_MOVE_WINDOWS
               }
               sommelier {
                 condition { steam_game_id: 123 }
                 disable: FEATURE_X11_MOVE_WINDOWS
               }"""
        )
        quirks.edit_quirks_config(config, 123, None, None, reset=True)
        self.assertProtosEqual(config, quirks_pb2.Config())

    def test_reset_ignores_other_rules(self):
        """reset=True doesn't remove rules associated with other games."""
        config = config_proto(
            """sommelier {
                 condition { steam_game_id: 123 }
                 enable: FEATURE_X11_MOVE_WINDOWS
               }
               sommelier {
                 condition { steam_game_id: 456 }
                 disable: FEATURE_X11_MOVE_WINDOWS
               }"""
        )

        quirks.edit_quirks_config(config, 123, None, None, reset=True)

        self.assertProtosEqual(
            config,
            config_proto(
                """sommelier {
                     condition { steam_game_id: 456 }
                     disable: FEATURE_X11_MOVE_WINDOWS
                   }"""
            ),
        )

    def test_enable_single_game(self):
        """Can enable a feature for a single game."""
        config = quirks_pb2.Config()
        quirks.edit_quirks_config(
            config, 123456, "FEATURE_X11_MOVE_WINDOWS", None, reset=False
        )

        expected_config = quirks_pb2.Config()
        rule = expected_config.sommelier.add()
        rule.condition.add().steam_game_id = 123456
        rule.enable.append(quirks_pb2.Feature.FEATURE_X11_MOVE_WINDOWS)
        self.assertProtosEqual(config, expected_config)

    def test_disable_single_game(self):
        """Can disable a feature for a single game."""
        config = quirks_pb2.Config()
        quirks.edit_quirks_config(
            config, 123456, None, "FEATURE_X11_MOVE_WINDOWS", reset=False
        )

        self.assertProtosEqual(
            config,
            config_proto(
                """sommelier {
                     condition { steam_game_id: 123456 }
                     disable: FEATURE_X11_MOVE_WINDOWS
                   }"""
            ),
        )

    def test_enable_is_idempotent(self):
        """Enabling a feature multiple times doesn't change the config after the first time."""
        text = """sommelier {
                    condition { steam_game_id: 123456 }
                    enable: FEATURE_X11_MOVE_WINDOWS
                  }"""
        config = config_proto(text)
        expected_config = config_proto(text)

        quirks.edit_quirks_config(
            config, 123456, "FEATURE_X11_MOVE_WINDOWS", None, reset=False
        )

        self.assertProtosEqual(config, expected_config)

    def test_disable_is_idempotent(self):
        """Enabling a feature multiple times doesn't change the config after the first time."""
        text = """sommelier {
                    condition { steam_game_id: 123456 }
                    disable: FEATURE_X11_MOVE_WINDOWS
                  }"""
        config = config_proto(text)
        expected_config = config_proto(text)

        quirks.edit_quirks_config(
            config, 123456, None, "FEATURE_X11_MOVE_WINDOWS", reset=False
        )

        self.assertProtosEqual(config, expected_config)

    def test_enable_two_games(self):
        """Can enable a feature for two games."""
        config = quirks_pb2.Config()
        quirks.edit_quirks_config(
            config, 123, "FEATURE_X11_MOVE_WINDOWS", None, reset=False
        )
        quirks.edit_quirks_config(
            config, 456, "FEATURE_X11_MOVE_WINDOWS", None, reset=False
        )

        expected_config = quirks_pb2.Config()
        rule = expected_config.sommelier.add()
        rule.condition.add().steam_game_id = 123
        rule.enable.append(quirks_pb2.Feature.FEATURE_X11_MOVE_WINDOWS)
        rule2 = expected_config.sommelier.add()
        rule2.condition.add().steam_game_id = 456
        rule2.enable.append(quirks_pb2.Feature.FEATURE_X11_MOVE_WINDOWS)
        self.assertProtosEqual(config, expected_config)

    def test_enable_can_replace_disable(self):
        """Enabling a feature edits an existing rule for that game."""
        config = config_proto(
            """sommelier {
                 condition { steam_game_id: 123456 }
                 disable: FEATURE_X11_MOVE_WINDOWS
               }"""
        )

        quirks.edit_quirks_config(
            config, 123456, "FEATURE_X11_MOVE_WINDOWS", None, reset=False
        )

        expected_config = quirks_pb2.Config()
        rule = expected_config.sommelier.add()
        rule.condition.add().steam_game_id = 123456
        rule.enable.append(quirks_pb2.Feature.FEATURE_X11_MOVE_WINDOWS)
        self.assertProtosEqual(config, expected_config)

    def test_disable_can_replace_enable(self):
        """Disabling a feature also edits an existing rule for that game."""
        config = config_proto(
            """sommelier {
                 condition { steam_game_id: 123456 }
                 enable: FEATURE_X11_MOVE_WINDOWS
               }"""
        )

        quirks.edit_quirks_config(
            config, 123456, None, "FEATURE_X11_MOVE_WINDOWS", reset=False
        )

        expected_config = quirks_pb2.Config()
        rule = expected_config.sommelier.add()
        rule.condition.add().steam_game_id = 123456
        rule.disable.append(quirks_pb2.Feature.FEATURE_X11_MOVE_WINDOWS)
        self.assertProtosEqual(config, expected_config)

    def test_can_modify_existing_rule_with_existing_feature(self):
        """Enabling a feature edits an existing rule."""
        config = config_proto(
            """sommelier {
                 condition { steam_game_id: 123456 }
                 enable: FEATURE_UNSPECIFIED
               }"""
        )

        quirks.edit_quirks_config(
            config, 123456, "FEATURE_X11_MOVE_WINDOWS", None, reset=False
        )

        expected_config = quirks_pb2.Config()
        rule = expected_config.sommelier.add()
        rule.condition.add().steam_game_id = 123456
        rule.enable.append(quirks_pb2.Feature.FEATURE_UNSPECIFIED)
        rule.enable.append(quirks_pb2.Feature.FEATURE_X11_MOVE_WINDOWS)
        self.assertProtosEqual(config, expected_config)
