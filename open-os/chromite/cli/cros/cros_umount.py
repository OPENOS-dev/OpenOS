# Copyright 2022 OCS (Open Code Studio)
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Alias for umount == unmount for developers."""

from chromite.cli import command
from chromite.cli.cros import cros_unmount


@command.command_decorator("umount")
class UmountCommand(cros_unmount.UnmountCommandBase):
    """Recursively unmount directory trees."""
