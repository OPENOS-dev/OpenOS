/* Copyright 2024 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * ound in the LICENSE file.
 */

#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cmocka.h>
#include "lib/cbi.h"
#include "lib/nonspd.h"

typeof(execvp) __wrap_execvp;

#define PART_NUM_STR "MT53E512M32D1NP-046WT"

const static char *ectool_mock_part_num;
static int ectool_exit_code;

static uint8_t buf[PART_NUM_LEN];

static int setup(void **state)
{
	ectool_mock_part_num = NULL;
	ectool_exit_code = 0;

	return 0;
}

int __wrap_execvp(const char *file, char *const argv[])
{
	if (strcmp(file, "ectool"))
		return -1;

	printf("%s\n", ectool_mock_part_num);

	exit(ectool_exit_code);
	__builtin_unreachable();
}

static void buffer_size_test(void **state)
{
	ectool_mock_part_num = PART_NUM_STR;

	assert_int_not_equal(cbi_get_dram_part_num(buf, 0), 0);
	assert_int_equal(cbi_get_dram_part_num(buf, sizeof(buf)), 0);
	assert_string_equal(buf, PART_NUM_STR);
}

static void ectool_long_part_num_test(void **state)
{
	ectool_mock_part_num = PART_NUM_STR PART_NUM_STR;

	assert_int_not_equal(cbi_get_dram_part_num(buf, sizeof(buf)), 0);
}

static void ectool_non_zero_exit_test(void **state)
{
	ectool_mock_part_num = PART_NUM_STR;
	ectool_exit_code = 1;

	assert_int_not_equal(cbi_get_dram_part_num(buf, sizeof(buf)), 0);
}

#define TEST(test_function_name) \
	cmocka_unit_test_setup(test_function_name, setup)

int main(void)
{
	const struct CMUnitTest tests[] = {
		TEST(ectool_long_part_num_test),
		TEST(ectool_non_zero_exit_test),
		TEST(buffer_size_test),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
