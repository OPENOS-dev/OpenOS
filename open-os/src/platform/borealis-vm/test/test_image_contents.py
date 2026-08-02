# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Tests ensuring the VM filesystem image contains necessary files."""

import os
import unittest


class TestLibraries(unittest.TestCase):
    """Test that required libraries are installed."""

    def test_libcrypt(self):
        """Test that we have 32-bit and 64-bit libcrypt.so.1.

        This was moved from libxcrypt to libxcrypt-compat in Feb 2022, breaking several games.
        """
        self.assertTrue(os.path.exists("/usr/lib/libcrypt.so.1"))
        self.assertTrue(os.path.exists("/usr/lib32/libcrypt.so.1"))


class TestServices(unittest.TestCase):
    """Test that required system services are installed."""

    def test_has_init(self):
        """Test that 'init' exists, so the VM can boot.

        In Feb 2023, the maitred binary was renamed from init to maitred, breaking the /sbin/init
        symlink and preventing the VM from booting.
         * os.path.exists does detect broken symlinks.
         * tools/termina_tools.py does check the maitred binary, but doesn't check this symlink.
        """
        self.assertTrue(os.path.exists("/sbin/init"))


if __name__ == "__main__":
    unittest.main()
