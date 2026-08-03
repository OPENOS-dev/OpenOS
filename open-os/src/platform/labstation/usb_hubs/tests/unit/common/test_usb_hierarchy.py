# Copyright 2026 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Unit tests for usb_hierarchy helpers."""

import os
import shutil
import tempfile
import unittest
from usb_hubs.common import usb_hierarchy as uh


class TestUSBHierarchy(unittest.TestCase):
    """Unit tests for usb_hierarchy module."""

    def setUp(self):
        self.test_dir = tempfile.mkdtemp()
        # Create a temp dir for /sys/...
        self.sys_dir = os.path.join(self.test_dir, "sys")
        os.makedirs(self.sys_dir)
        uh.mock_usb_sysfs_path_for_test(self.sys_dir)

    def tearDown(self):
        uh.restore_default_usb_sysfs_path_for_test()
        shutil.rmtree(self.test_dir)

    def _create_mock_device(self, dev_path, busnum, devnum, vid, pid, serial=None):
        full_path = os.path.join(self.sys_dir, dev_path)
        os.makedirs(full_path)
        with open(os.path.join(full_path, "busnum"), "w") as f:
            f.write(str(busnum))
        with open(os.path.join(full_path, "devnum"), "w") as f:
            f.write(str(devnum))
        with open(os.path.join(full_path, "idVendor"), "w") as f:
            f.write(vid)
        with open(os.path.join(full_path, "idProduct"), "w") as f:
            f.write(pid)
        if serial:
            with open(os.path.join(full_path, "serial"), "w") as f:
                f.write(serial)
        return full_path

    def test_get_all_usb_device_sysfs_paths(self):
        """Test getting all usb device sysfs paths."""
        self._create_mock_device("1-1", 1, 2, "18d1", "501b", "serial1")
        self._create_mock_device("1-2", 1, 3, "0403", "6001", "serial2")

        # Test all
        paths = uh.get_all_usb_device_sysfs_paths()
        self.assertEqual(len(paths), 2)

        # Test filtered by vid/pid
        vid_pid_list = [(0x18D1, 0x501B)]
        paths = uh.get_all_usb_device_sysfs_paths(vid_pid_list)
        self.assertEqual(len(paths), 1)
        self.assertTrue(paths[0].endswith("1-1"))

    def test_serial_from_sysfs(self):
        """Test reading serial from sysfs."""
        path = self._create_mock_device("1-1", 1, 2, "18d1", "501b", "myserial")
        self.assertEqual(uh.serial_from_sysfs(path), "myserial")

    def test_dev_num_from_sysfs(self):
        """Test reading devnum from sysfs."""
        path = self._create_mock_device("1-1", 1, 12, "18d1", "501b")
        self.assertEqual(uh.dev_num_from_sysfs(path), 12)

    def test_product_id_from_sysfs(self):
        """Test reading product id from sysfs."""
        path = self._create_mock_device("1-1", 1, 2, "18d1", "501b")
        self.assertEqual(uh.product_id_from_sysfs(path), 0x501B)

    def test_get_sysfs_parent_hub_stub(self):
        """Test getting parent hub stub."""
        self.assertEqual(
            uh.get_sysfs_parent_hub_stub("/sys/bus/usb/devices/1-1.2.3"),
            "/sys/bus/usb/devices/1-1.2",
        )
        self.assertEqual(
            uh.get_sysfs_parent_hub_stub("/sys/bus/usb/devices/1-1"),
            "/sys/bus/usb/devices/1",
        )

    def test_hierarchy_error_missing_file(self):
        """Test hierarchy error when file is missing."""
        path = self._create_mock_device("1-1", 1, 2, "18d1", "501b", serial="exist")
        os.remove(os.path.join(path, "serial"))
        with self.assertRaises(uh.HierarchyError):
            uh.serial_from_sysfs(path)

    def test_complement_bus_num(self):
        """Test complement bus num finding."""
        pci_dir = os.path.join(self.test_dir, "pci_dev")
        os.makedirs(pci_dir)

        usb1_dir = os.path.join(pci_dir, "usb1")
        usb2_dir = os.path.join(pci_dir, "usb2")
        os.makedirs(usb1_dir)
        os.makedirs(usb2_dir)

        mock_usb1_path = os.path.join(self.sys_dir, "usb1")
        mock_usb2_path = os.path.join(self.sys_dir, "usb2")

        os.symlink(usb1_dir, mock_usb1_path)
        os.symlink(usb2_dir, mock_usb2_path)

        self.assertEqual(uh.complement_bus_num(1), 2)
        self.assertEqual(uh.complement_bus_num(2), 1)

if __name__ == "__main__":
    unittest.main()
