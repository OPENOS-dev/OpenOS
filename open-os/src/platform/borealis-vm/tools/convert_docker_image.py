#!/usr/bin/env python3
# Copyright 2020 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Convert a docker container image to a borealis VM image.

Turns a docker image into a CrOS image. In addition to pulling the filesystem
out, we also tweak it slightly in ways that docker doesnt like.
"""

import argparse
import contextlib
import os
import subprocess
import sys
import tempfile

import sysroot_to_image

import config


@contextlib.contextmanager
def get_container_for_export(docker_image_name):
    container = (
        subprocess.check_output(["sudo", "docker", "create", docker_image_name])
        .decode(sys.stdout.encoding)
        .strip()
    )
    try:
        yield container
    finally:
        subprocess.check_call(["sudo", "docker", "container", "rm", container])


def extract_container_to_dir(container_id, output_dir):
    print("Extracting to", output_dir, " - This may take a while")
    cmd = ["sudo", "docker", "export", container_id]
    extract = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    subprocess.check_call(
        ["tar", "-x", "-F", "-", "-C", output_dir], stdin=extract.stdout
    )
    if extract.wait() != 0:
        raise subprocess.CalledProcessError(extract.returncode, cmd)


def massage_sysroot(sysroot):
    # Set permissions correctly, otherwise e.g. vshd cant acces /dev.
    subprocess.check_call(["sudo", "chmod", "-R", "a+rx", sysroot])
    # Setup resolv.conf the way maitred does it.
    #
    # TODO(hollingum): modify maitred so we dont need to do this.
    resolv = os.path.join(sysroot, "etc", "resolv.conf")
    subprocess.check_call(["rm", "-f", resolv])
    subprocess.check_call(["ln", "-s", "/run/resolv.conf", resolv])

    # Docker erases the contents of /etc/hostname during export, so we
    # have to create it here instead of in the Dockerfile.
    hostname = os.path.join(sysroot, "etc", "hostname")
    with open(hostname, "w") as f:
        f.write("Chromebook\n")


def convert(docker_image_name, output, should_shrink):
    with get_container_for_export(docker_image_name) as container_id:
        with tempfile.TemporaryDirectory() as tempdir:
            extract_container_to_dir(container_id, tempdir)
            massage_sysroot(tempdir)
            sysroot_to_image.create_fs_image(tempdir, output, should_shrink)
            # User must have write permissions for the destructor
            # (tempdir.cleanup()) to succeed deleting tempdir.
            # TODO(b/187794810): This WAR can be removed once the CrOS
            # development moves to Python 3.8+.
            subprocess.check_call(["chmod", "-R", "u+rw", tempdir])


def main():
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument(
        "--output",
        default="vm_rootfs.img",
        help="Name of the output image you will create",
    )
    parser.add_argument(
        "--noshrink",
        action="store_false",
        dest="shrink",
        help=(
            "Do not shrink the disk image. Useful when you need to make "
            "live modifications to it"
        ),
    )
    parser.add_argument(
        "--docker-image",
        default=config.BOREALIS_TAG,
        help="Name of the docker image you want to export",
    )

    args = parser.parse_args()
    convert(args.docker_image, args.output, args.shrink)


if __name__ == "__main__":
    main()
