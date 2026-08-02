/* Copyright 2024 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef LIB_CBI_H__
#define LIB_CBI_H__

#include <stddef.h>
#include <stdint.h>

/*
 * cbi_get_dram_part_num - Get DRAM_PART_NUM from CBI.
 *
 * @buf :      buffer for storing DRAM_PART_NUM
 * @buf_size : buffer size
 *
 * returns 0 to indicate success, otherwise failure.
 */
int cbi_get_dram_part_num(uint8_t *buf, size_t buf_size);

#endif /* LIB_CBI_H__ */
