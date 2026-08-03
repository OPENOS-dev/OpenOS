/* Copyright 2012 Google LLC
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
 * file.c: implementations some useful functions for files and directories
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mosys/alloc.h"
#include "mosys/log.h"
#include "mosys/mosys.h"

#include "lib/file.h"

/*
 * file_open  -  Generic file open function
 *
 * @file:       file path and name
 * @rw:         read=0, write=1
 *
 * returns valid file descriptor on success
 * returns <0 to indicate failure
 */
int file_open(const char *file, int rw)
{
	struct stat st1, st2;
	int fd;

	/* verify existance */
	if (lstat(file, &st1) < 0) {
		if (rw == FILE_WRITE) {
			/* does not exist, ok to create */
			fd = open(file, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
			if (fd == -1) {
				lperror(LOG_DEBUG,
					"Unable to open file %s for write",
					file);
				return -1;
			}
			/* created ok, now return the descriptor */
			return fd;
		} else {
			lprintf(LOG_DEBUG, "File %s does not exist\n", file);
			return -1;
		}
	}

	if (rw == FILE_READ) {
		/* on read skip the extra checks */
		fd = open(file, O_RDONLY);
		if (fd == -1) {
			lperror(LOG_NOTICE, "Unable to open file %s", file);
			return -1;
		}
		return fd;
	}

	/* it exists - only regular files and char devices */
	if (S_ISREG(st1.st_mode) == 0 && S_ISCHR(st1.st_mode) == 0) {
		lprintf(LOG_NOTICE, "File %s has invalid mode: %d\n", file,
			st1.st_mode);
		return -1;
	}

	/* allow only files with 1 link (itself) */
	if (st1.st_nlink != 1) {
		lprintf(LOG_NOTICE, "File %s has invalid link count: %d != 1\n",
			file, (int)st1.st_nlink);
		return -1;
	}

	fd = open(file, O_RDWR);
	if (fd == -1) {
		lperror(LOG_NOTICE, "Unable to open file %s", file);
		return -1;
	}

	/* stat again */
	if (fstat(fd, &st2) < 0) {
		lperror(LOG_NOTICE, "Unable to stat file %s", file);
		close(fd);
		return -1;
	}

	/* verify inode */
	if (st1.st_ino != st2.st_ino) {
		lprintf(LOG_NOTICE, "File %s has invalid inode: %jx != %jx\n",
			file, (uintmax_t)st1.st_ino, (uintmax_t)st2.st_ino);
		close(fd);
		return -1;
	}

	/* verify owner */
	if (st1.st_uid != st2.st_uid) {
		lprintf(LOG_NOTICE, "File %s has invalid user id: %d != %d\n",
			file, st1.st_uid, st2.st_uid);
		close(fd);
		return -1;
	}

	/* verify inode */
	if (st2.st_nlink != 1) {
		lprintf(LOG_NOTICE, "File %s has invalid link count: %d != 1\n",
			file, (int)st2.st_nlink);
		close(fd);
		return -1;
	}

	return fd;
}

ssize_t read_file(const char *path, char *buf, size_t buf_sz,
		  enum log_levels log_level)
{
	FILE *f;
	size_t bytes_read;

	/* Buffer size of 0 leaves us with no space to NULL-terminate */
	if (buf_sz == 0) {
		lprintf(log_level, "%s: cannot be used with zero-size buffer\n",
			__func__);
		return -1;
	}

	f = fopen(path, "rb");
	if (!f) {
		lprintf(log_level, "%s: Failed to open \"%s\" for reading\n",
			__func__, path);
		return -1;
	}

	bytes_read = fread(buf, 1, buf_sz - 1, f);
	if (ferror(f)) {
		lprintf(log_level, "%s: File \"%s\" in error state\n", __func__,
			path);
		goto fail;
	}

	if (!feof(f)) {
		/*
		 * Try to read one more byte from the file to see if
		 * it puts us at EOF first...
		 */
		char one_more_byte[1];

		if (fread(one_more_byte, 1, 1, f) == 0 && feof(f))
			goto ok;

		lprintf(log_level,
			"%s: The entire file \"%s\" could not fit in"
			"a buffer of size %zu\n",
			__func__, path, buf_sz);
		goto fail;
	}

ok:
	MOSYS_CHECK(bytes_read < buf_sz);

	/* NULL-terminate the buffer */
	buf[bytes_read] = '\0';

	fclose(f);
	return bytes_read;
fail:
	fclose(f);
	return -1;
}
