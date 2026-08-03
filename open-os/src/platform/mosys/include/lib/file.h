/*
 * Copyright 2012 Google LLC
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of Google LLC nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * file.h: helper functions for handling files and directories
 */

#ifndef MOSYS_LIB_FILE_H__
#define MOSYS_LIB_FILE_H__

#include "mosys/log.h"

enum file_mode {
	FILE_READ,
	FILE_WRITE,
};
int file_open(const char *file, int rw);

/**
 * read_file(path, buf, buf_sz, log_level) - Helper to read entire
 * file contents into a NULL-terminated buffer.
 * @path:       Filename to read.
 * @buf:        Buffer to write contents into.
 * @buf_sz:     The size of @buf.
 * @log_level:  Errors opening or reading the file will cause error
 *              messages printed at this log level.
 *
 * read_file will read the entire contents of a file into a buffer,
 * and will print errors if the file is not readable, or if the whole
 * file does not fit into the buffer.
 *
 * Return: -1 on error, or the number of bytes read on success, not
 * including the NULL terminator added at the end.
 */
ssize_t read_file(const char *path, char *buf, size_t buf_sz,
		  enum log_levels log_level);

#endif /* MOSYS_LIB_FILE_H__ */
