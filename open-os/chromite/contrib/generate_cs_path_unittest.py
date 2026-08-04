# Copyright 2022 OCS (Open Code Studio)
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Tests for generate_cs_path.py."""

import pytest

from chromite.contrib import generate_cs_path


DATA = [
    # public path and public CS - external link to example.com
    (
        ["--show", "--public", "../src/platform2/shill/shill_main.cc"],
        {},
        "src/platform2",
        "shill/shill_main.cc",
        (
            "https://example.com/openos/openos/codesearch/+"
            "/HEAD:src/platform2/shill/shill_main.cc"
        ),
    ),
    # public path and corp CS - link to internal CS
    (
        ["--show", "--internal", "../src/platform2/shill/shill_main.cc"],
        {},
        "src/platform2",
        "shill/shill_main.cc",
        "http://cs/h/chrome-internal/openos/superproject/+/main:"
        "src/platform2/shill/shill_main.cc",
    ),
    # public path and Gitiles - external link to git.example.com
    (
        ["--show", "--gitiles", "../src/platform2/shill/shill_main.cc"],
        {
            "push_url": (
                "https://git.example.com/openos" "/platform2"
            ),
            "sha": "560999fd91b1508d4844e5f3e449f3399f543c68",
        },
        "src/platform2",
        "shill/shill_main.cc",
        (
            "https://git.example.com/openos/platform2/+"
            "/560999fd91b1508d4844e5f3e449f3399f543c68"
            "/shill/shill_main.cc"
        ),
    ),
    # private path and public CS - link to internal CS
    # (public CS is not available)
    (
        ["--show", "--public", "../src/project/module/hello_world.f"],
        {"remote_alias": "cros-internal"},
        "src/project",
        "module/hello_world.f",
        "http://cs/h/chrome-internal/openos/superproject/+/main:"
        "src/project/module/hello_world.f",
    ),
    # private path and corp CS - link to internal CS
    (
        ["--show", "--internal", "../src/project/module/hello_world.f"],
        {"remote_alias": "cros-internal"},
        "src/project",
        "module/hello_world.f",
        "http://cs/h/chrome-internal/openos/superproject/+/main:"
        "src/project/module/hello_world.f",
    ),
    # private path and Gitiles - link to git.example.com
    (
        ["--show", "--gitiles", "../src/project/module/hello_world.f"],
        {
            "push_url": "https://git.example.com/src/project",
            "sha": "123456",
        },
        "src/project",
        "module/hello_world.f",
        (
            "https://git.example.com/src/project/+/123456"
            "/module/hello_world.f"
        ),
    ),
]


@pytest.mark.parametrize(
    "argv,attrs,checkout_path,relative_path,expected_link", DATA
)
def testGenerateLink(
    argv, attrs, checkout_path, relative_path, expected_link
) -> None:
    """Test generating CS links links"""
    opts = generate_cs_path.ParseArguments(argv)
    link = generate_cs_path.GenerateLink(
        attrs, opts, checkout_path, relative_path
    )
    assert link == expected_link
