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
 */

#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

// cmocka doesn't include some headers it uses, e.g. setjmp. Prevent
// clang-format from putting the headers in order, so it gets the above
// includes.
// clang-format off
#include <cmocka.h>
// clang-format on

#include "mosys/log.h"
#include "mosys/mosys.h"
#include "mosys/platform.h"

#include "lib/file.h"

static void get_testdata_file(const char *name, char *buf, size_t buf_sz)
{
	char *src = getenv("SRC");
	assert_non_null(src);

	snprintf(buf, buf_sz, "%s/unittests/testdata/%s", src, name);
}

/* This should be the contents of unittests/testdata/hello */
#define HELLO_DATA "hello world\n"

static void read_file_success_test(void **state)
{
	ssize_t ret;
	char path[PATH_MAX];
	char buf[__builtin_strlen(HELLO_DATA) + 1];

	get_testdata_file("hello", path, ARRAY_SIZE(path));
	ret = read_file(path, buf, ARRAY_SIZE(buf), LOG_DEBUG);
	assert_int_equal(ret, __builtin_strlen(HELLO_DATA));
	assert_string_equal(buf, HELLO_DATA);
}

static void read_file_no_buffer_test(void **state)
{
	ssize_t ret;
	char path[PATH_MAX];
	/* We want to pass a non-NULL address, so make it at least size 1 */
	char buf[1];

	get_testdata_file("hello", path, ARRAY_SIZE(path));
	ret = read_file(path, buf, 0, LOG_DEBUG);
	assert_int_equal(ret, -1);
}

static void read_file_small_buffer_test(void **state)
{
	ssize_t ret;
	char path[PATH_MAX];
	char buf[1];

	get_testdata_file("hello", path, ARRAY_SIZE(path));
	ret = read_file(path, buf, ARRAY_SIZE(buf), LOG_DEBUG);
	assert_int_equal(ret, -1);
}

static void read_file_no_file_test(void **state)
{
	ssize_t ret;
	char buf[1];

	ret = read_file("/does/not/exist", buf, ARRAY_SIZE(buf), LOG_DEBUG);
	assert_int_equal(ret, -1);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(read_file_success_test),
		cmocka_unit_test(read_file_no_buffer_test),
		cmocka_unit_test(read_file_small_buffer_test),
		cmocka_unit_test(read_file_no_file_test),
	};

	return cmocka_run_group_tests(tests, /*group_setup=*/NULL,
				      /*group_teardown=*/NULL);
}
