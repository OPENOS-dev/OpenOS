/* Copyright 2020 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef MOSYS_LIB_VAL2STR_H_
#define MOSYS_LIB_VAL2STR_H_

#include <stdint.h>

struct valstr {
	uint32_t val;
	const char *str;
};

/**
 * val2str_default() - convert value to string
 *
 * @val:         value to convert
 * @vs:          value-string data
 * @default_str: default string to return if no matching value found
 *
 * Returns: pointer to string, or default_str if no matching value
 * found.
 */
const char *val2str_default(uint32_t val, const struct valstr *vs,
			    const char *default_str);

/**
 * val2str() - convert value to string
 *
 * @val:        value to convert
 * @vs:         value-string data
 *
 * Returns: pointer to string, or "Unknown" static string if no
 * matching value found.
 */
const char *val2str(uint32_t val, const struct valstr *vs);

#endif /* MOSYS_LIB_VAL2STR_H_ */
