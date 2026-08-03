// Copyright 2022 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ext2fs/ext2fs.h>
#include <string.h>
#include <unistd.h>

// This program forces the inode and block bitmaps (static metadata), of
// the given unmounted ext4 filesystem, to be written out sequentially. Because
// of how VM storage works, data isn't backed by physical storage until it is
// written. Writing out the bitmaps ahead of time makes sure that they're backed
// by physical storage and won't have issues being committed in low-disk-space
// scenarios.

// For more about bitmaps, see
// https://ext4.wiki.kernel.org/index.php/Ext4_Disk_Layout#Block_and_inode_Bitmaps

void usage(void) { fprintf(stderr, "Usage: setup_bitmaps <device>\n"); }

// Checks that the |device_name| is safe to use.
int check_fs(const char *device_name) {
  int mount_flags;
  errcode_t retval = ext2fs_check_if_mounted(device_name, &mount_flags);
  if (retval) {
    fprintf(stderr, "Failed to determine if %s is mounted : %li\n", device_name,
            retval);
    return retval;
  }
  if (mount_flags & EXT2_MF_MOUNTED) {
    fprintf(stderr, "Aborting, %s is mounted.\n", device_name);
    return retval;
  }
  if (mount_flags & EXT2_MF_BUSY) {
    fprintf(stderr, "Aborting, %s is apparently in use by the system.\n",
            device_name);
    return retval;
  }
  return 0;
}

// Reads out and then writes the inode and block bitmaps, of |device_name|, in
// sequential order.
int write_bitmaps(const char *device_name) {
  io_manager io_ptr = unix_io_manager;
  ext2_filsys fs;
  errcode_t retval = ext2fs_open(
      device_name, EXT2_FLAG_64BITS | EXT2_FLAG_RW | EXT2_FLAG_EXCLUSIVE, 0, 0,
      io_ptr, &fs);
  if (retval) {
    fprintf(stderr, "Couldn't open %s: %li\n", device_name, retval);
    return retval;
  }
  retval = ext2fs_read_bitmaps(fs);
  if (retval) {
    fprintf(stderr, "Failed to read bitmaps: %li\n", retval);
    ext2fs_close(fs);
    return retval;
  }

  // Forces the bitmaps to be written out.
  ext2fs_mark_ib_dirty(fs);
  ext2fs_mark_bb_dirty(fs);

  retval = ext2fs_close(fs);
  if (retval) {
    fprintf(stderr, "Couldn't close %s: %li\n", device_name, retval);
    return retval;
  }
  sync();
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    usage();
    return EXIT_FAILURE;
  }

  char *device_name = argv[1];
  if (check_fs(device_name)) {
    return EXIT_FAILURE;
  }

  if (write_bitmaps(device_name)) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}