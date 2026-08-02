#!/usr/bin/env python3
# Copyright 2020 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Convert a sysroot to an ext4 image.

Creates an ext4 filesystem image out of your sysroot, which you can use with
crosvm to run it in a VM (BYO kernel).
"""

import argparse
import os
import subprocess
import tempfile


DEFAULT_IMG_NAME = "rootfs.ext4"


class ScopedMount:
    """Helper for mounting filesystems using python's 'with' syntax.

    Mounts and unmounts unix filesystems (via the 'mount' command) in a way that
    works with python's 'with' syntax. I.e. this will safely unmount the
    filesystem even when the code that uses the mounted one throws.
    """

    def __init__(self, mount_type, src, dst):
        self.mount_type = mount_type
        self.src = src
        self.dst = dst

    def __enter__(self):
        cmd = ["sudo", "mount"]
        if self.mount_type is None or self.mount_type == "bind":
            cmd += ["--bind"]
        else:
            cmd += ["-t", self.mount_type]
        subprocess.check_call(cmd + [self.src, self.dst])
        return self

    def __exit__(self, error_type, value, traceback):
        subprocess.check_call(["sudo", "umount", self.dst])


# TODO(hollingum): this is an almost perfect copy of termina_build_image.py, so
# use that instead.
def create_fs_image(sysroot, image, shrink):
    """Create an image from the sysroot.

    Converts the sysroot into a bootable system image.

    Args:
        sysroot: path to the sysroot which will become "/" in the image.
        image: path to the image file you want to create.
        shrink: if true, also removes unused space from the disk image.
    """
    # Figure out how big you want the image to be.  We conservatively start with
    # an image 2x the size to account for the image's block size and leave some
    # space in case the user wants to do extra things.
    du_output = subprocess.check_output(["sudo", "du", "-bsx", sysroot]).decode(
        "utf-8"
    )
    src_size = int(du_output.split()[0])
    img_size = int(src_size * 2)

    # Create a file of the appropriate size.
    try:
        # Remove it first; otherwise we occasionally get a corrupted image.
        os.remove(image)
    except FileNotFoundError:
        pass

    with open(image, "wb+") as vm_rootfs:
        vm_rootfs.truncate(img_size)

    # Convert the file into an ext4 image.
    subprocess.check_call(
        [
            "/sbin/mkfs.ext4",
            "-F",
            "-m",
            "0",
            "-i",
            "16384",
            "-b",
            "4096",
            "-O",
            "^has_journal",
            image,
        ]
    )

    # Copy the sysroot into the image.
    with tempfile.TemporaryDirectory() as mnt_dir:
        with ScopedMount("ext4", image, mnt_dir):
            subprocess.check_call(
                ["sudo", "rsync", "-aH", sysroot + "/", mnt_dir]
            )
            # Make root own its home directory
            subprocess.check_call(
                ["sudo", "chown", "-R", "root:root", mnt_dir + "/root"]
            )
            # Make root own /var/empty (required for SSH server)
            subprocess.check_call(
                ["sudo", "chown", "root:root", mnt_dir + "/var/empty"]
            )
            # TODO(davidriley): Remove this once b/170268125 is fixed
            # and setuid bits are maintained.
            subprocess.check_call(
                [
                    "sudo",
                    "chown",
                    "root:root",
                    mnt_dir + "/usr/lib/dbus-daemon-launch-helper",
                ]
            )
            subprocess.check_call(
                [
                    "sudo",
                    "chmod",
                    "+s",
                    mnt_dir + "/usr/lib/dbus-daemon-launch-helper",
                ]
            )

    # Shrink the image to save space.
    if shrink:
        subprocess.check_call(["/sbin/e2fsck", "-y", "-f", image])
        subprocess.check_call(["/sbin/resize2fs", "-M", image])


def main():
    """Main function."""
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument(
        "--sysroot",
        required=True,
        help="Location where the ubuntu sysroot will be stored",
    )
    parser.add_argument(
        "--img", default=DEFAULT_IMG_NAME, help="Output filename of the image."
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

    args = parser.parse_args()
    create_fs_image(args.sysroot, args.img, args.shrink)


if __name__ == "__main__":
    main()  # pylint: disable=no-value-for-parameter
