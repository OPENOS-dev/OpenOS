# Copyright 2026 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""USB hierarchy helpers. Refactored to be a stateless module."""

import os
import re
from typing import Callable, List, Optional, Tuple, Type, TypeVar, Union

T = TypeVar("T", int, str)

class HierarchyError(Exception):
    """Hierarchy error class."""

# Default sysfs path to use to for USB device information.
SYSFS_PATH = "/sys/bus/usb/devices"

# Regex to discover usb device folders in SYSFS_PATH.
DEV_RE = re.compile(r"\d+-\d+(\.\d+)*\Z")

def mock_usb_sysfs_path_for_test(mock_dir: str) -> None:
    """Set the sysfs usb devices path to mock_dir for testing."""
    global SYSFS_PATH
    SYSFS_PATH = mock_dir

def restore_default_usb_sysfs_path_for_test() -> None:
    """Restore the sysfs usb devices path to its default value."""
    global SYSFS_PATH
    SYSFS_PATH = "/sys/bus/usb/devices"

def _read_from_sysfs(
    sysfs_path: str, dev_file: str, cast: Union[Type[T], Callable[[str], T]] = str
) -> T:
    """Read dev_file from sysfs_path and return result cast into cast."""
    if not os.path.isabs(sysfs_path):
        sysfs_path = os.path.join(SYSFS_PATH, sysfs_path)
    dev_file_full = os.path.join(sysfs_path, dev_file)
    if not os.path.exists(dev_file_full):
        raise HierarchyError(
            f"Requested sysfs attribute at {dev_file_full!r} cannot be read "
            "because the file cannot be found."
        )
    try:
        with open(dev_file_full, "r", encoding="utf-8") as devf:
            content = devf.read().strip()
            if cast is int:
                # Handle both decimal and hex (with 0x)
                return int(content, 0)
            return cast(content)
    except (ValueError, OSError) as e:
        raise HierarchyError(
            f"Unexpected content {content!r} or error at sysfs file {dev_file_full!r}. {e}"
        ) from e

def dev_num_from_sysfs(sysfs_path: str) -> int:
    """Look for 'devnum' under sysfs_path and return its value."""
    return _read_from_sysfs(sysfs_path, "devnum", cast=int)

def product_id_from_sysfs(sysfs_path: str) -> int:
    """Look for 'idProduct' under sysfs_path and return its value."""
    return _read_from_sysfs(sysfs_path, "idProduct", cast=lambda x: int(f"0x{x}", 0))

def serial_from_sysfs(sysfs_path: str) -> str:
    """Look for 'serial' under sysfs_path and return its value."""
    return _read_from_sysfs(sysfs_path, "serial")

def get_all_usb_device_sysfs_paths(
    vid_pid_list: Optional[List[Tuple[int, Optional[int]]]] = None
) -> List[str]:
    """Return all USB devices sysfs path which match the given VID/PID's."""
    dev_paths = []
    if not os.path.exists(SYSFS_PATH):
        return dev_paths

    for usb_dir in os.listdir(SYSFS_PATH):
        if DEV_RE.match(usb_dir):
            path = os.path.join(SYSFS_PATH, usb_dir)
            if vid_pid_list is None:
                dev_paths.append(path)
            else:
                try:
                    vid = _read_from_sysfs(path, "idVendor", cast=lambda x: int(f"0x{x}", 0))
                    pid = product_id_from_sysfs(path)

                    for target_vid, target_pid in vid_pid_list:
                        if target_vid == vid and (target_pid is None or target_pid == pid):
                            dev_paths.append(path)
                            break
                except HierarchyError:
                    continue
    return dev_paths

def get_sysfs_parent_hub_stub(sysfs_dev_path: str) -> Optional[str]:
    """Retrieve the usb port hub path up to and not including the device itself."""
    usbdir, dev_path = os.path.split(sysfs_dev_path.rstrip("/"))
    parent = ".".join(dev_path.split(".")[:-1])
    if not parent:
        parent = dev_path.split("-")[0]
    return os.path.join(usbdir, parent) if parent else None

def complement_bus_num(busnum: int) -> Optional[int]:
    """Find the complement bus to busnum."""
    busdir = f"usb{busnum}"
    sysfs_buspath = os.path.join(SYSFS_PATH, busdir)
    if not os.path.exists(sysfs_buspath):
        raise HierarchyError(f"No usb bus {busnum} found")
    pci_dev_path = os.path.dirname(os.path.realpath(sysfs_buspath))
    complement_candidates = [
        c for c in os.listdir(pci_dev_path) if re.match(r"usb\d+$", c)
    ]
    if busdir in complement_candidates:
        complement_candidates.remove(busdir)
    if not complement_candidates:
        return None
    complement_candidates_busnums = [int(c[3:]) for c in complement_candidates]
    if len(complement_candidates_busnums) > 1:
        raise HierarchyError(
            f"{complement_candidates_busnums!r} all potential complement buses to {busnum}"
        )
    return complement_candidates_busnums[0]
