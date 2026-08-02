# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

load("@bazel_tools//tools/build_defs/repo:utils.bzl", "workspace_and_buildfile")

_attrs = {
    "build_file": attr.label(
        allow_single_file = True,
        doc =
            "The file to use as the BUILD file for this repository." +
            "This attribute is an absolute label (use '@//' for the main " +
            "repo). The file does not need to be named BUILD, but can " +
            "be (something like BUILD.new-repo-name may work well for " +
            "distinguishing it from the repository's actual BUILD files. " +
            "Either build_file or build_file_content must be specified.",
    ),
    "build_file_content": attr.string(
        doc =
            "The content for the BUILD file for this repository. " +
            "Either build_file or build_file_content must be specified.",
    ),
    "workspace_file": attr.label(
        doc =
            "The file to use as the `WORKSPACE` file for this repository. " +
            "Either `workspace_file` or `workspace_file_content` can be " +
            "specified, or neither, but not both.",
    ),
    "workspace_file_content": attr.string(
        doc =
            "The content for the WORKSPACE file for this repository. " +
            "Either `workspace_file` or `workspace_file_content` can be " +
            "specified, or neither, but not both.",
    ),
}

def _find_depot_tools(ctx):
    paths = ctx.os.environ.get("PATH", "").split(":")
    for path in paths:
        if path.endswith("depot_tools"):
            return path

    fail("Cannot find depot_tools (is it in your PATH?)")

def _depot_tools_impl(ctx):
    location = _find_depot_tools(ctx)
    ctx.symlink(location, ctx.path(".").get_child(ctx.attr.name))
    workspace_and_buildfile(ctx)
    return None

depot_tools = repository_rule(
    implementation = _depot_tools_impl,
    local = True,
    configure = True,
    environ = ["PATH"],
    attrs = _attrs,
    doc = "Find depot_tools and expose as a repository",
)
