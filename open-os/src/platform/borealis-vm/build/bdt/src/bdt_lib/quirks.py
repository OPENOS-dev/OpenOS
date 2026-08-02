# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Implements `bdt quirk`, a utility for editing/validating the user's quirks config."""

import os.path
import sys
import textwrap

import google.protobuf.text_format  # pylint: disable=import-error

from . import quirks_pb2  # pylint: disable=no-name-in-module


USER_QUIRKS_CONFIG = "~/.config/borealis.textproto"
SHIPPED_QUIRKS_CONFIG = "/etc/quirks/default.textproto"


def quirk_rule_error(rule):
    """Check `rule` for errors.

    Returns:
        A string describing a problem with `rule`, or None if the rule passed validation.
    """
    # These checks make sense for v1, but will need relaxing
    # once we add more types of condition.
    if len(rule.condition) != 1:
        return "Must have exactly one condition"
    cond = rule.condition[0]
    if not cond.steam_game_id and not cond.always:
        return (
            "Condition must be always active or specify a nonzero steam_game_id"
        )

    # This check will need relaxing once we add more types of effect
    # other than simplying enabling/disabling a feature flag.
    if not rule.enable and not rule.disable:
        return "Rule must enable/disable at least one feature"

    # Can't both enable and disable a feature flag.
    enabled = set(rule.enable)
    disabled = set(rule.disable)
    if not enabled.isdisjoint(disabled):
        enabled.intersection_update(disabled)
        features = ", ".join(quirks_pb2.Feature.Name(f) for f in enabled)
        return f"A feature must either be enabled or disabled, not both ({features})"

    # Rule is valid.
    return None


def edit_quirks_config(config, game_id, enable, disable, reset):
    """Modify quirks rules for the given game.

    Exactly one of enable, disable, and reset must be "truthy".
    Each represents a mutually exclusive operation.

    Args:
        config: The current quirks config proto.
        game_id: Steam App ID for the game to apply quirks rules to.
        enable: Name of a feature to enable for that game.
        disable: Name of a feature to disable for that game.
        reset: If true, remove all custom config for that game.
    """
    args = (1 if enable else 0) + (1 if disable else 0) + (1 if reset else 0)
    if args != 1:
        raise ValueError("Specify exactly one of enable, disable, and reset.")

    # Filter out invalid rules, and rules already set for this game.
    sommelier_rules = []
    this_game_rules = []
    for rule in config.sommelier:
        err = quirk_rule_error(rule)
        if err:
            # Invalid rules are not kept.
            print(
                f"Warning: Removing rule `{rule}` because it failed validation: {err}"
            )
        elif rule.condition[0].steam_game_id == game_id:
            # Modify this later.
            this_game_rules.append(rule)
        else:
            # Rule is for a different case, leave it unchanged.
            sommelier_rules.append(rule)

    if not reset:
        if not this_game_rules:
            # No existing rule, so create one.
            rule = quirks_pb2.SommelierRule()
            this_game_rules.append(rule)

            condition = rule.condition.add()
            condition.steam_game_id = game_id

        # Remove all references to the feature being edited from the game's rule.
        # If the game has multiple rules, edit the last one.
        feature = quirks_pb2.Feature.Value(enable if enable else disable)

        try:
            this_game_rules[-1].enable.remove(feature)
        except ValueError:
            pass
        try:
            this_game_rules[-1].disable.remove(feature)
        except ValueError:
            pass

        if enable:
            this_game_rules[-1].enable.append(feature)
        if disable:
            this_game_rules[-1].disable.append(feature)

        sommelier_rules.extend(this_game_rules)

    # Sort the rules and replace them in the config.
    sommelier_rules.sort(key=lambda rule: rule.condition[0].steam_game_id)
    # `config.sommelier[:] = ...` is documented to work, but doesn't seem to.
    del config.sommelier[:]
    config.sommelier.extend(sommelier_rules)


def load_quirks(filename):
    """Parse a Config proto from the file at `filename`."""
    config = quirks_pb2.Config()
    # Note: This won't behave as expected for regular filenames starting with a tilde.
    # This is a corner case that didn't seem worth supporting.
    with open(os.path.expanduser(filename), "r", encoding="utf-8") as f:
        try:
            google.protobuf.text_format.Parse(f.read(), config)
        except google.protobuf.text_format.ParseError as e:
            print(e)
            # The shipped config can't be removed, so don't suggest that.
            if filename != SHIPPED_QUIRKS_CONFIG:
                print(
                    "\nThe config file appears to be invalid. "
                    "If you want to remove it and start fresh, you could run:\n"
                    f"  rm {filename}"
                )
            sys.exit(1)

    return config


def quirk_command(game, enable, disable, reset):
    """Handler for `bdt quirk` subcommand."""
    try:
        config = load_quirks(USER_QUIRKS_CONFIG)
    except FileNotFoundError:
        # Missing file = empty config.
        config = quirks_pb2.Config()

    edit_quirks_config(config, game, enable, disable, reset)
    with open(
        os.path.expanduser(USER_QUIRKS_CONFIG), "w", encoding="utf-8"
    ) as f:
        f.write(
            google.protobuf.text_format.MessageToString(config, as_utf8=True)
        )

    if reset:
        print(f"Custom rules affecting game {game} removed.")
    if enable:
        print(f"Feature {enable} enabled for game {game}.")
    if disable:
        print(f"Feature {disable} disabled for game {game}.")
    print("\nTip: To check the game ID is correct, visit:")
    print(f"     https://store.steampowered.com/app/{game}")
    print(
        "\nNote: To apply your change, open crosh and type: vmc stop borealis"
    )
    print("      This will forcibly close Steam and all running games.")
    print("      Alternatively, restart your Chromebook.\n")


def validate_quirks_command(config, print_config):
    """Handler for `bdt validate-quirks` subcommand."""
    try:
        proto = load_quirks(config)
    except FileNotFoundError:
        print(f"\nError: Quirks config {config} not found.")
        sys.exit(1)

    if print_config:
        print(google.protobuf.text_format.MessageToString(proto, as_utf8=True))

    error_count = 0
    for rule in proto.sommelier:
        err = quirk_rule_error(rule)
        if err:
            r = textwrap.indent(str(rule), "    ")
            print(f"\nError: {err}:\n{r}")
            error_count += 1

    if error_count:
        print(f"Validating {config} failed with {error_count} errors.")
        sys.exit(1)

    print(f"Success: {config} passed validation.")
