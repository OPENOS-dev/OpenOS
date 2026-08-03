#!/usr/bin/env python3
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""A wrapper script to be used to capture Windows API traces

   The script expects the final script parameter to be the
   command that invokes the Windows executable

   ApiTrace (d3d8 - d3d11, OpenGL):
   The script will place the generated trace file(s) in the
   users home directory with the name <sourceExe>_<dateTimeCode>.trace
   The TRACE_FILE environment variable will be used to specify the
   target trace file if the app id cannot be extracted from the input
   command. This has the downside of allowing trace file collisions
   when multiple processes initialize the graphics API. For this reason,
   this method of specifying the output trace file is used as a fallback.

   The script offers two approaches for capturing a trace file, each with
   their own pitfalls/advantages. These are detailed within the script,
   see the comments on GfxTraceRunner class methods for more details.

   As for the pitfalls common to both approaches:
   1) 32-bit/64-bit: If a launcher or wrapper executable is of a different type
   than the target game executable, then tracing will fail. Wine cannot load a 32-bit
   DLL into a 64-bit process and vice versa (this is also true for Windows proper).

   GfxReconstruct (d3d12):
   For D3D12 traces using gfxreconstruct, the wrapper method is the only one available
   and only 64-bit executables can be traced.
"""

import argparse
import os
import pathlib
import shutil
import subprocess
import sys
import time

import pefile  # pylint: disable=import-error


def convert_to_windows_path(path):
    """Converts a path to one that can be used by Windows executables"""
    return "Z:" + path.replace("/", "\\")


def get_default_apitrace_path():
    """Returns the default location for the Windows apitrace package"""
    return "/opt/win_tools/apitrace"


def get_default_gfxreconstruct_path():
    """Returns the default location for the Windows gfxreconstruct package"""
    return "/opt/win_tools/gfxreconstruct"


class GfxTraceRunner:
    """The main GfxTracer runner class

    This class extracts from the command line parameters
    all useful information necessary to create a trace file
    name. It also has logic to use the correct DLLs/EXEs to
    match with the game binary.
    """

    def __init__(self, launch_command, package, api):
        self.full_exe_path = None
        self.full_exe_directory = None
        self.exe_prefix = None
        self.exe_name = None
        self.full_trace_path = None
        self.date_code = None
        self.full_command = launch_command

        # We have a different set of Wine DLL overrides and DLLs to wrap list
        # depending on which tool we are using.
        #
        # The code below will document them. They will then be assigned later on
        # to a common variable so common code paths can be employed.
        gfxreconstruct_dlls_to_wrap = [
            "d3d12.dll",
            "d3d12_capture.dll",
            "dxgi.dll",
        ]
        apitrace_dlls_to_wrap = [
            "d3d9.dll",
            "d3d10.dll",
            "d3d11.dll",
            "dxgi.dll",
            "dxgitrace.dll",
            "opengl32.dll",
        ]

        apitrace_wine_dll_overrides = (
            "d3d9=n,b;d3d10=n,b;d3d11=n,b;"
            "dxgi=n,b;dxgitrace=n,b;opengl32=n,b"
        )
        gfxreconstruct_wine_dll_overrides = (
            "d3d12=n,b;d3d12_capture=n,b;dxgi=n,b"
        )

        self.package_path = package
        self.compat_path = None
        self.exe_type = None
        self.app_id = None
        self.api = api
        self.mount_paths = []

        if api == "d3d12":
            self.dlls_to_wrap = gfxreconstruct_dlls_to_wrap
            self.wine_dll_overrides = gfxreconstruct_wine_dll_overrides
        else:
            self.dlls_to_wrap = apitrace_dlls_to_wrap
            self.wine_dll_overrides = apitrace_wine_dll_overrides

    def _add_compat_mount(self, path):
        home_path = pathlib.Path(os.path.expanduser("~"))
        compat_path = pathlib.Path(path)

        # The home directory of the user is already mapped into the container
        # This is to avoid adding it again
        if not compat_path.is_relative_to(
            home_path
        ):  # pylint: disable=no-member
            self.mount_paths.append(path)

    def _get_app_id(self):
        """Attempt to retrieve the AppId from the executable command

        This function retrieves the AppId from the executable command.
        It will scan for the format 'AppId=<appId>' and extract the value
        of <appId>.

        If this element is not found within the command line None will be
        returned.
        """
        for cmd in self.full_command:
            if "AppId" in cmd:
                app_id_split = cmd.split("=")
                self.app_id = app_id_split[1]
                break
        else:
            self.app_id = None

    def _get_exe_part(self):
        for cmd in self.full_command:
            if cmd.endswith("exe"):
                return cmd
        raise FileNotFoundError

    def _get_exe_type(self):
        # fast_load should be set to True, otherwise the pefile
        # library will spend a long time loading parts that are
        # not relevant for our purpose
        peobj = pefile.PE(name=self.full_exe_path, fast_load=True)
        exe_type = pefile.MACHINE_TYPE[peobj.FILE_HEADER.Machine]

        # Current version of pefile will return
        # 'IMAGE_FILE_MACHINE_AMD64' for x86_64 binary
        # 'IMAGE_FILE_MACHINE_I386' for base x86 binary
        if exe_type == "IMAGE_FILE_MACHINE_AMD64":
            self.exe_type = "x86_64"
        elif exe_type == "IMAGE_FILE_MACHINE_I386":
            self.exe_type = "i386"
        else:
            self.exe_type = "Unknown"

    def _generate_trace_path(self):
        trace_name = self.exe_prefix + "_" + self.date_code + ".trace"
        self.full_trace_path = os.path.join(os.path.expanduser("~"), trace_name)

    def _copy_dlls(self, pre_exec, dll_source_path):
        """Copies the wrapper DLL's into the same directory as the target EXE

        This function will copy the listed wrapper DLL's into the
        same directory as the target EXE file.
        This, combined with the WINEDLLOVERRIDES environment variable will
        have these DLL's loaded when the game loads them implicitly through
        import or explicitly through LoadLibrary.

        Args:
            pre_exec: True = DLLs->EXE directory False =  Undo ops in 'True'
            dll_source_path: The source path of the wrapper DLL's.
        """
        for dllname in self.dlls_to_wrap:
            dest = os.path.join(self.full_exe_directory, dllname)
            source = os.path.join(dll_source_path, dllname)

            if pre_exec:
                # Copy over to the EXE directory
                if os.path.exists(dest):
                    shutil.move(dest, dest + self.date_code)

                shutil.copy(src=source, dst=dest)
            else:
                # Delete the DLLs that have been copied over previously
                os.remove(dest)

                if os.path.exists(dest + self.date_code):
                    shutil.move(dest + self.date_code, dest)

    def _get_compat_data_path(self):
        # We want to go and find the steamapps directory from the full path to the
        # game's executable. From there, we can figure out where the game's compatdata
        # directory is.

        if self.app_id is None:
            return None

        compat_path_obj = pathlib.Path(self.full_exe_directory)

        try:
            index = compat_path_obj.parts.index("steamapps")
        except ValueError:
            # If we cannot find steamapps within the decomposed path
            # then we cannot retrieve a compatdata path
            print("steamapps not found in executable path!")
            return None

        return os.path.join(
            *compat_path_obj.parts[0 : index + 1], "compatdata", self.app_id
        )

    def _prepare_pkgpaths(self):
        # We are expecting a full path here
        self._add_compat_mount(self.package_path)

    def _prepare_for_trace(self):
        # Setup critical variables necessary for trace capture
        self.full_exe_path = "/" + self._get_exe_part()
        self.full_exe_directory = os.path.dirname(self.full_exe_path)
        self.exe_name = os.path.basename(self.full_exe_path)
        self.exe_prefix = self.exe_name.replace(".exe", "")
        self.date_code = time.strftime("%Y%m%d%H%M%S")

        self._get_app_id()
        self._get_exe_type()
        self._prepare_pkgpaths()
        self.compat_path = self._get_compat_data_path()

    def _prepare_env(self):
        runenv = os.environ.copy()
        runenv["WINEDLLOVERRIDES"] = '"' + self.wine_dll_overrides + '"'
        runenv["STEAM_COMPAT_MOUNTS"] = ":".join(self.mount_paths)
        return runenv

    # This method implements the wrapper approach of trace capture
    # This method is shared between the GfxReconstruct and ApiTrace paths
    #
    # This approach involves using the incoming command line to:
    # a) Identify the location of the EXE that is invoked by Steam
    # b) Determine the binary type (32-bit or 64-bit)
    # c) Copy over the appropriate wrapper DLLs to the same directory as the EXE.
    #    This is to ensure that the wrapper DLLs are loaded when the game
    #    attempts to use Direct3D or OpenGL for rendering.
    # d) Set the WINEDLLOVERRIDES variable to override the DLLs with
    #    the wrapper DLLs. Without this, Wine will ignore the wrapper DLLs.
    # e) Then run the command as is, unmodified.
    #
    # The unique downside here is that if the game is using a launcher/wrapper executable
    # that is in a separate directory from the actual game executable then this approach
    # will fail.
    def _run_wrapper_method(self, trace_env, dll_source_path):
        self._copy_dlls(pre_exec=True, dll_source_path=dll_source_path)
        # In the wrapper case, we still need to clean up the DLL's
        # we copied if 'run' raises an exception
        try:
            subprocess.run(args=self.full_command, env=trace_env, check=True)
        finally:
            self._copy_dlls(pre_exec=False, dll_source_path=dll_source_path)

    # This method implements the apitrace command approach
    #
    # In this approach, the incoming command line is modified so that apitrace.exe
    # is used to indirectly run the game and perform any setup necessary
    # to capture the trace. The modified command line will then be executed.
    # This has the advantage of working correctly if the launcher/wrapper executable is
    # in a different directory. Apitrace will follow child processes that have been forked
    # from the original.
    #
    # The user MUST specify the correct API that the application is using. If the user
    # gets this wrong, no trace file will be output.
    def _run_trace_cmd_method(self, trace_env):
        # This code is simply locating the appropriate place
        # to insert the apitrace.exe command.
        # This will be immediately preceding the game executable
        mod_command = self.full_command

        cmd = self._get_exe_part()

        trace_exe = os.path.join(
            self.package_path, self.exe_type, "bin", "apitrace.exe"
        )
        trace_cmds = [trace_exe, "trace", "-a", self.api]

        exe_index = mod_command.index(cmd)
        mod_command[exe_index:exe_index] = trace_cmds
        subprocess.run(args=mod_command, env=trace_env, check=True)

    def _relocate_trace_files(
        self, trace_location, trace_glob_filter, append_time_code
    ):
        """Relocates the generated trace files to the Linux users home directory

        This method will use a glob operator to locate all the trace files
        within the specified trace_location directory.

        It will then move said files to the Linux users home directory
        with a date time code attached, if requested.

        Args:
            trace_location: The path to look for the resulting traces
            trace_glob_filter: The filter text to use to locate the resulting trace files
            append_time_code: Used to indicate if appending a time code is desired
        """

        # Perform the globbing to locate all the trace files
        desktop_path_obj = pathlib.Path(trace_location)
        pathlist = list(desktop_path_obj.glob(trace_glob_filter))

        for mypath in pathlist:
            if append_time_code:
                target_name = mypath.stem
                target_suffix = mypath.suffix
                target_name = target_name + "_" + self.date_code + target_suffix
            else:
                target_name = mypath.name

            destpath = pathlib.Path(
                os.path.join(os.path.expanduser("~"), target_name)
            )

            shutil.move(src=mypath.as_posix(), dst=destpath.as_posix())

    def run_gfxreconstruct_trace(self):
        """Top level gfxreconstruct trace execution function

        This is the main entry point of the GfxTraceRunner script
        for a gfxreconstruct trace. It will perform all the steps
        necessary to capture a gfxreconstruct trace.

        This path is only supported for D3D12. In addition, the binaries
        only support tracing 64-bit executables.
        """
        self._prepare_for_trace()
        trace_env = self._prepare_env()

        # Add specific GfxReconstruct options to enable them to work under Proton/Wine
        # Without these options the games will crash fairly shortly after startup.
        #
        # GFXRECON_PAGE_GUARD_EXTERNAL_MEMORY defaults to true. The problem with this
        # under Proton/Wine is that it requires kernel level support to track
        # memory writes to specific areas of memory. It is not implemented
        # under Wine and will not work correctly.
        trace_env["GFXRECON_PAGE_GUARD_EXTERNAL_MEMORY"] = "false"

        # Some titles could end up placing the trace in a directory that
        # does not house the executable. To ensure that the trace is placed
        # in an appropriate place, force the location of the trace
        # to be the users home directory.
        #
        # GFXRECON_CAPTURE_FILE_TIMESTAMP by default is true, so we do not
        # need to worry about creating a unique file name. See GfxRecon's
        # D3D12 README for details.
        trace_env["GFXRECON_CAPTURE_FILE"] = os.path.join(
            os.path.expanduser("~"), "gfxrecon_capture.gfxr"
        )

        # The exact same approach as the apitrace wrapper method is taken here.
        dll_source_path = os.path.join(self.package_path, "d3d12_capture")
        print("Using gfxreconstruct for d3d12 capture")
        self._run_wrapper_method(trace_env, dll_source_path)

    def run_apitrace(self):
        """Top level trace execution function

        This is the main entry point of the GfxTraceRunner script.
        It is invoked with no additional arguments. It will go
        and execute the commands necessary to capture the trace.
        """

        # An attempt will be made to parse the executable command and locate
        # the AppId=<x> portion. If it is found, it will then be used
        # to get the location of the games compatdata directory. If neither steps are
        # successful, the TRACE_FILE environment variable will be used as a
        # fallback. See the script docstring for details.
        #
        # Without the TRACE_FILE environment variable being set, the Windows build of
        # apitrace will place the trace file in the Windows desktop. These files will be
        # moved to the Linux user's home directory with the name modified to include a
        # date and time code.

        wrapper_method = not bool(self.api)
        self._prepare_for_trace()
        trace_env = self._prepare_env()

        if self.compat_path is None:
            print("compat_path is missing, falling back to using TRACE_FILE")
            self._generate_trace_path()
            trace_env["TRACE_FILE"] = convert_to_windows_path(
                self.full_trace_path
            )

        if wrapper_method:
            dll_source_path = os.path.join(
                self.package_path, self.exe_type, "lib", "wrappers"
            )
            print("Using apitrace wrapper method")
            self._run_wrapper_method(trace_env, dll_source_path)
        else:
            print("Using apitrace.exe method")
            self._run_trace_cmd_method(trace_env)

        # Relocate the trace file(s) if there is a valid compat_path
        if self.compat_path is not None:
            self._relocate_trace_files(
                trace_location=os.path.join(
                    self.compat_path,
                    "pfx",
                    "drive_c",
                    "users",
                    "steamuser",
                    "Desktop",
                ),
                trace_glob_filter="*.trace",
                append_time_code=True,
            )


def main():
    """Main entry point function of the script"""
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument(
        "--api",
        help="Explicitly state the API to capture for (gl, d3d8, d3d9, etc.)",
    )
    parser.add_argument(
        "--package",
        help="Specify another location for the Windows apitrace/gfxreconstruct package",
    )

    known_args, unknown_args = parser.parse_known_args()

    if not unknown_args:
        parser.print_help()
        sys.exit()

    # D3D12 uses GfxReconstruct for capture
    # All other Gfx API's use apitrace
    if not known_args.package:
        if known_args.api == "d3d12":
            package_path = get_default_gfxreconstruct_path()
        else:
            package_path = get_default_apitrace_path()
    else:
        package_path = known_args.package_path

    tracer = GfxTraceRunner(unknown_args, package_path, known_args.api)

    # For D3D12, we are going to make use of GFXReconstruct.
    # It is a requirement to explicitly specify d3d12 as the api
    # in order to make use of this path.
    if known_args.api == "d3d12":
        tracer.run_gfxreconstruct_trace()
    else:
        tracer.run_apitrace()


if __name__ == "__main__":
    main()
