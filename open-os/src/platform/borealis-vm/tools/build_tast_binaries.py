#!/usr/bin/env python3
# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Build the tarball used for borealis tast testing.

Uses docker and some of the other tools to construct the borealis tast testing
tarball.
"""

import argparse
import datetime
import os
import re
import subprocess
import tempfile

import build_full
import chroot
import convert_docker_image


ZLIB_VERSION = "1.2.13"
ZLIB_CHECKSUM = (
    "b3a24de97a8fdbc835b9833169501030b8977031bcb54b3b3ac13740f846ab30"
)
PIGLIT_COMMIT = "c2b31333926a6171c3c02d182b756efad7770410"


def query_package(ch: chroot.Chroot, package: str) -> str:
    """Query the package ebuild path and returns the relative path from chromeos."""
    # ex.:
    # full_path = /mnt/host/source/path/to/ebuild
    # full_path[17:] = path/to/ebuild
    query_cmd = ch.gen_cros_sdk_command(
        ["--", "equery-borealis", "which", package]
    )
    full_path = subprocess.check_output(query_cmd, text=True).strip()
    # Chop the first 17 characters which correspoding to /mnt/host/source/.
    return full_path[17:]


def gather_build_config(ch: chroot.Chroot):
    """Gather various build related versions."""

    def re_find_one(regexp, content):
        s = re.findall(regexp, content)
        if len(s) != 1:
            raise Exception(f"Expected 1 match for {regexp}, got {len(s)}")
        return s[0]

    borealis_dir = build_full.get_borealis_dir()

    deqp_runner_path = query_package(ch, "media-gfx/deqp-runner")
    deqp_runner_version = re_find_one(r"\d+\.\d+\.\d+", deqp_runner_path)

    deqp_path = query_package(ch, "media-gfx/deqp")
    print("Query ebuild:", deqp_path)
    with open(
        os.path.join(borealis_dir, "../../../", deqp_path),
        "r",
        encoding="utf-8",
    ) as f:
        deqp_content = f.read()
    deqp_commit = re_find_one(r"MY_DEQP_COMMIT=\'([\w\s]+)\'", deqp_content)
    amber_commit = re_find_one(r"MY_AMBER_COMMIT=\'([\w\s]+)\'", deqp_content)
    glslang_commit = re_find_one(
        r"MY_GLSLANG_COMMIT=\'([\w\s]+)\'", deqp_content
    )
    spirv_tools_commit = re_find_one(
        r"MY_SPIRV_TOOLS_COMMIT=\'([\w\s]+)\'", deqp_content
    )
    spirv_headers_commit = re_find_one(
        r"MY_SPIRV_HEADERS_COMMIT=\'([\w\s]+)\'", deqp_content
    )
    nvidia_video_samples_commit = re_find_one(
        r"MY_NVIDIA_VIDEO_SAMPLES_COMMIT=\'([\w\s]+)\'", deqp_content
    )

    return {
        "VERSION_deqp_runner": deqp_runner_version,
        "COMMIT_deqp": deqp_commit,
        "COMMIT_amber": amber_commit,
        "COMMIT_glslang": glslang_commit,
        "COMMIT_spirv_tools": spirv_tools_commit,
        "COMMIT_spirv_headers": spirv_headers_commit,
        "COMMIT_nvidia_video_samples": nvidia_video_samples_commit,
        "ZLIB_VERSION": ZLIB_VERSION,
        "ZLIB_CHECKSUM": ZLIB_CHECKSUM,
        "PIGLIT_COMMIT": PIGLIT_COMMIT,
    }


def create_zst_tarball(docker_image_name, output, root):
    """Create a tarball from a docker image directory."""
    with convert_docker_image.get_container_for_export(
        docker_image_name
    ) as container_id:
        with tempfile.TemporaryDirectory() as tempdir:
            convert_docker_image.extract_container_to_dir(container_id, tempdir)
            # Remove leading / from root as it is now under tempdir
            root = root[1:] if root.startswith("/") else root
            subprocess.check_output(
                ["tar", "-I", "zstd", "-C", tempdir, "-cf", output, root]
            )


def main():
    """main function of build_tast_binaries."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--skip-termina",
        action="store_true",
        help="Do not try to rebuild the termina tools archive.",
    )
    parser.add_argument(
        "--image-name",
        default="tast-binary-build",
        help="Name (docker tag) for the image built by docker.",
    )
    parser.add_argument(
        "--stage", default="tast-binary-build", help="Stage to build."
    )
    parser.add_argument(
        "--out-dir", default=None, help="Path to the CrOS output directory."
    )
    parser.add_argument(
        "--chroot",
        default=None,
        help="Path to the CrOS chroot (i.e. with build/ subdir)",
    )
    parser.add_argument(
        "--output",
        default="public-borealis-tast-binaries",
        help="Output tarball name.",
    )
    parser.add_argument(
        "--output-append-date",
        action="store_true",
        default=True,
        help="If set, append date to the output file.",
    )
    parser.add_argument(
        "--no-output-append-date",
        action="store_false",
        dest="output_append_date",
        help="If set, do not append date to the output file.",
    )
    parser.add_argument(
        "--root",
        default="/scratch",
        help="Root inside the container to pack into tarball.",
    )
    args = parser.parse_args()

    ch = chroot.Chroot(
        build_full.get_cros_dir(),
        board="borealis",
        chroot_path=args.chroot,
        out_dir=args.out_dir,
    )
    config = gather_build_config(ch)
    print("Build_full config args:", config)
    build_full.build_full(
        skip_termina=args.skip_termina,
        build_dir=build_full.get_build_dir(),
        image_name=args.image_name,
        stage=args.stage,
        cros_path=build_full.get_cros_dir(),
        licenses_tar=None,
        licenses_html=None,
        build_arg=[f"{key}={value}" for key, value in config.items()],
    )

    if args.output_append_date:
        args.output += "-" + datetime.date.today().strftime("%Y.%m.%d.%H%M%S")
    args.output += ".tar.zst"
    create_zst_tarball(args.image_name, args.output, args.root)


if __name__ == "__main__":
    main()
