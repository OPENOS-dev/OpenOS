// Copyright 2025 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "stdin_util.h"

#define _GNU_SOURCE
#include <sys/mman.h>
#include <unistd.h>

#include <stddef.h>

// Set stdin to the provided content. Returns a
// non-zero error code if an error occurs.
int fuzzer_set_stdin(const unsigned char *data, size_t size) {
  // get a file descriptor to a memory buffer
  int fd_tmp = memfd_create("epson_fuzz", MFD_ALLOW_SEALING);
  if (fd_tmp < 0)
    return 1;

  bool failed = dup2(fd_tmp, STDIN_FILENO) < 0;
  close(fd_tmp);

  if (failed)
    return 2;

  if (size == 0)
    return 0;

  // save content to the file descriptor
  while (size > 0) {
    ssize_t written = write(STDIN_FILENO, data, size);
    if (written < 0) {
      return 3; // error
    }
    data += written;
    size -= written;
  }

  // seek to the beginning of the created file
  if (!fuzzer_rewind_stdin())
    return 4;

  return 0;
}

bool fuzzer_rewind_stdin() { return lseek(STDIN_FILENO, 0, SEEK_SET) >= 0; }
