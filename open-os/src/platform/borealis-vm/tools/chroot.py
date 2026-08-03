# Copyright 2021 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Tools for working with a CrOS chroot.

Provides a convenient API for working inside a CrOS chroot, useful for building
packages/boards.
"""

import logging
import os
import pathlib
import subprocess


logger = logging.getLogger("chroot")


class Chroot:
    """Execute commands/tasks inside a CrOS chroot."""

    def __init__(self, cros_path, board, chroot_path=None, out_dir=None):
        self.cros_path = cros_path
        self.board = board
        self.chroot_path = (
            chroot_path if chroot_path else self.guess_chroot_path()
        )
        self.out_dir = out_dir if out_dir else self.guess_out_dir()

    def get_cros_path(self, *args):
        """Returns the CrOS root path with added paths."""
        return os.path.join(self.cros_path, *args)

    def get_chroot_path(self, *args):
        """Returns the chroot root path with added paths."""
        return os.path.join(self.chroot_path, *args)

    def guess_chroot_path(self):
        """Returns the chroot root path."""
        return os.path.join(self.cros_path, "chroot")

    def get_out_dir(self, *args):
        """Returns the out dir root path with added paths."""
        return os.path.join(self.out_dir, *args)

    def guess_out_dir(self):
        """Returns the out dir root path."""
        return os.path.join(self.cros_path, "out")

    def get_build_path(self, *args):
        """Returns the sysroot (or its descendants) for this chroot's board."""
        # If the newstyle build dir exists, return that.
        # Otherwise return the old build directory.
        if os.path.exists(self.get_out_dir("build", self.board, *args)):
            return self.get_out_dir("build", self.board, *args)
        return self.get_chroot_path("build", self.board, *args)

    def cros_sdk(self):
        """Returns cros_sdk with extra arguments."""
        return [
            "cros_sdk",
            f"--chroot={self.chroot_path}",
            f"--out-dir={self.out_dir}",
        ]

    def gen_cros_sdk_command(self, command):
        """Generate a cros_sdk command as a list."""
        return self.cros_sdk() + command

    def as_chroot_path(self, path, absolute=True):
        """Convert a path from outside the chroot to chroot relative."""
        chroot_path = pathlib.PurePath(path).relative_to(self.get_chroot_path())
        if absolute:
            chroot_path = os.path.join("/", chroot_path)
        return chroot_path

    def is_set_up(self):
        """Checks if the chroot is set up to build its board."""
        return os.path.isdir(self.get_build_path())

    def set_up(self):
        """Sets this chroot up to build its board (even if it was already)."""
        cmd = self.gen_cros_sdk_command(
            ["--", "setup_board", "--board=" + self.board, "--force"]
        )
        logger.info("Running: %s", cmd)
        subprocess.check_call(cmd, cwd=self.cros_path)

    def build_packages(self, package_name, clean_build=False):
        """Builds the given package for its board in the chroot."""
        if not self.is_set_up():
            self.set_up()
        cmd = self.gen_cros_sdk_command(
            [
                "--",
                "env",
                "USE=vm_borealis",
                "cros",
                "build-packages",
                "--autosetgov",
                "--board=" + self.board,
                package_name,
            ]
        )
        if clean_build:
            cmd += ["--cleanbuild"]
        logger.info("Running: %s", cmd)
        subprocess.check_call(cmd, cwd=self.cros_path)

    def emerge_package(self, package_name):
        """Emerge the given package for its board in the chroot."""
        if not self.is_set_up():
            self.set_up()
        cmd = self.gen_cros_sdk_command(
            [
                "--",
                "env",
                "USE=vm_borealis",
                "emerge-" + self.board,
                package_name,
            ]
        )
        logger.info("Running: %s", cmd)
        subprocess.check_call(cmd, cwd=self.cros_path)

    def check_files(self, expected_dirs, expected_files):
        """Checks if the given list of files exist."""
        for expected in expected_dirs:
            path = self.get_build_path(expected)
            if not os.path.isdir(path):
                raise Exception(path + " was not built.")
        for expected in expected_files:
            path = self.get_build_path(expected)
            if not os.path.isfile(path):
                raise Exception(expected + " was not found at " + path)

    def create_archive(self, output, root_dir, paths=None):
        """Creates an archive based on a directory within the chroot."""
        if paths is None:
            paths = ["./"]
        root_path = self.get_build_path(root_dir)
        cmd = ["tar", "cjvf", output, "-C", root_path]
        cmd.extend(paths)
        logger.info("Running: %s", cmd)
        subprocess.check_call(cmd)

    def unmerge_package(self, package_name):
        """Unmerge the given package for its board in the chroot."""
        if not self.is_set_up():
            self.set_up()
        cmd = self.gen_cros_sdk_command(
            [
                "--",
                "env",
                "USE=vm_borealis",
                "emerge-" + self.board,
                "-C",
                package_name,
            ]
        )
        logger.info("Running: %s", cmd)
        subprocess.check_call(cmd, cwd=self.cros_path)

    def build_and_archive(
        self,
        package_name,
        archive_output,
        archive_dir,
        expected_files=None,
        archive_paths=None,
    ):
        """Build a package and archive the results."""
        if expected_files is None:
            expected_files = []
        if archive_paths is None:
            archive_paths = ["./"]
        # Build the packages.
        self.build_packages(package_name)

        # Check the expected files in archive_dir were created.
        expected_paths = [os.path.join(archive_dir, x) for x in expected_files]
        self.check_files([archive_dir], expected_paths)

        # Create the archive.
        self.create_archive(archive_output, archive_dir, paths=archive_paths)

    def get_workon_list(self):
        """Returns the list of packages being cros_workon'd."""
        cmd = self.gen_cros_sdk_command(
            [
                "--",
                "env",
                "USE=vm_borealis",
                "cros_workon",
                "--board=" + self.board,
                "list",
            ]
        )
        logger.info("Running: %s", cmd)
        return (
            subprocess.check_output(cmd, cwd=self.cros_path)
            .decode("utf-8")
            .split()
        )

    def is_workon(self, package_name):
        """Checks if a package is being cros_workon'd."""
        return package_name in self.get_workon_list()

    def generate_breakpad_symbols(self, symbols_file, breakpad_dir):
        """Emerge the given package for its board in the chroot."""
        if not self.is_set_up():
            self.set_up()
        cmd = self.gen_cros_sdk_command(
            [
                "--",
                "cros_generate_borealis_breakpad_symbols",
                "--symbols-file",
                symbols_file,
                "--breakpad-dir",
                breakpad_dir,
            ]
        )
        logger.info("Running: %s", cmd)
        subprocess.check_call(cmd, cwd=self.cros_path)
