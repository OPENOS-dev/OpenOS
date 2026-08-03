/* Copyright 2024 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "lib/cbi.h"
#include "lib/nonspd.h"

#define READ_FD 0
#define WRITE_FD 1
#define MIN(a, b) (((a) > (b)) ? (b) : (a))

static void _strip_eol(char *str)
{
	char *newline = strchr(str, '\n');

	if (newline)
		*newline = '\0';
}

int cbi_get_dram_part_num(uint8_t *buf, size_t buf_size)
{
	pid_t pid;
	int fd[2];

	ssize_t count;
	char tmp_buf[PART_NUM_LEN];

	if (pipe(fd) != 0) {
		lprintf(LOG_ERR, "%s: pipe() error\n", __func__);
		return -1;
	}

	pid = fork();
	if (pid < 0) {
		lprintf(LOG_ERR, "%s: failed to fork child process\n",
			__func__);
		return -1;
	} else if (pid == 0) {
		char *argv[] = { "ectool", "cbi", "get", "3", NULL };

		if (dup2(fd[WRITE_FD], STDOUT_FILENO) == -1 ||
		    dup2(fd[WRITE_FD], STDERR_FILENO) == -1) {
			lprintf(LOG_ERR, "%s: dup2() error\n", __func__);
			exit(EXIT_FAILURE);
		}

		if (close(fd[READ_FD]) || close(fd[WRITE_FD])) {
			lprintf(LOG_ERR, "%s: close() error\n", __func__);
			exit(EXIT_FAILURE);
		}

		execvp(argv[0], argv);
		exit(EXIT_FAILURE);
	} else {
		int status;
		ssize_t read_rv;
		ssize_t total_read = 0;

		if (close(fd[WRITE_FD]) < 0) {
			lprintf(LOG_DEBUG, "%s: close() error\n", __func__);
			return -1;
		}

		while ((read_rv = read(fd[READ_FD], tmp_buf,
				       sizeof(tmp_buf) - 1)) > 0) {
			total_read += read_rv;
			lprintf(LOG_DEBUG, "%s: read_rv: %zu, tmp_buf: %s\n",
				__func__, read_rv, tmp_buf);
		}

		lprintf(LOG_DEBUG, "%s: read_rv: %zu, tmp_buf: %s\n", __func__,
			read_rv, tmp_buf);

		if (close(fd[READ_FD]) < 0) {
			lprintf(LOG_DEBUG, "%s: close() error\n", __func__);
			return -1;
		}

		if (waitpid(pid, &status, 0) < 0) {
			lprintf(LOG_DEBUG, "%s: wait() error\n", __func__);
			return -1;
		}

		if (WEXITSTATUS(status) != 0) {
			lprintf(LOG_DEBUG, "%s: WEXITSTATUS(): %d\n", __func__,
				WEXITSTATUS(status));
			return -1;
		}

		/* We don't expect ectool to return the string longer then
		 * PART_NUM_LEN.
		 */
		if (read_rv < 0 || total_read > (sizeof(tmp_buf) - 1))
			return -1;

		/* We only need the result before `read` reaches EOF. */
		count = strlen(tmp_buf);

		_strip_eol(tmp_buf);
	}

	/* Need an extra byte for '\0' */
	if (buf_size <= count)
		return -1;

	strncpy((char *)buf, tmp_buf, MIN(buf_size, count));

	return 0;
}
