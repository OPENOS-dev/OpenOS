#!/usr/bin/env python3
# Copyright 2021 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""exerun automates the invocation of Windows executables under Borealis

   The script expects the final script parameter(s) to be the EXE path
   and its arguments.

   This script makes references to a "Wine prefix".
   Description of a Wine prefix:

   It's probably best to think of WINE prefixes sort of like virtual machines.
   They're not virtual machines, but they do behave somewhat similarly.
   A WINE prefix is a folder that contains all of the WINE configurations as well as
   all of the Windows pieces that WINE uses for compatibility, including libraries
   and a registry.
   The default WINE prefix is ~/.wine, but different and multiple prefixes can be used.

   The above paragraph is quoted from:
   https://linuxconfig.org/using-wine-prefixes
"""

import argparse
import os
import shutil
import subprocess

import vdf  # pylint: disable=import-error


def replace_token(token, input_array, replacement):
    """Replaces all occurences of 'token' with 'replacement'"""
    return [x.replace(token, replacement) for x in input_array]


def get_tool_manifest(path):
    """Opens and parses a toolmanifest.vdf file

    This function will also unravel the first layer ('manifest')
    and return the resulting dictionary
    """
    toolmanifest = os.path.join(path, "toolmanifest.vdf")
    manifest_dictionary = vdf.load(open(toolmanifest, encoding="utf-8"))

    return manifest_dictionary["manifest"]


def get_command_from_manifest(manifest_path):
    """Returns a command list suitable for subprocess.run from a manifest dict

    This function extracts the 'commandline' parameter from a toolmanifest.vdf file and
    produces an array that is suitable for direct use with subprocess.run()

    Args:
        manifest_path: The full path of directory that contains the toolmanifest.vdf file
    """
    toolmanifest = get_tool_manifest(manifest_path)

    manifest_cmd = toolmanifest["commandline"].split(" ")
    manifest_cmd[0] = manifest_path + manifest_cmd[0]
    return manifest_cmd


class ExeRunner:
    """The main exe runner class"""

    def __init__(self, slr, proton, prefix, exe, debug):
        self.slr_path = os.path.abspath(slr)
        self.proton_path = os.path.abspath(proton)
        self.prefix_path = os.path.abspath(prefix)
        self.exe_path = os.path.dirname(os.path.abspath(exe))
        self.proton_cmd = None
        self.slr_cmd = None
        self.mount_paths = []
        self.debug = debug

    def _add_compat_mount(self, path):
        """Adds a path to be mounted by the pressure_vessel sandbox"""
        home_path = os.path.expanduser("~")

        # The home directory of the user is already mapped into the container.
        # Paths relative to it do not need to be mapped.
        if home_path not in path:
            self.mount_paths.append(path)

    def _parse_slr_vdf(self):
        self.slr_cmd = get_command_from_manifest(self.slr_path)

    def _parse_proton_vdf(self):
        self.proton_cmd = get_command_from_manifest(self.proton_path)

    def _prepare_env(self):
        env = os.environ.copy()

        # Ensure paths necessary for command execution are available within the
        # pressure-vessel sandbox.
        self._add_compat_mount(self.slr_path)
        self._add_compat_mount(self.proton_path)
        self._add_compat_mount(self.prefix_path)
        self._add_compat_mount(self.exe_path)
        env["STEAM_COMPAT_MOUNTS"] = ":".join(self.mount_paths)

        # Configure Proton and subcomponents
        env["STEAM_COMPAT_DATA_PATH"] = self.prefix_path
        env["DXVK_STATE_CACHE_PATH"] = self.prefix_path
        env["STEAM_COMPAT_CLIENT_INSTALL_PATH"] = ""

        if self.debug:
            env["SteamGameId"] = "0"
            env["PROTON_LOG"] = "1"

        return env

    def run_exe(self, cmd):
        """The main EXE run method

        This method invokes the EXE using the parameters provided in the
        class constructor
        """
        self._parse_proton_vdf()
        self._parse_slr_vdf()

        if not os.path.isdir(self.prefix_path):
            print("Making prefix at: " + self.prefix_path)
            os.makedirs(self.prefix_path)

        full_proton_cmd = replace_token(
            "%verb%", self.proton_cmd, "waitforexitandrun"
        )
        full_proton_cmd.extend(cmd)

        full_cmd = replace_token("%verb%", self.slr_cmd, "waitforexitandrun")
        full_cmd.extend(full_proton_cmd)

        # One can add Shell=True here to more closely match the environment
        # that Proton runs under. However, this does not work correctly
        # as the individual parts of the command needs to be quoted.
        #
        # Some additional work will need to be done on the command line
        # in order to support this method of invocation.
        #
        # This comment is left as a debugging aid in case issues arise from
        # this discrepancy in the future.
        subprocess.run(args=full_cmd, env=self._prepare_env(), check=True)

    def purge(self):
        """Deletes the wine prefix directory"""

        print("Purging wine prefix at " + self.prefix_path)
        if os.path.exists(self.prefix_path):
            shutil.rmtree(path=self.prefix_path)


def main():
    """Main entry point for the script"""
    parser = argparse.ArgumentParser(description=__doc__)

    default_prefix = os.path.expanduser("~/.exerun/prefix")

    parser.add_argument(
        "--proton", help="The path to the proton package to use", required=True
    )
    parser.add_argument(
        "--slr", help="The path to the SLR package to use", required=True
    )
    parser.add_argument(
        "--wine_prefix",
        help="The path to the directory to use for a wine prefix (optional)",
        default=default_prefix,
    )
    parser.add_argument(
        "--run", action="store_true", help=" [verb] Runs an EXE (default)"
    )
    parser.add_argument(
        "--purge",
        action="store_true",
        help="[verb] Deletes the given prefix directory",
    )
    parser.add_argument(
        "--debug",
        action="store_true",
        help="[verb] Writes debug logs to ~/steam-0.log",
    )
    parser.add_argument(
        "cmd", nargs="+", help="The EXE and args to execute through Proton"
    )

    args = parser.parse_args()

    exerunner = ExeRunner(
        slr=os.path.expanduser(args.slr),
        proton=os.path.expanduser(args.proton),
        prefix=os.path.expanduser(args.wine_prefix),
        exe=os.path.expanduser(args.cmd[0]),
        debug=args.debug,
    )

    if args.purge:
        exerunner.purge()

    exerunner.run_exe(args.cmd)


if __name__ == "__main__":
    main()
