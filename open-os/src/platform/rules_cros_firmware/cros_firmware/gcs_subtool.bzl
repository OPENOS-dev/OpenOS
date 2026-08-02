# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

_GET_PATH_SCRIPT = """#!/bin/bash
dirname "$(realpath {license_file})"
"""

def _get_path_impl(ctx):
    """Simple run rule to get the path to the root of the archive."""
    license_file_path = ctx.files.license_file[0].root.path + ctx.files.license_file[0].short_path
    script = ctx.actions.declare_file("get_path")
    script_contents = _GET_PATH_SCRIPT.format(license_file = license_file_path)
    ctx.actions.write(script, script_contents, is_executable = True)
    runfiles = ctx.runfiles(files = ctx.files.license_file)
    return [DefaultInfo(executable = script, runfiles = runfiles)]

_get_path = rule(
    implementation = _get_path_impl,
    attrs = {
        "license_file": attr.label(
            doc = "Path to license.html.zst",
            allow_single_file = True,
        ),
    },
    executable = True,
)

def setup_subtool():
    native.filegroup(
        name = "all_files",
        srcs = native.glob(["**/*"]),
        visibility = ["//visibility:public"],
    )
    _get_path(name = "get_path", license_file = ":license.html.zst")
