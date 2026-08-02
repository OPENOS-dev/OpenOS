/* Copyright 2020 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdint.h>
#include <stdlib.h>

#include "lib/val2str.h"

const char *val2str_default(uint32_t val, const struct valstr *vs,
			    const char *default_str)
{
	for (size_t i = 0; vs[i].str; i++) {
		if (vs[i].val == val)
			return vs[i].str;
	}

	return default_str;
}

const char *val2str(uint32_t val, const struct valstr *vs)
{
	return val2str_default(val, vs, "Unknown");
}
