#!/usr/bin/env python3
# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Create a test tarball from a docker image."""

import argparse
import tarfile
import tempfile

from convert_docker_image import extract_container_to_dir
from convert_docker_image import get_container_for_export

import config


def create_tarball(docker_image_name, output, root):
    """Create a tarball from a docker image directory."""
    with get_container_for_export(docker_image_name) as container_id:
        with tempfile.TemporaryDirectory() as tempdir:
            extract_container_to_dir(container_id, tempdir)
            with tarfile.open(output, "w") as t:
                # Don't use os.path.join() to handle root being absolute.
                t.add(tempdir + "/" + root, arcname=root)


def main():
    """Main function."""
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument(
        "--output",
        default="test_tools.tar",
        help="Name of the output tarball you will create",
    )
    parser.add_argument(
        "--docker-image",
        default=config.BOREALIS_TAG,
        help="Name of the docker image you want to export",
    )
    parser.add_argument("root", help="Name of the directory to extract")

    args = parser.parse_args()
    create_tarball(args.docker_image, args.output, args.root)


if __name__ == "__main__":
    main()
